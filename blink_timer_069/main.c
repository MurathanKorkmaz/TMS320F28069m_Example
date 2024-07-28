
#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File


__interrupt void cpu_timer0_isr(void);


void main(void)
{

    InitSysCtrl();


    //
    DINT;


    //
    InitPieCtrl();


    //
    IER = 0x0000;
    IFR = 0x0000;


    //
    InitPieVectTable();


    //
    EALLOW;    // This is needed to write to EALLOW protected registers
    PieVectTable.TINT0 = &cpu_timer0_isr;
    EDIS;      // This is needed to disable write to EALLOW protected registers


    //
    InitCpuTimers();   // For this example, only initialize the Cpu Timers


    //
    ConfigCpuTimer(&CpuTimer0, 80, 50000);


    //


    //
    CpuTimer0Regs.TCR.all = 0x4001;


    //


    //
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 0;  // GPIO1'i GPIO pini olarak ayarla
    GpioCtrlRegs.GPADIR.bit.GPIO1 = 1;   // GPIO1'i çýkýþ pini olarak ayarla
    EDIS;


    //
    IER |= M_INT1;


    //
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;


    //
    EINT;   // Enable Global interrupt INTM
    ERTM;   // Enable Global realtime interrupt DBGM


    //
    for(;;);
}


//
__interrupt void
cpu_timer0_isr(void)
{
    CpuTimer0.InterruptCount++;

    //
    // Toggle GPIO34 once per 500 milliseconds
    //
    GpioDataRegs.GPATOGGLE.bit.GPIO1 = 1;

    //
    // Acknowledge this interrupt to receive more interrupts from group 1
    //
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

//
// End of File
//

