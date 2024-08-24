
#include "DSP28x_Project.h"      // Cihaz Baþlýk Dosyasý ve Örnekler Dahil Dosyasý


#define CPU_FREQ    90E6         // CPU frekansý (90 MHz)
#define LSPCLK_FREQ (CPU_FREQ/4) // Düþük hýzlý çevresel saat frekansý
#define SCI_FREQ    100E3        // SCI (Seri Ýletiþim Arabirimi) frekansý
#define SCI_PRD     ((LSPCLK_FREQ/(SCI_FREQ*8))-1) // SCI periyodunun hesaplanmasý



__interrupt void sciaTxFifoIsr(void); // SCI-A TX FIFO kesme servis rutini

void scia_fifo_init(void); // SCI FIFO'yu baþlatmak için fonksiyon

Uint16 sdataA[2];    // SCI-A için gönderilecek veri dizisi

void main(void)
{

    Uint16 i;


    InitSysCtrl();  // Sistem kontrolünü baþlat (PLL, WatchDog, çevresel saatler)


    InitSciGpio();  // SCI-A ve SCI-B için GPI/O pinlerini baþlat


    DINT;  // CPU kesmelerini devre dýþý býrak


    InitPieCtrl();  // PIE kontrol kaydýný baþlat


    IER = 0x0000;  // CPU kesme etkinleþtirme kaydýný temizle
    IFR = 0x0000;  // CPU kesme bayraklarýný temizle


    InitPieVectTable();  // PIE vektör tablosunu baþlat


    EALLOW;  // EALLOW korumalý kayýtlara yazmak için izin ver
    PieVectTable.SCITXINTA = &sciaTxFifoIsr;  // SCI-A TX FIFO ISR'yi PIE vektör tablosuna atama
    EDIS;   // EALLOW korumasýný devre dýþý býrak


    scia_fifo_init();  // SCI-A FIFO'yu baþlat


    for(i = 0; i<2; i++)  // Gönderilecek veriyi baþlat
    {
        sdataA[i] = i;
    }


    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;   // PIE bloðunu etkinleþtir
    PieCtrlRegs.PIEIER9.bit.INTx1=1;     // PIE Grup 9, INT1 (SCI-A TX)
    IER = 0x100;                         // CPU INT (INT9) etkinleþtir
    EINT;                                // Genel kesmeleri etkinleþtir

    for(;;);  // Sonsuz döngü, ana program burada kalýr

}


__interrupt void sciaTxFifoIsr(void)  // SCI-A TX FIFO kesme servis rutini
{
    Uint16 i;
    for(i=0; i< 2; i++)  // Veriyi SCI-A TX tamponuna gönder
    {
        SciaRegs.SCITXBUF=sdataA[i];
    }

    for(i=0; i< 2; i++)  // Bir sonraki gönderi için veriyi güncelle
    {
        sdataA[i] = (sdataA[i]+1) & 0x00FF;
    }

    SciaRegs.SCIFFTX.bit.TXFFINTCLR=1;  // SCI TX kesme bayraðýný temizle
    PieCtrlRegs.PIEACK.all|=0x100;      // PIE kesme onayýný gönder
}



void scia_fifo_init()  // SCI-A FIFO'yu baþlatma fonksiyonu
{

    SciaRegs.SCICCR.all =0x0007;  // 1 durak bit, döngü yok, parite yok, 8 veri biti


    SciaRegs.SCICTL1.all =0x0003;  // TX'i etkinleþtir, RX ve SCICLK devre dýþý
    SciaRegs.SCICTL2.bit.TXINTENA =1;  // TX kesmesini etkinleþtir
    SciaRegs.SCIHBAUD = ((Uint16)SCI_PRD) >> 8;  // Yüksek bayt baud oraný
    SciaRegs.SCILBAUD = SCI_PRD;  // Düþük bayt baud oraný
    SciaRegs.SCICCR.bit.LOOPBKENA =1;   // Döngü geri beslemeyi etkinleþtir (test amaçlý)
    SciaRegs.SCIFFTX.all=0xC022;  // TX FIFO'yý baþlat
    SciaRegs.SCIFFRX.all=0x0020;  // RX FIFO'yu devre dýþý býrak
    SciaRegs.SCIFFCT.all=0x00;    // FIFO kontrol kaydý sýfýrlama

    SciaRegs.SCICTL1.all =0x0023;  // SCI'yi reset'ten çýkar ve baþlat
    SciaRegs.SCIFFTX.bit.TXFIFOXRESET=1;  // TX FIFO reseti
}

