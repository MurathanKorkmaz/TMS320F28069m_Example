#include "DSP28x_Project.h"
#include "F2806x_Examples.h"

void SetupGpio()
{
    EALLOW;

    // GPIO1 ve GPIO2'yi çýkýþ pini olarak ayarla
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 0;   // GPIO1'i GPIO pini olarak ayarla
    GpioCtrlRegs.GPADIR.bit.GPIO1 = 1;    // GPIO1'i çýkýþ pini olarak ayarla
    GpioDataRegs.GPACLEAR.bit.GPIO1 = 1;  // GPIO1'i sýfýrla

    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 0;   // GPIO2'yi GPIO pini olarak ayarla
    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;    // GPIO2'yi çýkýþ pini olarak ayarla
    GpioDataRegs.GPACLEAR.bit.GPIO2 = 1;  // GPIO2'yi sýfýrla

    // GPIO3 ve GPIO4'ü giriþ pini olarak ayarla
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 0;   // GPIO3'ü GPIO pini olarak ayarla
    GpioCtrlRegs.GPADIR.bit.GPIO3 = 0;    // GPIO3'ü giriþ pini olarak ayarla

    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 0;   // GPIO4'ü GPIO pini olarak ayarla
    GpioCtrlRegs.GPADIR.bit.GPIO4 = 0;    // GPIO4'ü giriþ pini olarak ayarla

    // GPIO3 ve GPIO4 için Pin Change Interrupt ayarlarý
    GpioCtrlRegs.GPACTRL.bit.QUALPRD3 = 0xFF; // Kesme filtre periyodu ayarla
    GpioCtrlRegs.GPAQSEL1.bit.GPIO3 = 2;  // GPIO3 için yükselen kenar kesme seç
    GpioCtrlRegs.GPAQSEL1.bit.GPIO4 = 2;  // GPIO4 için yükselen kenar kesme seç

    // GPIO3 ve GPIO4 Pin Change Interrupt ayarlarý
    GpioIntRegs.GPIOXINT1SEL.bit.GPIOSEL = 3; // XINT1 için GPIO3'ü seç
    GpioIntRegs.GPIOXINT2SEL.bit.GPIOSEL = 4; // XINT2 için GPIO4'ü seç

    // Pin Change Interrupt ayarlarý
    XIntruptRegs.XINT1CR.bit.POLARITY = 1; // Yükselen kenar kesme
    XIntruptRegs.XINT1CR.bit.ENABLE = 1;   // XINT1'i etkinleþtir
    XIntruptRegs.XINT2CR.bit.POLARITY = 1; // Yükselen kenar kesme
    XIntruptRegs.XINT2CR.bit.ENABLE = 1;   // XINT2'yi etkinleþtir

    EDIS;
}

void main(void)
{
    InitSysCtrl();  // Sistem baþlatma kodlarý

    SetupGpio();    // GPIO pinlerini yapýlandýr

    while(1)
    {
        // Ana döngüde yapýlacak iþlemler
    }
}

// GPIO3 kesme hizmet altprogramý (ISR)
interrupt void gpio3_isr(void)
{
    GpioDataRegs.GPASET.bit.GPIO1 = 1;   // GPIO1'i set (LED'i yak)
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // PIE kesme bayraðýný temizle
}

// GPIO4 kesme hizmet altprogramý (ISR)
interrupt void gpio4_isr(void)
{
    GpioDataRegs.GPASET.bit.GPIO2 = 1;   // GPIO2'yi set (LED'i yak)
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // PIE kesme bayraðýný temizle
}

