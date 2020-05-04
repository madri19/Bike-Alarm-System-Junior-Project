/*******************************************************************************
 *  FileName:
 *      timing.c
 *  Description: 
 *      File defines a simple millisecond timer with interrupt for use with
 *      real time requirements
 ******************************************************************************/
#include "TIMING_MS.h"
#include <sys/attribs.h>
#include <stdint.h>
#include <xc.h>

// Remove plib warnings
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING
#define _SUPPRESS_PLIB_WARNING

#include <plib.h>


#define	GetSystemClock() 			(80000000ul)
#define	GetPeripheralClock()		(GetSystemClock()/(1 << OSCCONbits.PBDIV))

#define TIMER_ON_BIT    0x8000
#define TICK_MILLIS     10000U // 10 Mhz bus -> 10,000 count/millisecond  

volatile static uint32_t milliseconds;

extern volatile int _ALARM_ELAPSED_TIME_;
extern volatile int _Alarm_Going_;

const int InitTiming(void)
{
    //SYSTEMConfigPerformance(80000000);
    
    INTEnableSystemMultiVectoredInt(); 
    INTDisableInterrupts();
    // Zero the counts
    milliseconds = 0;
    SYSKEY = 0; // Ensure lock
    SYSKEY = 0xAA996655; // Write Key 1
    SYSKEY = 0x556699AA; // Write Key 2
    OSCCONbits.COSC = 3; // 10 MHz 
    SYSKEY = 0; // Re-lock      
    TMR2 = 0x0;         //Millisecond timer
    PR2 = TICK_MILLIS;    
//  First, turn off timer and clear its interrupt flag.
    T2CON = 0x0;
    IFS0bits.T2IF = 0;//CLR = _IFS0_T2IF_MASK; // | _IFS1_T9IF_MASK;
//  establish the priority of the service routine .
    IPC2bits.T2IP = 7; //SET  = (2<<_IPC2_T2IP_POSITION); 
//  enable the timer interrupt (set the appropriate IECx flag)
    IEC0bits.T2IE = 1;//SET = _IEC0_T2IE_MASK; // | _IEC1_T9IE_MASK;
//  before starting timer 2 and globally enabling interrupts (ei).
    T2CONbits.ON = 1;//SET = TIMER_ON_BIT;
    
    //LED set up
    TRISCbits.TRISC1 = 0; 
    LATCbits.LATC1 = 1;
    
    INTEnableInterrupts();
}    

/* Millisecond functions */
void delay(uint32_t millis)  // Busy wait for specified number of Milliseconds
{   
    millis += milliseconds;    
    while(millis > milliseconds);   // Busy wait
}

inline const uint32_t millis()  // Return the number of milliseconds since start
{
    return milliseconds;
}

void toggleLED(){
    if(LATCbits.LATC1 == 1){
        LATCbits.LATC1 = 0;
    }
    else{
        LATCbits.LATC1 = 1;
    }
}

void __ISR(_TIMER_2_VECTOR, IPL7SOFT) Timer2Handler (void){    
    ++milliseconds;
    if (milliseconds % 1000 == 0){
            toggleLED();
            if(_Alarm_Going_)
            {
                _ALARM_ELAPSED_TIME_++;
            }
    }           
    mT2ClearIntFlag();
}