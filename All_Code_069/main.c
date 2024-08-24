#include "DSP28x_Project.h"  // TMS320F28069 iþlemcisi için gerekli baþlýk dosyasýný içerir

// Fonksiyon bildirimi
void btn_blink_setup();
void btn_blink_loop();

void HRPWM2_Config(Uint16);   // ePWM2 için yüksek çözünürlüklü PWM yapýlandýrma fonksiyonu
void InitEPwm2Gpio(void);     // ePWM2 için GPIO pinlerini baþlatma fonksiyonu

void HRPWM1_Config(Uint16 period);  // ePWM1 için yüksek çözünürlüklü PWM yapýlandýrma fonksiyonu
void InitEPwm1Gpio(void);           // ePWM1 için GPIO pinlerini baþlatma fonksiyonu

// Diðer fonksiyon bildirimleri
void initEPWM3(void);  // ePWM3'ü baþlatma fonksiyonu
void initADCSOC(void); // ADC'yi baþlatma ve SOC (Start of Conversion) yapýlandýrma fonksiyonu

// ADC sonuçlarýný ölçeklendirmek için haritalama fonksiyonu
long map(long x, long in_min, long in_max, long out_min, long out_max);

// Kesme servis rutinleri (ISR) için bildirimler
__interrupt void cpu_timer0_isr(void);
__interrupt void adc_isr(void);

// Buton ve LED iþlemleri için fonksiyonlar
void btn_blink_setup(void);
void btn_blink_loop(void);

// Dijital giriþ/çýkýþ iþlemleri için fonksiyonlar
int digitalRead(int pin);
void digitalWrite(int pin, int value);

// Buton durumlarýný saklamak için deðiþkenler
int buttonState3;
int buttonState4;

// LED ve zamanlayýcý durumunu takip eden deðiþkenler
int led1_on = 0;
int timer_started = 0;

// Global deðiþkenler (ADC sonuçlarý ve PWM görev döngüleri)
Uint16 DutyCycle1 = 0, DutyCycle2 = 0, Voltage1 = 0, Voltage2 = 0;

void main(void)
{
    // Sistem kontrolünü baþlatýr (örn. PLL yapýlandýrma)
    InitSysCtrl();

    // ePWM1 ve ePWM2 için GPIO pinlerini baþlatýr
    InitEPwm1Gpio();
    InitEPwm2Gpio();
    // Buton ve LED için gerekli GPIO pinlerini baþlatýr
    btn_blink_setup();

    DINT;  // Tüm kesmeleri devre dýþý býrakýr
    InitPieCtrl();  // PIE (Peripheral Interrupt Expansion) kontrolünü baþlatýr
    IER = 0x0000;   // Kesme enable register'ý sýfýrlar
    IFR = 0x0000;   // Kesme bayrak register'ýný sýfýrlar

    // PIE vektör tablosunu baþlatýr
    InitPieVectTable();

    EALLOW;  // Korumalý register'lara yazmak için gerekli
    PieVectTable.TINT0 = &cpu_timer0_isr;  // Timer0 kesmesini tanýmlar
    PieVectTable.ADCINT1 = &adc_isr;       // ADCINT1 kesmesini tanýmlar
    EDIS;    // Korumalý register'lara yazmayý devre dýþý býrakýr

    // CPU zamanlayýcýlarýný baþlatýr
    InitCpuTimers();

    // CPU Timer0'ý yapýlandýrýr (80 MHz CPU hýzýnda 1000 ms zaman aþýmý)
    ConfigCpuTimer(&CpuTimer0, 80, 1000000);

    // Timer0'ý baþlatýr
    CpuTimer0Regs.TCR.all = 0x4001;

    // Kesme enable register'ýnda grup 1 kesmelerini etkinleþtirir
    IER |= M_INT1;

    // PIE'deki INTx7 kesmesini etkinleþtirir
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

    // ADC modülünü baþlatýr ve ofset kalibrasyonu yapar
    InitAdc();
    AdcOffsetSelfCal();
    // ePWM3 ve ADC SOC (Start of Conversion) yapýlandýrýr
    initEPWM3();
    initADCSOC();

    // Zaman tabanlý modülün (Time Base Clock) senkronizasyonunu durdurur
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;

    // ePWM1 ve ePWM2 için yüksek çözünürlüklü PWM yapýlandýrýr
    HRPWM1_Config(10);
    HRPWM2_Config(10);

    // Zaman tabanlý modülün (Time Base Clock) senkronizasyonunu baþlatýr
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    // Grup 1 kesmelerini, global kesmeleri ve gerçek zamanlý kesmeleri etkinleþtirir
    IER |= M_INT1;
    EINT;
    ERTM;

    // PIE'deki INTx1 kesmesini etkinleþtirir
    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;

    while(1)
    {
        // Ana döngüde buton ve LED iþlemlerini sürekli olarak kontrol eder
        btn_blink_loop();
    }
}


