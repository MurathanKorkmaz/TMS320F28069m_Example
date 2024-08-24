#include "DSP28x_Project.h"  // TMS320F28069 i�lemcisi i�in gerekli ba�l�k dosyas�n� i�erir

// Fonksiyon bildirimi
void btn_blink_setup();
void btn_blink_loop();

void HRPWM2_Config(Uint16);   // ePWM2 i�in y�ksek ��z�n�rl�kl� PWM yap�land�rma fonksiyonu
void InitEPwm2Gpio(void);     // ePWM2 i�in GPIO pinlerini ba�latma fonksiyonu

void HRPWM1_Config(Uint16 period);  // ePWM1 i�in y�ksek ��z�n�rl�kl� PWM yap�land�rma fonksiyonu
void InitEPwm1Gpio(void);           // ePWM1 i�in GPIO pinlerini ba�latma fonksiyonu

// Di�er fonksiyon bildirimleri
void initEPWM3(void);  // ePWM3'� ba�latma fonksiyonu
void initADCSOC(void); // ADC'yi ba�latma ve SOC (Start of Conversion) yap�land�rma fonksiyonu

// ADC sonu�lar�n� �l�eklendirmek i�in haritalama fonksiyonu
long map(long x, long in_min, long in_max, long out_min, long out_max);

// Kesme servis rutinleri (ISR) i�in bildirimler
__interrupt void cpu_timer0_isr(void);
__interrupt void adc_isr(void);

// Buton ve LED i�lemleri i�in fonksiyonlar
void btn_blink_setup(void);
void btn_blink_loop(void);

// Dijital giri�/��k�� i�lemleri i�in fonksiyonlar
int digitalRead(int pin);
void digitalWrite(int pin, int value);

// Buton durumlar�n� saklamak i�in de�i�kenler
int buttonState3;
int buttonState4;

// LED ve zamanlay�c� durumunu takip eden de�i�kenler
int led1_on = 0;
int timer_started = 0;

// Global de�i�kenler (ADC sonu�lar� ve PWM g�rev d�ng�leri)
Uint16 DutyCycle1 = 0, DutyCycle2 = 0, Voltage1 = 0, Voltage2 = 0;

void main(void)
{
    // Sistem kontrol�n� ba�lat�r (�rn. PLL yap�land�rma)
    InitSysCtrl();

    // ePWM1 ve ePWM2 i�in GPIO pinlerini ba�lat�r
    InitEPwm1Gpio();
    InitEPwm2Gpio();
    // Buton ve LED i�in gerekli GPIO pinlerini ba�lat�r
    btn_blink_setup();

    DINT;  // T�m kesmeleri devre d��� b�rak�r
    InitPieCtrl();  // PIE (Peripheral Interrupt Expansion) kontrol�n� ba�lat�r
    IER = 0x0000;   // Kesme enable register'� s�f�rlar
    IFR = 0x0000;   // Kesme bayrak register'�n� s�f�rlar

    // PIE vekt�r tablosunu ba�lat�r
    InitPieVectTable();

    EALLOW;  // Korumal� register'lara yazmak i�in gerekli
    PieVectTable.TINT0 = &cpu_timer0_isr;  // Timer0 kesmesini tan�mlar
    PieVectTable.ADCINT1 = &adc_isr;       // ADCINT1 kesmesini tan�mlar
    EDIS;    // Korumal� register'lara yazmay� devre d��� b�rak�r

    // CPU zamanlay�c�lar�n� ba�lat�r
    InitCpuTimers();

    // CPU Timer0'� yap�land�r�r (80 MHz CPU h�z�nda 1000 ms zaman a��m�)
    ConfigCpuTimer(&CpuTimer0, 80, 1000000);

    // Timer0'� ba�lat�r
    CpuTimer0Regs.TCR.all = 0x4001;

    // Kesme enable register'�nda grup 1 kesmelerini etkinle�tirir
    IER |= M_INT1;

    // PIE'deki INTx7 kesmesini etkinle�tirir
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

    // ADC mod�l�n� ba�lat�r ve ofset kalibrasyonu yapar
    InitAdc();
    AdcOffsetSelfCal();
    // ePWM3 ve ADC SOC (Start of Conversion) yap�land�r�r
    initEPWM3();
    initADCSOC();

    // Zaman tabanl� mod�l�n (Time Base Clock) senkronizasyonunu durdurur
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;

    // ePWM1 ve ePWM2 i�in y�ksek ��z�n�rl�kl� PWM yap�land�r�r
    HRPWM1_Config(10);
    HRPWM2_Config(10);

    // Zaman tabanl� mod�l�n (Time Base Clock) senkronizasyonunu ba�lat�r
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    // Grup 1 kesmelerini, global kesmeleri ve ger�ek zamanl� kesmeleri etkinle�tirir
    IER |= M_INT1;
    EINT;
    ERTM;

    // PIE'deki INTx1 kesmesini etkinle�tirir
    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;

    while(1)
    {
        // Ana d�ng�de buton ve LED i�lemlerini s�rekli olarak kontrol eder
        btn_blink_loop();
    }
}


