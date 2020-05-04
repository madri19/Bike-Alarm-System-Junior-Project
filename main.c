// PIC32MX795F512L Configuration Bit Settings
// 'C' source line config statements

// DEVCFG3
#pragma config USERID = 0xFFFF          // Enter Hexadecimal value (Enter Hexadecimal value)
#pragma config FSRSSEL = PRIORITY_7     // SRS Select (SRS Priority 7)
#pragma config FMIIEN = ON              // Ethernet RMII/MII Enable (MII Enabled)
#pragma config FETHIO = ON              // Ethernet I/O Pin Select (Default Ethernet I/O)
#pragma config FCANIO = ON              // CAN I/O Pin Select (Default CAN I/O)
#pragma config FUSBIDIO = ON            // USB USID Selection (Controlled by the USB Module)
#pragma config FVBUSONIO = ON           // USB VBUS ON Selection (Controlled by USB Module)

// DEVCFG2
#pragma config FPLLIDIV = DIV_2        // PLL Input Divider (12x Divider)
#pragma config FPLLMUL = MUL_20         // PLL Multiplier (24x Multiplier)
#pragma config UPLLIDIV = DIV_12        // USB PLL Input Divider (12x Divider)
#pragma config UPLLEN = OFF             // USB PLL Enable (Disabled and Bypassed)
#pragma config FPLLODIV = DIV_1       // System PLL Output Clock Divider (PLL Divide by 256)

// DEVCFG1
#pragma config FNOSC = 1           // Oscillator Selection Bits (Fast RC Osc w/Div-by-N (FRCDIV))
#pragma config FSOSCEN = ON             // Secondary Oscillator Enable (Enabled)
#pragma config IESO = ON                // Internal/External Switch Over (Enabled)
#pragma config POSCMOD = OFF            // Primary Oscillator Configuration (Primary osc disabled)
#pragma config OSCIOFNC = OFF           // CLKO Output Signal Active on the OSCO Pin (Disabled)


#pragma config FPBDIV = DIV_8           // Peripheral Clock Divisor (Pb_Clk is Sys_Clk/8)
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor Selection (Clock Switch Disable, FSCM Disabled)
#pragma config WDTPS = PS1048576        // Watchdog Timer Postscaler (1:1048576)
#pragma config FWDTEN = ON              // Watchdog Timer Enable (WDT Enabled)

// DEVCFG0
#pragma config DEBUG = OFF              // Background Debugger Enable (Debugger is disabled)
#pragma config ICESEL = ICS_PGx2        // ICE/ICD Comm Channel Select (ICE EMUC2/EMUD2 pins shared with PGC2/PGD2)
#pragma config PWP = OFF                // Program Flash Write Protect (Disable)
#pragma config BWP = OFF                // Boot Flash Write Protect bit (Protection Disabled)
#pragma config CP = OFF                 // Code Protect (Protection Disabled)

// Remove plib warnings
#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING

volatile int front;

volatile int end;

volatile char newBuffer[64];

#include <xc.h>
#include <sys/attribs.h>
#include <plib.h>
#include "TIMING_MS.h"
#include "UART_LIB.h"
#include "GSM_MQTT.h"

volatile int _ALARM_ELAPSED_TIME_;
volatile int _ALARM_FINAL_TIME_;//seconds
volatile int _PANIC_SIGNAL_;// 0 = off, 1 = on
volatile int _ARM_SIGNAL_;
volatile int _Alarm_Going_;

void resetGSM()
{
   LATDbits.LATD4 = 0;
   delay(1500);
   LATDbits.LATD4 = 1;
}

void enableSensors()
{
    enableI2C();
    setupAccerlerometerI2C();
    PROXINIT();
    //set up external interrupt 1(pin 2 on the board)  (priority 7, interrupt on falling edge, external interrupt enable)
    ConfigINT1(EXT_INT_PRI_7 | FALLING_EDGE_INT | EXT_INT_ENABLE);
}

void disableSensors()
{
    //disable proximity sensor
    disableProxSensor();
    ConfigINT1(EXT_INT_PRI_7 | FALLING_EDGE_INT | EXT_INT_DISABLE);
}

void firstSetup()
{
    // pin 8 digital output for ARM_SIGNAL
    TRISDbits.TRISD3 = 0;
    LATDbits.LATD3 = 0;
    
    // Pin 8 output digital
    TRISDbits.TRISD12 = 0;
    LATDbits.LATD12 = 0;

    //initialize global variables
    _ALARM_ELAPSED_TIME_ = 0;
    _PANIC_SIGNAL_ = 0;
    _Alarm_Going_ = 0;
    
    //pin 10 digital output for resetting gsm module(PWX pin)
    // Set up and make it high
    TRISDbits.TRISD4 = 0;
    LATDbits.LATD4 = 1;
}

int main(){
    
    firstSetup();
    UARTinit();
    InitTiming();
    
    resetGSM();
    delay(1000);
    resetGSM();
    
    begin();
    
    while(1){
        
//        if(!_ARM_SIGNAL_)
//        {
//            delay(3000);
//            //arm signal led
//            LATDbits.LATD3 = 1;
//            _ARM_SIGNAL_ = 1;
//        }
        
        if(_ARM_SIGNAL_)
        {
            enableSensors();
            clearAccelerometerInterruptFlag();
            while(_ARM_SIGNAL_)
            {
                if (_PANIC_SIGNAL_)
                {
                    // Raise the LED Alarm
                    _PANIC_SIGNAL_ = 0;
                    _Alarm_Going_ = 1;
                    LATDbits.LATD12 = 1;
                    //publish(0, 0, 0, _generateMessageID(), "GiselleLFreude/feeds/panic-signal-watch", "PANIC");
                    MakePhoneCall(); //call user
                    
                }
                // If Elapsed time is true then turn off alarm sound
                if(_ALARM_ELAPSED_TIME_ >= _ALARM_FINAL_TIME_){
                    //turn siren off
                    _Alarm_Going_ = 0;
                    LATDbits.LATD12 = 0;
                    _ALARM_ELAPSED_TIME_ = 0;
                    _ARM_SIGNAL_ = 0;
                    //arm signal led
                    LATDbits.LATD3 = 0;
                    clearAccelerometerInterruptFlag();
                    disableSensors();
                }
                processing();
            }
        }
        //clearAccelerometerInterruptFlag();
        
        processing();
        
    }
    
    return 0;
}

void __ISR(_EXTERNAL_1_VECTOR, IPL7SOFT) MyINT1Handler (void)
{
    // If Alarm is Off then turn it on
    if(_PANIC_SIGNAL_ == 0 && _ARM_SIGNAL_ == 1){
        _PANIC_SIGNAL_ = 1;
    }
    
    // Clear INT3 external interrupt flag
    mINT1ClearIntFlag();
}
