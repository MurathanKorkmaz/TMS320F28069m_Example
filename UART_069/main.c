
#include "DSP28x_Project.h"      // Cihaz Ba�l�k Dosyas� ve �rnekler Dahil Dosyas�


#define CPU_FREQ    90E6         // CPU frekans� (90 MHz)
#define LSPCLK_FREQ (CPU_FREQ/4) // D���k h�zl� �evresel saat frekans�
#define SCI_FREQ    100E3        // SCI (Seri �leti�im Arabirimi) frekans�
#define SCI_PRD     ((LSPCLK_FREQ/(SCI_FREQ*8))-1) // SCI periyodunun hesaplanmas�



__interrupt void sciaTxFifoIsr(void); // SCI-A TX FIFO kesme servis rutini

void scia_fifo_init(void); // SCI FIFO'yu ba�latmak i�in fonksiyon

Uint16 sdataA[2];    // SCI-A i�in g�nderilecek veri dizisi

void main(void)
{

    Uint16 i;


    InitSysCtrl();  // Sistem kontrol�n� ba�lat (PLL, WatchDog, �evresel saatler)


    InitSciGpio();  // SCI-A ve SCI-B i�in GPI/O pinlerini ba�lat


    DINT;  // CPU kesmelerini devre d��� b�rak


    InitPieCtrl();  // PIE kontrol kayd�n� ba�lat


    IER = 0x0000;  // CPU kesme etkinle�tirme kayd�n� temizle
    IFR = 0x0000;  // CPU kesme bayraklar�n� temizle


    InitPieVectTable();  // PIE vekt�r tablosunu ba�lat


    EALLOW;  // EALLOW korumal� kay�tlara yazmak i�in izin ver
    PieVectTable.SCITXINTA = &sciaTxFifoIsr;  // SCI-A TX FIFO ISR'yi PIE vekt�r tablosuna atama
    EDIS;   // EALLOW korumas�n� devre d��� b�rak


    scia_fifo_init();  // SCI-A FIFO'yu ba�lat


    for(i = 0; i<2; i++)  // G�nderilecek veriyi ba�lat
    {
        sdataA[i] = i;
    }


    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;   // PIE blo�unu etkinle�tir
    PieCtrlRegs.PIEIER9.bit.INTx1=1;     // PIE Grup 9, INT1 (SCI-A TX)
    IER = 0x100;                         // CPU INT (INT9) etkinle�tir
    EINT;                                // Genel kesmeleri etkinle�tir

    for(;;);  // Sonsuz d�ng�, ana program burada kal�r

}


__interrupt void sciaTxFifoIsr(void)  // SCI-A TX FIFO kesme servis rutini
{
    Uint16 i;
    for(i=0; i< 2; i++)  // Veriyi SCI-A TX tamponuna g�nder
    {
        SciaRegs.SCITXBUF=sdataA[i];
    }

    for(i=0; i< 2; i++)  // Bir sonraki g�nderi i�in veriyi g�ncelle
    {
        sdataA[i] = (sdataA[i]+1) & 0x00FF;
    }

    SciaRegs.SCIFFTX.bit.TXFFINTCLR=1;  // SCI TX kesme bayra��n� temizle
    PieCtrlRegs.PIEACK.all|=0x100;      // PIE kesme onay�n� g�nder
}



void scia_fifo_init()  // SCI-A FIFO'yu ba�latma fonksiyonu
{

    SciaRegs.SCICCR.all =0x0007;  // 1 durak bit, d�ng� yok, parite yok, 8 veri biti


    SciaRegs.SCICTL1.all =0x0003;  // TX'i etkinle�tir, RX ve SCICLK devre d���
    SciaRegs.SCICTL2.bit.TXINTENA =1;  // TX kesmesini etkinle�tir
    SciaRegs.SCIHBAUD = ((Uint16)SCI_PRD) >> 8;  // Y�ksek bayt baud oran�
    SciaRegs.SCILBAUD = SCI_PRD;  // D���k bayt baud oran�
    SciaRegs.SCICCR.bit.LOOPBKENA =1;   // D�ng� geri beslemeyi etkinle�tir (test ama�l�)
    SciaRegs.SCIFFTX.all=0xC022;  // TX FIFO'y� ba�lat
    SciaRegs.SCIFFRX.all=0x0020;  // RX FIFO'yu devre d��� b�rak
    SciaRegs.SCIFFCT.all=0x00;    // FIFO kontrol kayd� s�f�rlama

    SciaRegs.SCICTL1.all =0x0023;  // SCI'yi reset'ten ��kar ve ba�lat
    SciaRegs.SCIFFTX.bit.TXFIFOXRESET=1;  // TX FIFO reseti
}

