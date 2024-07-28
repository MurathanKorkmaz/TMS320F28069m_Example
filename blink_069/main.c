#include "DSP28x_Project.h"


void digitalWrite(int code, int value);

void main(void)
{
    //
    // Ad�m 1. Sistem Kontrol�n� Ba�lat�n:
    // PLL, WatchDog, �evresel Saatleri etkinle�tir
    // Bu �rnek i�lev, F2806x_SysCtrl.c dosyas�nda bulunur.
    //
    InitSysCtrl();


    //
    // Ad�m 2. GPIO'yu Ba�lat�n:
    // Bu �rnek fonksiyon F2806x_Gpio.c dosyas�nda bulunur ve
    // GPIO'nun varsay�lan durumuna nas�l ayarlanaca��n� g�sterir.
    //
    // InitGpio(); // Bu �rnek i�in atland�


    //
    // GPIO3'� GPIO ��k�� pini olarak yap�land�r�n
    //
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO4=0;
    GpioCtrlRegs.GPADIR.bit.GPIO4=1;
    EDIS;

    //
    // BO�TA d�ng�. Sadece otur ve sonsuza kadar d�n (iste�e ba�l�)
    //


    while(1)
    {
        digitalWrite(3,0);
        DELAY_US(50000);
        digitalWrite(3,1);
        DELAY_US(50000);
    }
}

void digitalWrite(int code, int value)
{
    if(code == 3)
    {
        if(value == 0)
        {
            GpioDataRegs.GPACLEAR.bit.GPIO4 = 1;
        }
        else if(value == 1)
        {
            GpioDataRegs.GPASET.bit.GPIO4 = 1;
        }
    }
}
