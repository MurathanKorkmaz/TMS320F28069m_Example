#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include "F2806x_Device.h"         // F2806x Headerfile
#include "F2806x_Examples.h"       // F2806x Examples Headerfile
#include "F2806x_EPwm_defines.h"

//
// Function Prototypes
//
__interrupt void adc_isr(void);
void Adc_Config(void);

void HRPWM2_Config(Uint16);
long map(long x, long in_min, long in_max, long out_min, long out_max);


Uint16 Voltage1;
Uint16 DutyCycle;

void main(void)
{

    InitSysCtrl();

    InitEPwm2Gpio();

    DINT;


    InitPieCtrl();


    IER = 0x0000;
    IFR = 0x0000;


    InitPieVectTable();


    EALLOW;  // This is needed to write to EALLOW protected register
    PieVectTable.ADCINT1 = &adc_isr;
    EDIS;    // This is needed to disable write to EALLOW protected registers

    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;

    HRPWM2_Config(10);      // ePWM1 target, Period = 10

    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;


    InitAdc();  // For this example, init the ADC
    AdcOffsetSelfCal();


    PieCtrlRegs.PIEIER1.bit.INTx1 = 1; // Enable INT 1.1 in the PIE
    IER |= M_INT1;                     // Enable CPU Interrupt 1
    EINT;                              // Enable Global interrupt INTM
    ERTM;                              // Enable Global realtime interrupt DBGM



    EALLOW;
    AdcRegs.ADCCTL2.bit.ADCNONOVERLAP = 1; // Enable non-overlap mode


    AdcRegs.ADCCTL1.bit.INTPULSEPOS = 1;

    AdcRegs.INTSEL1N2.bit.INT1E     = 1;  // Enabled ADCINT1
    AdcRegs.INTSEL1N2.bit.INT1CONT  = 0;  // Disable ADCINT1 Continuous mode


    AdcRegs.INTSEL1N2.bit.INT1SEL   = 1;

    AdcRegs.ADCSOC0CTL.bit.CHSEL    = 4;  // set SOC0 channel select to ADCINA4


    AdcRegs.ADCSOC0CTL.bit.TRIGSEL  = 5;


    AdcRegs.ADCSOC1CTL.bit.TRIGSEL  = 5;


    AdcRegs.ADCSOC0CTL.bit.ACQPS    = 6;


    AdcRegs.ADCSOC1CTL.bit.ACQPS    = 6;
    EDIS;


    EPwm1Regs.ETSEL.bit.SOCAEN  = 1;        // Enable SOC on A group
    EPwm1Regs.ETSEL.bit.SOCASEL = 4;        // Select SOC from CMPA on upcount
    EPwm1Regs.ETPS.bit.SOCAPRD  = 1;        // Generate pulse on 1st event
    EPwm1Regs.CMPA.half.CMPAHR  = 200;   // Set compare A value
    EPwm1Regs.TBPRD             = 0xFFFF;   // Set period for ePWM1
    EPwm1Regs.TBCTL.bit.CTRMODE = 0;        // count up and start


    while(1)
    {

    }
}


__interrupt void
adc_isr(void)
{
    Voltage1 = AdcResult.ADCRESULT0;

    DutyCycle = map(Voltage1,4095,7,0,255 );
    EPwm2Regs.CMPA.half.CMPAHR = DutyCycle << 8;


    AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;

    return;
}

void HRPWM2_Config(Uint16 period)
{
    //
    // ePWM2 register configuration with HRPWM
    // ePWM2A toggle low/high with MEP control on Rising edge
    //
    EPwm2Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;    // set Immediate load
    EPwm2Regs.TBPRD = period-1;                  // PWM frequency = 1 / period
    EPwm2Regs.CMPA.half.CMPA = period / 2;       // set duty 50% initially
    EPwm2Regs.CMPA.half.CMPAHR = (1 << 8);       // initialize HRPWM extension
    EPwm2Regs.CMPB = period / 2;                 // set duty 50% initially
    EPwm2Regs.TBPHS.all = 0;
    EPwm2Regs.TBCTR = 0;

    EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;
    EPwm2Regs.TBCTL.bit.PHSEN = TB_DISABLE;      // EPwm2 is the Master
    EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_DISABLE;
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm2Regs.TBCTL.bit.CLKDIV = TB_DIV1;

    EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
    EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

    EPwm2Regs.AQCTLA.bit.ZRO = AQ_CLEAR;        // PWM toggle low/high
    EPwm2Regs.AQCTLA.bit.CAU = AQ_SET;
    EPwm2Regs.AQCTLB.bit.ZRO = AQ_CLEAR;
    EPwm2Regs.AQCTLB.bit.CBU = AQ_SET;

    EALLOW;
    EPwm2Regs.HRCNFG.all = 0x0;
    EPwm2Regs.HRCNFG.bit.EDGMODE = HR_REP;      // MEP control on Rising edge
    EPwm2Regs.HRCNFG.bit.CTLMODE = HR_CMP;
    EPwm2Regs.HRCNFG.bit.HRLOAD  = HR_CTR_ZERO;
    EDIS;
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