//
// HRPWM2_Config - ePWM2 mod�l�n� y�ksek ��z�n�rl�kl� PWM (HRPWM) ile yap�land�r�r
//
void HRPWM2_Config(Uint16 period)
{
    EPwm2Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;    // An�nda y�kleme modunu ayarlar
    EPwm2Regs.TBPRD = period-1;                  // PWM periyodunu ayarlar
    EPwm2Regs.CMPA.half.CMPA = period / 2;       // �lk g�rev d�ng�s�n� %50 olarak ayarlar
    EPwm2Regs.CMPA.half.CMPAHR = (1 << 8);       // HRPWM uzant�s�n� ba�lat�r
    EPwm2Regs.CMPB = period / 2;                 // �kinci PWM kanal�n�n g�rev d�ng�s�n� %50 olarak ayarlar
    EPwm2Regs.TBPHS.all = 0;
    EPwm2Regs.TBCTR = 0;

    EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;   // Sayma modunu yukar� sayma olarak ayarlar
    EPwm2Regs.TBCTL.bit.PHSEN = TB_DISABLE;      // Faza senkronizeyi devre d��� b�rak�r (EPwm2 master modundad�r)
    EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_DISABLE;
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm2Regs.TBCTL.bit.CLKDIV = TB_DIV1;

    EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
    EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

    EPwm2Regs.AQCTLA.bit.ZRO = AQ_CLEAR;        // PWM sinyalini s�f�rlarken ��k��� s�f�rlama
    EPwm2Regs.AQCTLA.bit.CAU = AQ_SET;          // Kar��la�t�rma A yukar� saymada ��k��� set et
    EPwm2Regs.AQCTLB.bit.ZRO = AQ_CLEAR;
    EPwm2Regs.AQCTLB.bit.CBU = AQ_SET;

    EALLOW;
    EPwm2Regs.HRCNFG.all = 0x0;
    EPwm2Regs.HRCNFG.bit.EDGMODE = HR_REP;      // Y�kselen kenarda MEP kontrol�
    EPwm2Regs.HRCNFG.bit.CTLMODE = HR_CMP;
    EPwm2Regs.HRCNFG.bit.HRLOAD  = HR_CTR_ZERO;
    EDIS;
}


//
// HRPWM1_Config - ePWM1 mod�l�n� y�ksek ��z�n�rl�kl� PWM (HRPWM) ile yap�land�r�r
//
void HRPWM1_Config(Uint16 period)
{
    EPwm1Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;    // An�nda y�kleme modunu ayarlar
    EPwm1Regs.TBPRD = period-1;                  // PWM periyodunu ayarlar
    EPwm1Regs.CMPA.half.CMPA = period / 2;       // �lk g�rev d�ng�s�n� %50 olarak ayarlar
    EPwm1Regs.CMPA.half.CMPAHR = (1 << 8);       // HRPWM uzant�s�n� ba�lat�r
    EPwm1Regs.CMPB = period / 2;                 // �kinci PWM kanal�n�n g�rev d�ng�s�n� %50 olarak ayarlar
    EPwm1Regs.TBPHS.all = 0;
    EPwm1Regs.TBCTR = 0;

    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;   // Sayma modunu yukar� sayma olarak ayarlar
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;      // Faza senkronizeyi devre d��� b�rak�r (EPwm1 master modundad�r)
    EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_DISABLE;
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;

    EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

    EPwm1Regs.AQCTLA.bit.ZRO = AQ_CLEAR;        // PWM sinyalini s�f�rlarken ��k��� s�f�rlama
    EPwm1Regs.AQCTLA.bit.CAU = AQ_SET;          // Kar��la�t�rma A yukar� saymada ��k��� set et
    EPwm1Regs.AQCTLB.bit.ZRO = AQ_CLEAR;
    EPwm1Regs.AQCTLB.bit.CBU = AQ_SET;

    EALLOW;
    EPwm1Regs.HRCNFG.all = 0x0;
    EPwm1Regs.HRCNFG.bit.EDGMODE = HR_REP;      // Y�kselen kenarda MEP kontrol�
    EPwm1Regs.HRCNFG.bit.CTLMODE = HR_CMP;
    EPwm1Regs.HRCNFG.bit.HRLOAD  = HR_CTR_ZERO;
    EDIS;
}


