/*******************************************************************************
 * 
 *  FileName:
 *      timing.h
 * 
 * Description:
 *      provides some timing parameters useful to port timing
 * 
 ******************************************************************************/
#pragma once
#include <xc.h>
#include <stdint.h>
// Remove plib warnings
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING
#define _SUPPRESS_PLIB_WARNING
#include <plib.h>
/*------------------------------------------------------------------------------
 * Types
 -----------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
 * Procedures
 -----------------------------------------------------------------------------*/
const int InitTiming(void);     // Setup the system timing buses
void toggleLED();
void delay(uint32_t millis);     // Busy wait for specified number of Milliseconds
inline const uint32_t millis();  // Return the number of milliseconds since start

