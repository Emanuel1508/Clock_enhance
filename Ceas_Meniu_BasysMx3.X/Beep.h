
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef BEEP_H
#define	BEEP_H

#include <xc.h>


void AUDIO_ConfigurePins();

void Beep(uint16_t Frequency_Hz, uint16_t Length_ms, uint16_t PauseTime_ms, uint8_t RepeatTimes);

void BeepFSM();

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

