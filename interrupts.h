/*******************************************************************************
 *  
 * Filename:
 *      Interrupts.h
 * 
 * Description:
 *      Initializes the vectors and priority for the system ISR's
 *  
 ******************************************************************************/

#pragma once
#include <xc.h>

#define DISABLE_ALL_INTERRUPTS __asm__("di")
#define ENABLE_ALL_INTERRUPTS __asm__("ei")

const int SetupInterrupts(void)
{   
    DISABLE_ALL_INTERRUPTS;
    // Enable the shadow registers.
    PRISS = 0x76543210;   
    // Set up Multi-Vector mode,
    INTCONSET = _INTCON_MVEC_MASK;
    ENABLE_ALL_INTERRUPTS;
}

