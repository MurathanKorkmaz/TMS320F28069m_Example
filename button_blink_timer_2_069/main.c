#include "DSP28x_Project.h"

int digitalRead(int pin);
void digitalWrite(int pin, int value);
void btn_blink_setup();
void btn_blink_loop();

int buttonState3; // Buton 3 durumunu saklamak i�in de�i�ken
int buttonState4; // Buton 4 durumunu saklamak i�in de�i�ken

int led1_on = 0; // LED'in yanma durumunu saklamak i�in de�i�ken
int timer_started = 0; // Timer'�n ba�lad���n� belirten de�i�ken

__interrupt void cpu_timer0_isr(void);

int main(void)
{
    // Sistem ba�latma kodlar�
    InitSysCtrl();

    btn_blink_setup();

    DINT;

    InitPieCtrl();

    IER = 0x0000;
    IFR = 0x0000;

    InitPieVectTable();

    EALLOW;    // This is needed to write to EALLOW protected registers
    PieVectTable.TINT0 = &cpu_timer0_isr;
    EDIS;      // This is needed to disable write to EALLOW protected registers

    InitCpuTimers();   // For this example, only initialize the Cpu Timers

    ConfigCpuTimer(&CpuTimer0, 80, 1000000); // 1000 ms at 80 MHz

    CpuTimer0Regs.TCR.all = 0x4001;

    IER |= M_INT1;

    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

    EINT;   // Enable Global interrupt INTM
    ERTM;   // Enable Global realtime interrupt DBGM

    while(1)
    {
        btn_blink_loop();
    }
}

void btn_blink_setup()
{
    // GPIO pinlerini yap�land�rma
    EALLOW;

    // GPIO3'� giri� pini olarak ayarla
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 0;  // GPIO3'� GPIO pini olarak ayarla
    GpioCtrlRegs.GPADIR.bit.GPIO3 = 0;   // GPIO3'� giri� pini olarak ayarla

    // GPIO4'� giri� pini olarak ayarla
    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 0;  // GPIO4'� GPIO pini olarak ayarla
    GpioCtrlRegs.GPADIR.bit.GPIO4 = 0;   // GPIO4'� giri� pini olarak ayarla

    // GPIO1'i ��k�� pini olarak ayarla
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 0;  // GPIO1'i GPIO pini olarak ayarla
    GpioCtrlRegs.GPADIR.bit.GPIO1 = 1;   // GPIO1'i ��k�� pini olarak ayarla

    EDIS;
}

void btn_blink_loop()
{
    buttonState3 = digitalRead(3); // GPIO3 pinini oku
    buttonState4 = digitalRead(4); // GPIO4 pinini oku

    if((buttonState3 == 1 || buttonState4 == 1) && !led1_on)
    {
        // Buton bas�ld�
        digitalWrite(1, 1); // GPIO1 ��k���n� 1 yap (LED'i yak)
        led1_on = 1;
        timer_started = 1;
        CpuTimer0Regs.TCR.bit.TSS = 0; // Timer'� ba�lat
    }
}

int digitalRead(int pin)
{
    if(pin == 3)
    {
        return GpioDataRegs.GPADAT.bit.GPIO3; // GPIO3 pininin durumunu oku
    }
    else if(pin == 4)
    {
        return GpioDataRegs.GPADAT.bit.GPIO4; // GPIO4 pininin durumunu oku
    }
    return 0;
}

void digitalWrite(int pin, int value)
{
    if(pin == 1)
    {
        if(value == 0)
        {
            GpioDataRegs.GPACLEAR.bit.GPIO1 = 1; // GPIO1 ��k���n� 0 yap (LED'i s�nd�r)
        }
        else if(value == 1)
        {
            GpioDataRegs.GPASET.bit.GPIO1 = 1; // GPIO1 ��k���n� 1 yap (LED'i yak)
        }
    }
}

__interrupt void cpu_timer0_isr(void)
{
    static int led1_timer = 0;

    if(timer_started)
    {
        led1_timer++;
        if(led1_timer >= 1) // 1000 ms ge�ti
        {
            digitalWrite(1, 0); // GPIO1 ��k���n� 0 yap (LED'i s�nd�r)
            led1_timer = 0;
            led1_on = 0;
            timer_started = 0;
            CpuTimer0Regs.TCR.bit.TSS = 1; // Timer'� durdur
        }
    }

    CpuTimer0.InterruptCount++;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}
