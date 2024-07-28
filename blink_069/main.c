#include "DSP28x_Project.h"


void digitalWrite(int code, int value);

void main(void)
{
    //
    // Adým 1. Sistem Kontrolünü Baþlatýn:
    // PLL, WatchDog, Çevresel Saatleri etkinleþtir
    // Bu örnek iþlev, F2806x_SysCtrl.c dosyasýnda bulunur.
    //
    InitSysCtrl();


    //
    // Adým 2. GPIO'yu Baþlatýn:
    // Bu örnek fonksiyon F2806x_Gpio.c dosyasýnda bulunur ve
    // GPIO'nun varsayýlan durumuna nasýl ayarlanacaðýný gösterir.
    //
    // InitGpio(); // Bu örnek için atlandý


    //
    // GPIO3'ü GPIO çýkýþ pini olarak yapýlandýrýn
    //
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO4=0;
    GpioCtrlRegs.GPADIR.bit.GPIO4=1;
    EDIS;

    //
    // BOÞTA döngü. Sadece otur ve sonsuza kadar dön (isteðe baðlý)
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