//
// HRPWM2_Config - ePWM2 modülünü yüksek çözünürlüklü PWM (HRPWM) ile yapýlandýrýr
//
void HRPWM2_Config(Uint16 period)
{
    EPwm2Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;    // Anýnda yükleme modunu ayarlar
    EPwm2Regs.TBPRD = period-1;                  // PWM periyodunu ayarlar
    EPwm2Regs.CMPA.half.CMPA = period / 2;       // Ýlk görev döngüsünü %50 olarak ayarlar
    EPwm2Regs.CMPA.half.CMPAHR = (1 << 8);       // HRPWM uzantýsýný baþlatýr
    EPwm2Regs.CMPB = period / 2;                 // Ýkinci PWM kanalýnýn görev döngüsünü %50 olarak ayarlar
    EPwm2Regs.TBPHS.all = 0;
    EPwm2Regs.TBCTR = 0;

    EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;   // Sayma modunu yukarý sayma olarak ayarlar
    EPwm2Regs.TBCTL.bit.PHSEN = TB_DISABLE;      // Faza senkronizeyi devre dýþý býrakýr (EPwm2 master modundadýr)
    EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_DISABLE;
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm2Regs.TBCTL.bit.CLKDIV = TB_DIV1;

    EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
    EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

    EPwm2Regs.AQCTLA.bit.ZRO = AQ_CLEAR;        // PWM sinyalini sýfýrlarken çýkýþý sýfýrlama
    EPwm2Regs.AQCTLA.bit.CAU = AQ_SET;          // Karþýlaþtýrma A yukarý saymada çýkýþý set et
    EPwm2Regs.AQCTLB.bit.ZRO = AQ_CLEAR;
    EPwm2Regs.AQCTLB.bit.CBU = AQ_SET;

    EALLOW;
    EPwm2Regs.HRCNFG.all = 0x0;
    EPwm2Regs.HRCNFG.bit.EDGMODE = HR_REP;      // Yükselen kenarda MEP kontrolü
    EPwm2Regs.HRCNFG.bit.CTLMODE = HR_CMP;
    EPwm2Regs.HRCNFG.bit.HRLOAD  = HR_CTR_ZERO;
    EDIS;
}