//
// initEPWM3 - ePWM3 mod�l�n� ba�lat�r ve SOC (Start of Conversion) ayarlar�n� yapar
//
void initEPWM3(void)
{
    EPwm3Regs.ETSEL.bit.SOCAEN = 1;    // A grubunda SOC'u etkinle�tir
    EPwm3Regs.ETSEL.bit.SOCASEL = 4;   // CMPA'n�n yukar� saymas�nda SOC'u se�
    EPwm3Regs.ETPS.bit.SOCAPRD = 1;    // 1. olayda darbe �ret
    EPwm3Regs.CMPA.half.CMPAHR = 200;  // Kar��la�t�rma A de�erini ayarla
    EPwm3Regs.TBPRD = 0xFFFF;          // ePWM3 i�in periyodu ayarla
    EPwm3Regs.TBCTL.bit.CTRMODE = 0;   // Yukar� sayma modunda �al��t�r
}


//
// initADCSOC - ADC SOC (Start of Conversion) yap�land�rmas� yapar
//
void initADCSOC(void)
{
    EALLOW;
    AdcRegs.ADCCTL2.bit.ADCNONOVERLAP = 1; // ADC �rnekleme sinyalini �ak��mayacak �ekilde ayarla
    AdcRegs.ADCCTL1.bit.INTPULSEPOS = 1;   // ADC kesme sinyalinin d�n��t�rme tamamland�ktan sonra olu�mas�n� ayarla
    AdcRegs.INTSEL1N2.bit.INT1E = 1;       // ADCINT1 kesmesini etkinle�tir
    AdcRegs.INTSEL1N2.bit.INT1CONT = 0;    // ADCINT1 S�rekli modu devre d��� b�rak
    AdcRegs.INTSEL1N2.bit.INT1SEL = 1;     // ADCINT1 kesmesini SOC0'dan sonra tetiklenecek �ekilde ayarla
    AdcRegs.ADCSOC0CTL.bit.CHSEL = 4;      // SOC0 kanal se�imini ADCINA4 olarak ayarla
    AdcRegs.ADCSOC0CTL.bit.TRIGSEL = 5;    // SOC0 i�in tetikleme kayna��n� se�
    AdcRegs.ADCSOC1CTL.bit.TRIGSEL = 5;    // SOC1 i�in tetikleme kayna��n� se�
    AdcRegs.ADCSOC0CTL.bit.ACQPS = 6;      // SOC0 i�in �rnekleme penceresi geni�li�ini ayarla
    AdcRegs.ADCSOC1CTL.bit.ACQPS = 6;      // SOC1 i�in �rnekleme penceresi geni�li�ini ayarla
    EDIS;
}


//
// cpu_timer0_isr - CPU Timer0 i�in kesme servisi rutini (ISR)
//
__interrupt void cpu_timer0_isr(void)
{
    static int led1_timer = 0;  // LED zamanlay�c� de�i�keni

    if(timer_started)
    {
        led1_timer++;
        if(led1_timer >= 1000) // 1000 ms ge�ti�inde
        {
            GpioDataRegs.GPASET.bit.GPIO6 = 1; // GPIO6 ��k���n� 1 yap (LED'i yak)
            led1_timer = 0;
            led1_on = 0;
            timer_started = 0;
            CpuTimer0Regs.TCR.bit.TSS = 1; // Timer'� durdur
        }
    }

    CpuTimer0.InterruptCount++; // Kesme say�s�n� art�r

    // Grubun 1'den daha fazla kesme almas� i�in bu kesmeyi onaylay�n
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // Kesme onay�n� g�nder
}


