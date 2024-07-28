#include "DSP28x_Project.h"
#include "F2806x_Examples.h"

void SetupGpio()
{
    EALLOW;

    // GPIO1 ve GPIO2'yi ��k�� pini olarak ayarla
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 0;   // GPIO1'i GPIO pini olarak ayarla
    GpioCtrlRegs.GPADIR.bit.GPIO1 = 1;    // GPIO1'i ��k�� pini olarak ayarla
    GpioDataRegs.GPACLEAR.bit.GPIO1 = 1;  // GPIO1'i s�f�rla

    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 0;   // GPIO2'yi GPIO pini olarak ayarla
    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;    // GPIO2'yi ��k�� pini olarak ayarla
    GpioDataRegs.GPACLEAR.bit.GPIO2 = 1;  // GPIO2'yi s�f�rla

    // GPIO3 ve GPIO4'� giri� pini olarak ayarla
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 0;   // GPIO3'� GPIO pini olarak ayarla
    GpioCtrlRegs.GPADIR.bit.GPIO3 = 0;    // GPIO3'� giri� pini olarak ayarla

    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 0;   // GPIO4'� GPIO pini olarak ayarla
    GpioCtrlRegs.GPADIR.bit.GPIO4 = 0;    // GPIO4'� giri� pini olarak ayarla

    // GPIO3 ve GPIO4 i�in Pin Change Interrupt ayarlar�
    GpioCtrlRegs.GPACTRL.bit.QUALPRD3 = 0xFF; // Kesme filtre periyodu ayarla
    GpioCtrlRegs.GPAQSEL1.bit.GPIO3 = 2;  // GPIO3 i�in y�kselen kenar kesme se�
    GpioCtrlRegs.GPAQSEL1.bit.GPIO4 = 2;  // GPIO4 i�in y�kselen kenar kesme se�

    // GPIO3 ve GPIO4 Pin Change Interrupt ayarlar�
    GpioIntRegs.GPIOXINT1SEL.bit.GPIOSEL = 3; // XINT1 i�in GPIO3'� se�
    GpioIntRegs.GPIOXINT2SEL.bit.GPIOSEL = 4; // XINT2 i�in GPIO4'� se�

    // Pin Change Interrupt ayarlar�
    XIntruptRegs.XINT1CR.bit.POLARITY = 1; // Y�kselen kenar kesme
    XIntruptRegs.XINT1CR.bit.ENABLE = 1;   // XINT1'i etkinle�tir
    XIntruptRegs.XINT2CR.bit.POLARITY = 1; // Y�kselen kenar kesme
    XIntruptRegs.XINT2CR.bit.ENABLE = 1;   // XINT2'yi etkinle�tir

    EDIS;
}

void main(void)
{
    InitSysCtrl();  // Sistem ba�latma kodlar�

    SetupGpio();    // GPIO pinlerini yap�land�r

    while(1)
    {
        // Ana d�ng�de yap�lacak i�lemler
    }
}

// GPIO3 kesme hizmet altprogram� (ISR)
interrupt void gpio3_isr(void)
{
    GpioDataRegs.GPASET.bit.GPIO1 = 1;   // GPIO1'i set (LED'i yak)
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // PIE kesme bayra��n� temizle
}

// GPIO4 kesme hizmet altprogram� (ISR)
interrupt void gpio4_isr(void)
{
    GpioDataRegs.GPASET.bit.GPIO2 = 1;   // GPIO2'yi set (LED'i yak)
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // PIE kesme bayra��n� temizle
}

