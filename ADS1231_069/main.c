
#include "DSP28x_Project.h"

#define SCLK    3   // OUTPUT   1 GPIO3 (Clock Signal for ADS)
#define DOUT    1   // INPUT    0 GPIO1 (Data Output from ADS)
#define SPEED   0   // OUTPUT   0 GPIO0 (Speed Selection for ADS)
#define PDWN    40  // OUTPUT   1 GPIO40 (Power Down Control for ADS)

// Function Prototypes
void Defines_Pin_ADS();
void ADS_begin(int highSpeed, int power);
int ADS_read();

// Digital I/O functions
int digitalRead(int pin);
void digitalWrite(int pin, int value);

// Global variables
volatile unsigned long millisCounter = 0; // Counter to store milliseconds
Uint32 val = 0; // Variable to store the read value from ADS
int maskResult = 0; // Variable to store the result of ADS_read function

// Interrupt Service Routine (ISR) Prototypes
__interrupt void cpu_timer0_isr(void);

// millis Function Prototype
unsigned long millis(void);

void main(void)
{
    // System initialization code
    InitSysCtrl(); // Initialize system control

    Defines_Pin_ADS(); // Configure GPIO pins for ADS
    ADS_begin(0, 1); // Initialize ADS with normal speed and power on

    DINT; // Disable CPU interrupts

    InitPieCtrl(); // Initialize PIE control registers

    IER = 0x0000; // Disable all CPU interrupts
    IFR = 0x0000; // Clear all CPU interrupt flags

    InitPieVectTable(); // Initialize the PIE vector table

    EALLOW;    // This is needed to write to EALLOW protected registers
    PieVectTable.TINT0 = &cpu_timer0_isr; // Map ISR to the timer interrupt
    EDIS;      // This is needed to disable write to EALLOW protected registers

    InitCpuTimers();   // Initialize CPU timers

    ConfigCpuTimer(&CpuTimer0, 80, 1000); // Configure CPU Timer0 to interrupt every 1ms

    CpuTimer0Regs.TCR.all = 0x4001; // Start Timer0

    IER |= M_INT1; // Enable CPU interrupt 1

    PieCtrlRegs.PIEIER1.bit.INTx7 = 1; // Enable PIE Group 1, INT7

    EINT;   // Enable Global interrupt INTM
    ERTM;   // Enable Global real-time interrupt DBGM

    while(1)
    {
        static unsigned long previousMillis = 0;
        unsigned long currentMillis = millis(); // Get current time in milliseconds

        // Check if 100 milliseconds have passed
        if(currentMillis - previousMillis >= 100)
        {
            previousMillis = currentMillis;
            maskResult = ADS_read(); // Read data from ADS
            // The actual value is stored in the global variable 'val'
        }
    }
}

void Defines_Pin_ADS()
{
    // Configure GPIO pins for ADS communication
    EALLOW; // Enable write access to protected registers

    // Configure GPIO1 as input pin for DOUT
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 0;  // Set GPIO1 as GPIO pin
    GpioCtrlRegs.GPADIR.bit.GPIO1 = 0;   // Set GPIO1 as input

    // Configure GPIO0 as output pin for SPEED
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 0;  // Set GPIO0 as GPIO pin
    GpioCtrlRegs.GPADIR.bit.GPIO0 = 1;   // Set GPIO0 as output

    // Configure GPIO3 as output pin for SCLK
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 0;  // Set GPIO3 as GPIO pin
    GpioCtrlRegs.GPADIR.bit.GPIO3 = 1;   // Set GPIO3 as output

    // Configure GPIO40 as output pin for PDWN
    GpioCtrlRegs.GPBMUX1.bit.GPIO40 = 0;  // Set GPIO40 as GPIO pin
    GpioCtrlRegs.GPBDIR.bit.GPIO40 = 1;   // Set GPIO40 as output

    EDIS; // Disable write access to protected registers
}

