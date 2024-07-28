#include "DSP28x_Project.h"

int digitalRead(int pin);
void digitalWrite(int pin, int value);

void btn_blink_setup();
void btn_blink_loop();

int buttonState3; // Buton 3 durumunu saklamak i�in de�i�ken
int buttonState4; // Buton 4 durumunu saklamak i�in de�i�ken

int main(void)
{

    btn_blink_setup();

    while(1)
    {
        btn_blink_loop();
    }
}


void btn_blink_setup()
{
    // Sistem ba�latma kodlar�
    InitSysCtrl();

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

    // GPIO2'i ��k�� pini olarak ayarla
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 0;  // GPIO2'i GPIO pini olarak ayarla
    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;   // GPIO2'i ��k�� pini olarak ayarla
    EDIS;
}

void btn_blink_loop()
{
    buttonState3 = digitalRead(3); // GPIO3 pinini oku
    buttonState4 = digitalRead(4); // GPIO4 pinini oku

    if(buttonState3 == 1)
    {
        // Buton bas�ld�
        digitalWrite(1, 1); // GPIO1 ��k���n� 1 yap (LED'i yak)
    }
    else
    {
        // Buton bas�l� de�il
        digitalWrite(1, 0); // GPIO1 ��k���n� 0 yap (LED'i s�nd�r)
    }


    if(buttonState4 == 1)
    {
        // Buton bas�ld�
        digitalWrite(2, 1); // GPIO2 ��k���n� 1 yap (LED'i yak)
    }
    else
    {
        // Buton bas�l� de�il
        digitalWrite(2, 0); // GPIO2 ��k���n� 0 yap (LED'i s�nd�r)
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
        return GpioDataRegs.GPADAT.bit.GPIO4; // GPIO3 pininin durumunu oku
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
    else if(pin == 2)
    {
        if(value == 0)
        {
            GpioDataRegs.GPACLEAR.bit.GPIO2 = 1; // GPIO1 ��k���n� 0 yap (LED'i s�nd�r)
        }
        else if(value == 1)
        {
            GpioDataRegs.GPASET.bit.GPIO2 = 1; // GPIO1 ��k���n� 1 yap (LED'i yak)
        }
    }
}
