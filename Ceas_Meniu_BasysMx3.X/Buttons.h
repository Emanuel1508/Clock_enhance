
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef BUTTONS_H
#define	BUTTONS_H


#include <xc.h>
#include "btn.h"
#include "config.h"

//Incrementarea BtnTimeCount: Realizam in Timer0
//Acesta va fi setat la perioada de 1ms
//3ms pentru debounce: STABLE_TIME_TICKS
#define STABLE_TIME_TICKS 3 
//1500ms pentru LONGPRESS
#define LONGPRESS_TICKS 1500
//200ms pentru DBLCLICK_TIMEOUT
#define DBLCLICK_TIMEOUT_TICKS 50




enum BtnFsmStates{stBtnIdle, stBtnPressed, stBtnClick,
                  stBtnDblClick, stBtnLongPress};
                  
//EP -> adding the right button as well
enum Buttons {BtnU=0, BtnL, BtnC, BtnD, BtnR, NumButtons};

enum BtnValues {BtnReleased=0, BtnPressed};

enum BtnFlagTypes {BtnIdle = 0, Click, DblClick, LPress, LpRelease};



void BtnFSM(enum Buttons Btn);
void Init_Buttons();





#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