//
// HRPWM1_Config - ePWM1 modülünü yüksek çözünürlüklü PWM (HRPWM) ile yapýlandýrýr
//
void HRPWM1_Config(Uint16 period)
{
    EPwm1Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;    // Anýnda yükleme modunu ayarlar
    EPwm1Regs.TBPRD = period-1;                  // PWM periyodunu ayarlar
    EPwm1Regs.CMPA.half.CMPA = period / 2;       // Ýlk görev döngüsünü %50 olarak ayarlar
    EPwm1Regs.CMPA.half.CMPAHR = (1 << 8);       // HRPWM uzantýsýný baþlatýr
    EPwm1Regs.CMPB = period / 2;                 // Ýkinci PWM kanalýnýn görev döngüsünü %50 olarak ayarlar
    EPwm1Regs.TBPHS.all = 0;
    EPwm1Regs.TBCTR = 0;

    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;   // Sayma modunu yukarý sayma olarak ayarlar
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;      // Faza senkronizeyi devre dýþý býrakýr (EPwm1 master modundadýr)
    EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_DISABLE;
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;

    EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

    EPwm1Regs.AQCTLA.bit.ZRO = AQ_CLEAR;        // PWM sinyalini sýfýrlarken çýkýþý sýfýrlama
    EPwm1Regs.AQCTLA.bit.CAU = AQ_SET;          // Karþýlaþtýrma A yukarý saymada çýkýþý set et
    EPwm1Regs.AQCTLB.bit.ZRO = AQ_CLEAR;
    EPwm1Regs.AQCTLB.bit.CBU = AQ_SET;

    EALLOW;
    EPwm1Regs.HRCNFG.all = 0x0;
    EPwm1Regs.HRCNFG.bit.EDGMODE = HR_REP;      // Yükselen kenarda MEP kontrolü
    EPwm1Regs.HRCNFG.bit.CTLMODE = HR_CMP;
    EPwm1Regs.HRCNFG.bit.HRLOAD  = HR_CTR_ZERO;
    EDIS;
}


//
// initEPWM3 - ePWM3 modülünü baþlatýr ve SOC (Start of Conversion) ayarlarýný yapar
//
void initEPWM3(void)
{
    EPwm3Regs.ETSEL.bit.SOCAEN = 1;    // A grubunda SOC'u etkinleþtir
    EPwm3Regs.ETSEL.bit.SOCASEL = 4;   // CMPA'nýn yukarý saymasýnda SOC'u seç
    EPwm3Regs.ETPS.bit.SOCAPRD = 1;    // 1. olayda darbe üret
    EPwm3Regs.CMPA.half.CMPAHR = 200;  // Karþýlaþtýrma A deðerini ayarla
    EPwm3Regs.TBPRD = 0xFFFF;          // ePWM3 için periyodu ayarla
    EPwm3Regs.TBCTL.bit.CTRMODE = 0;   // Yukarý sayma modunda çalýþtýr
}


//
// initADCSOC - ADC SOC (Start of Conversion) yapýlandýrmasý yapar
//
void initADCSOC(void)
{
    EALLOW;
    AdcRegs.ADCCTL2.bit.ADCNONOVERLAP = 1; // ADC örnekleme sinyalini çakýþmayacak þekilde ayarla
    AdcRegs.ADCCTL1.bit.INTPULSEPOS = 1;   // ADC kesme sinyalinin dönüþtürme tamamlandýktan sonra oluþmasýný ayarla
    AdcRegs.INTSEL1N2.bit.INT1E = 1;       // ADCINT1 kesmesini etkinleþtir
    AdcRegs.INTSEL1N2.bit.INT1CONT = 0;    // ADCINT1 Sürekli modu devre dýþý býrak
    AdcRegs.INTSEL1N2.bit.INT1SEL = 1;     // ADCINT1 kesmesini SOC0'dan sonra tetiklenecek þekilde ayarla
    AdcRegs.ADCSOC0CTL.bit.CHSEL = 4;      // SOC0 kanal seçimini ADCINA4 olarak ayarla
    AdcRegs.ADCSOC0CTL.bit.TRIGSEL = 5;    // SOC0 için tetikleme kaynaðýný seç
    AdcRegs.ADCSOC1CTL.bit.TRIGSEL = 5;    // SOC1 için tetikleme kaynaðýný seç
    AdcRegs.ADCSOC0CTL.bit.ACQPS = 6;      // SOC0 için örnekleme penceresi geniþliðini ayarla
    AdcRegs.ADCSOC1CTL.bit.ACQPS = 6;      // SOC1 için örnekleme penceresi geniþliðini ayarla
    EDIS;
}