//
// adc_isr - ADC i�in kesme servisi rutini (ISR)
//
__interrupt void adc_isr(void)
{
    Voltage1 = AdcResult.ADCRESULT0; // ADC'den okunan de�eri al
    Voltage2 = AdcResult.ADCRESULT0;

    // ADC voltaj�n� g�rev d�ng�s�ne e�le
    DutyCycle1 = map(Voltage1, 0, 4095, 0, 255);
    DutyCycle2 = map(Voltage2, 0, 4095, 0, 255);

    EPwm1Regs.CMPA.half.CMPAHR = DutyCycle1 << 8; // DutyCycle'� PWM'e uygula
    EPwm2Regs.CMPA.half.CMPAHR = DutyCycle2 << 8; // DutyCycle'� PWM'e uygula

    AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // ADC kesme bayra��n� temizle

    if(1 == AdcRegs.ADCINTOVF.bit.ADCINT1)
    {
        AdcRegs.ADCINTOVFCLR.bit.ADCINT1 = 1; // INT1 ta�ma bayra��n� temizle
        AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // INT1 bayra��n� temizle
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // PIE mod�l�ne kesmenin i�lendi�ini bildir

    return;
}


//
// btn_blink_setup - Buton ve LED i�in gerekli GPIO ayarlar�n� yapar
//
void btn_blink_setup()
{
    EALLOW;

    // GPIO6 ve GPIO10 pinlerini giri� pini olarak ayarla
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 0;  // GPIO6'u GPIO pini olarak ayarla
    GpioCtrlRegs.GPAMUX1.bit.GPIO10 = 0;  // GPIO10'u GPIO pini olarak ayarla
    GpioCtrlRegs.GPADIR.bit.GPIO6 = 0;   // GPIO6'u giri� pini olarak ayarla
    GpioCtrlRegs.GPADIR.bit.GPIO10 = 0;   // GPIO10'u giri� pini olarak ayarla

    // GPIO9'u dijital ��k�� olarak ayarla
    GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 0;  // GPIO9'u GPIO pini olarak ayarla
    GpioCtrlRegs.GPADIR.bit.GPIO9 = 1;   // GPIO9'u ��k�� pini olarak ayarla

    EDIS;
}


//
// btn_blink_loop - Buton durumlar�n� kontrol eder ve LED'i yakar/s�nd�r�r
//
void btn_blink_loop()
{
    buttonState3 = digitalRead(6); // GPIO6 pinini oku
    buttonState4 = digitalRead(10); // GPIO10 pinini oku

    // Butonlardan birine bas�ld�ysa ve LED yanm�yorsa
    if((buttonState3 == 1 || buttonState4 == 1) && !led1_on)
    {
        digitalWrite(9, 1); // GPIO9 ��k���n� 1 yap (LED'i yak)
        led1_on = 1;
        timer_started = 1;
        CpuTimer0Regs.TCR.bit.TSS = 0; // Timer'� ba�lat
    }
}


//
// digitalRead - GPIO pininin durumunu okur
//
int digitalRead(int pin)
{
    if(pin == 6)
    {
        return GpioDataRegs.GPADAT.bit.GPIO6; // GPIO6 pininin durumunu oku
    }
    else if(pin == 10)
    {
        return GpioDataRegs.GPADAT.bit.GPIO10; // GPIO10 pininin durumunu oku
    }
    return 0;
}


//
// digitalWrite - GPIO pininin ��k�� durumunu ayarlar
//
void digitalWrite(int pin, int value)
{
    if(pin == 9)
    {
        if(value == 0)
        {
            GpioDataRegs.GPACLEAR.bit.GPIO9 = 1; // GPIO9 ��k���n� 0 yap (LED'i s�nd�r)
        }
        else if(value == 1)
        {
            GpioDataRegs.GPASET.bit.GPIO9 = 1; // GPIO9 ��k���n� 1 yap (LED'i yak)
        }
    }
}


//
// map - Bir de�eri bir aral�ktan ba�ka bir aral��a e�ler
//
long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
