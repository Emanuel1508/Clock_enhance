
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef TIMERS_H
#define	TIMERS_H


#define TIMER1_FREQUENCY_HZ 1000
#define TIMER5_FREQUENCY_HZ 1000

//Timer 1 used as timebase
void ConfigureTimer1(unsigned short Frequency);
//Timer3 used for sound generation
void ConfigureTimer3(unsigned short Frequency);
//Reconfigure: Just set Frequency and start
void ReConfigureTimer3(unsigned short Frequency);
//Timer5 for the Button State Machines
void ConfigureTimer5(unsigned short Frequency);


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