//
// cpu_timer0_isr - CPU Timer0 için kesme servisi rutini (ISR)
//
__interrupt void cpu_timer0_isr(void)
{
    static int led1_timer = 0;  // LED zamanlayýcý deðiþkeni

    if(timer_started)
    {
        led1_timer++;
        if(led1_timer >= 1000) // 1000 ms geçtiðinde
        {
            GpioDataRegs.GPASET.bit.GPIO6 = 1; // GPIO6 çýkýþýný 1 yap (LED'i yak)
            led1_timer = 0;
            led1_on = 0;
            timer_started = 0;
            CpuTimer0Regs.TCR.bit.TSS = 1; // Timer'ý durdur
        }
    }

    CpuTimer0.InterruptCount++; // Kesme sayýsýný artýr

    // Grubun 1'den daha fazla kesme almasý için bu kesmeyi onaylayýn
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // Kesme onayýný gönder
}


//
// adc_isr - ADC için kesme servisi rutini (ISR)
//
__interrupt void adc_isr(void)
{
    Voltage1 = AdcResult.ADCRESULT0; // ADC'den okunan deðeri al
    Voltage2 = AdcResult.ADCRESULT0;

    // ADC voltajýný görev döngüsüne eþle
    DutyCycle1 = map(Voltage1, 0, 4095, 0, 255);
    DutyCycle2 = map(Voltage2, 0, 4095, 0, 255);

    EPwm1Regs.CMPA.half.CMPAHR = DutyCycle1 << 8; // DutyCycle'ý PWM'e uygula
    EPwm2Regs.CMPA.half.CMPAHR = DutyCycle2 << 8; // DutyCycle'ý PWM'e uygula

    AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // ADC kesme bayraðýný temizle

    if(1 == AdcRegs.ADCINTOVF.bit.ADCINT1)
    {
        AdcRegs.ADCINTOVFCLR.bit.ADCINT1 = 1; // INT1 taþma bayraðýný temizle
        AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // INT1 bayraðýný temizle
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // PIE modülüne kesmenin iþlendiðini bildir

    return;
}


//
// btn_blink_setup - Buton ve LED için gerekli GPIO ayarlarýný yapar
//
void btn_blink_setup()
{
    EALLOW;

    // GPIO6 ve GPIO10 pinlerini giriþ pini olarak ayarla
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 0;  // GPIO6'u GPIO pini olarak ayarla
    GpioCtrlRegs.GPAMUX1.bit.GPIO10 = 0;  // GPIO10'u GPIO pini olarak ayarla
    GpioCtrlRegs.GPADIR.bit.GPIO6 = 0;   // GPIO6'u giriþ pini olarak ayarla
    GpioCtrlRegs.GPADIR.bit.GPIO10 = 0;   // GPIO10'u giriþ pini olarak ayarla

    // GPIO9'u dijital çýkýþ olarak ayarla
    GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 0;  // GPIO9'u GPIO pini olarak ayarla
    GpioCtrlRegs.GPADIR.bit.GPIO9 = 1;   // GPIO9'u çýkýþ pini olarak ayarla

    EDIS;
}


//
// btn_blink_loop - Buton durumlarýný kontrol eder ve LED'i yakar/söndürür
//
void btn_blink_loop()
{
    buttonState3 = digitalRead(6); // GPIO6 pinini oku
    buttonState4 = digitalRead(10); // GPIO10 pinini oku

    // Butonlardan birine basýldýysa ve LED yanmýyorsa
    if((buttonState3 == 1 || buttonState4 == 1) && !led1_on)
    {
        digitalWrite(9, 1); // GPIO9 çýkýþýný 1 yap (LED'i yak)
        led1_on = 1;
        timer_started = 1;
        CpuTimer0Regs.TCR.bit.TSS = 0; // Timer'ý baþlat
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
// digitalWrite - GPIO pininin çýkýþ durumunu ayarlar
//
void digitalWrite(int pin, int value)
{
    if(pin == 9)
    {
        if(value == 0)
        {
            GpioDataRegs.GPACLEAR.bit.GPIO9 = 1; // GPIO9 çýkýþýný 0 yap (LED'i söndür)
        }
        else if(value == 1)
        {
            GpioDataRegs.GPASET.bit.GPIO9 = 1; // GPIO9 çýkýþýný 1 yap (LED'i yak)
        }
    }
}


//
// map - Bir deðeri bir aralýktan baþka bir aralýða eþler
//
long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