void ADS_begin(int highSpeed, int power)
{
    // Set the speed of ADS
    if(highSpeed)
    {
        digitalWrite(SPEED, 1); // High speed mode
    }
    else
    {
        digitalWrite(SPEED, 0); // Normal speed mode
    }

    // Set the power mode of ADS
    if(power)
    {
        digitalWrite(PDWN, 1); // Power on
    }
    else
    {
        digitalWrite(PDWN, 0); // Power down
    }

    digitalWrite(DOUT, 1); // Initialize DOUT to high
    digitalWrite(SCLK, 0); // Initialize SCLK to low
}

int ADS_read()
{
    int i = 0;
    unsigned long start;
    int startOffset;

    // Determine start offset based on speed
    if(digitalRead(SPEED) == 1)
    {
        startOffset = 19; // Offset for high speed
    }
    else
    {
        startOffset = 150; // Offset for normal speed
    }

    start = millis(); // Get start time
    // Wait for DOUT to go high
    while(digitalRead(DOUT) != 1)
    {
        if(millis() > start + startOffset) // Timeout condition
        {
            return -1;
        }
    }

    start = millis(); // Reset start time
    // Wait for DOUT to go low
    while(digitalRead(DOUT) != 0)
    {
        if(millis() > start + startOffset) // Timeout condition
        {
            return -2;
        }
    }

    // Read 24 bits from ADS
    for(i = 23; i >= 0; i--)
    {
        digitalWrite(SCLK, 1); // Set clock high
        val = (val << 1) + digitalRead(DOUT); // Read bit and shift into val
        digitalWrite(SCLK, 0); // Set clock low
    }

    val = (val << 8) / 256; // Convert to 24-bit value

    digitalWrite(SCLK, 1); // Extra clock pulse
    digitalWrite(SCLK, 0);

    return 0; // Return success
}

int digitalRead(int pin)
{
    // Read the state of a GPIO pin
    if(pin == 0)
    {
        return GpioDataRegs.GPADAT.bit.GPIO0; // Read state of GPIO0
    }
    else if(pin == 1)
    {
        return GpioDataRegs.GPADAT.bit.GPIO1; // Read state of GPIO1
    }
    else if(pin == 3)
    {
        return GpioDataRegs.GPADAT.bit.GPIO3; // Read state of GPIO3
    }
    else if(pin == 40)
    {
        return GpioDataRegs.GPBDAT.bit.GPIO40; // Read state of GPIO40
    }
    return 0; // Default return value
}

void digitalWrite(int pin, int value)
{
    // Write a value to a GPIO pin
    if(pin == 0)
    {
        if(value == 0)
        {
            GpioDataRegs.GPACLEAR.bit.GPIO0 = 1; // Clear GPIO0 output (set low)
        }
        else if(value == 1)
        {
            GpioDataRegs.GPASET.bit.GPIO0 = 1; // Set GPIO0 output (set high)
        }
    }
    else if(pin == 1)
    {
        if(value == 0)
        {
            GpioDataRegs.GPACLEAR.bit.GPIO1 = 1; // Clear GPIO1 output (set low)
        }
        else if(value == 1)
        {
            GpioDataRegs.GPASET.bit.GPIO1 = 1; // Set GPIO1 output (set high)
        }
    }
    else if(pin == 3)
    {
        if(value == 0)
        {
            GpioDataRegs.GPACLEAR.bit.GPIO3 = 1; // Clear GPIO3 output (set low)
        }
        else if(value == 1)
        {
            GpioDataRegs.GPASET.bit.GPIO3 = 1; // Set GPIO3 output (set high)
        }
    }
    else if(pin == 40)
    {
        if(value == 0)
        {
            GpioDataRegs.GPBCLEAR.bit.GPIO40 = 1; // Clear GPIO40 output (set low)
        }
        else if(value == 1)
        {
            GpioDataRegs.GPBSET.bit.GPIO40 = 1; // Set GPIO40 output (set high)
        }
    }
}

__interrupt void cpu_timer0_isr(void)
{
    millisCounter++; // Increment the millisecond counter

    CpuTimer0.InterruptCount++; // Increment interrupt count
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // Acknowledge the PIE group 1 interrupt
}

unsigned long millis(void)
{
    return millisCounter; // Return the number of milliseconds since start
}
