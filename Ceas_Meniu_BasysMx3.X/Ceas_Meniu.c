/* 
 * File:   Ceas_Meniu.c
 * Author: Albertf
 *
 * Created on March 24, 2019, 10:05 AM
 */

#include <stdio.h>
#include <stdlib.h>

#include "config_bits.h"
#include <xc.h>
#include "config.h"
#include "led.h"
#include "swt.h"
#include "btn.h"
#include "lcd.h"
#include "utils.h"
#include <sys/attribs.h>
#include <string.h>

#include "Buttons.h"
#include "time_utils.h"
#include "Timers.h"
#include "Beep.h"


#define TIMEBASE_TICKS 1000
#define BLINK_PERIOD   500 // should be less than TIMEBASE_TICKS!
#define FAST_BLINK_PERIOD   250 // should be less than TIMEBASE_TICKS!
#define FAST_ADJ_PERIOD 125 // for FAST Adjust

volatile unsigned short TimebaseCount=0;

//Flag for TimerBase, meaning that 1 Second passed
volatile uint8_t Timebase_Tick = 0;
//Flags for Blinking
volatile uint8_t Blink_ON = 0;
volatile uint8_t Blink_Tick = 0;

volatile uint8_t FastBlink_ON = 0;
volatile uint8_t FastBlink_Tick = 0;

//flag for Fast Adjust
volatile uint8_t FastAdj_Tick = 0;

//Flag for Beep Timebase
volatile uint8_t Beep_Timebase_Tick = 0;

extern const uint8_t monthDays[];
extern const char WeekdayNames[];
extern const uint8_t WeekdayNameLength;

tmelements_t TimeDate; 
tmelements_t * pTimeDate = &TimeDate;

extern enum BtnFsmStates BtnFSMState[NumButtons];

extern enum BtnValues BtnVal[NumButtons];

extern enum BtnFlagTypes BtnFlag[NumButtons];

extern unsigned short BtnTimeCount[NumButtons];


volatile enum WatchStates {stWatch, stAdjSec, stAdjMin, stAdjHr, stAdjYear, stAdjMonth, stAdjDay} WatchState;


//Automatul secvential de reglaj
void WatchStateMachine();

//ajustarea elementelor in meniul de reglaj
uint8_t IncItem(uint8_t Item, uint8_t Lower, uint8_t Upper);
uint8_t DecItem(uint8_t Item, uint8_t Lower, uint8_t Upper);

//afisarea ceasului/calendarului
void FormatAndDisplayTime(tmelements_t * TmDt);

int main(int argc, char** argv) {
     uint32_t CurrentTimeSecs;
    char Line0[17], Line1[17];
    
    LED_Init(); SWT_Init(); BTN_Init(); LCD_Init();
    AUDIO_ConfigurePins();

    //Prepare Btn Change Notice Interrupts
    Init_Buttons();
    
    DelayAprox10Us(100);

    
    ConfigureTimer1(TIMER1_FREQUENCY_HZ);
    
    //Just a Timer3 initialization. 
    //When a Beep sound is needed, Timer3 will be reconfigured
    ConfigureTimer3(1000);
    
    ConfigureTimer5(TIMER5_FREQUENCY_HZ);
    
   
    macro_enable_interrupts();
    
     
    //Setare Ceas/Calendar la o valoare data
    /*
    TimeDate.Second = 50; 
    TimeDate.Minute = 59; 
    TimeDate.Hour = 23; 
    TimeDate.Day  = 28;
    TimeDate.Month = 2; 
    TimeDate.Year = 19;   // offset from 2000; 
   */
    
    //Setare Ceas/Calendar la Data/Ora compilarii
    RtcDateTime(pTimeDate, __DATE__, __TIME__);
    
    
    //Usage: Beep(uint16_t Frequency_Hz, uint16_t Length_ms, uint16_t PauseTime_ms, uint8_t RepeatTimes)
    Beep(2500, 100, 100, 2);
    
      
    
    while (1){
        
       if (Timebase_Tick){
            breakTime(makeTime(&TimeDate) +1, &TimeDate);//incrementam secunda
            FormatAndDisplayTime(&TimeDate);
            Timebase_Tick = 0;
       }
       if (Blink_Tick || FastBlink_Tick){
           FormatAndDisplayTime(&TimeDate);
           if (Blink_Tick) Blink_Tick = 0;
           if (FastBlink_Tick) FastBlink_Tick = 0;
        }
        
    }
    
    return (EXIT_SUCCESS);
}


void FormatAndDisplayTime(tmelements_t * TmDt){
    
    uint8_t i;
    uint8_t index;
    char Line0[17], Line1[17];
    //string separat pentru ziua din saptamana
    //Se copiaza din time_utils.c/WeekdayNames
    char wkDayStr[4];
    //Clean the strings first
    for (i = 0; i<17; i++) {Line0[i] = 0; Line1[i] = 0;}
    
    if (Timebase_Tick){
        sprintf(Line0, "% 2d:%02d:%02d  ", TmDt->Hour, TmDt->Minute, TmDt->Second);
        //cautam ziua din saptamana si il copiem
        index = TmDt->Wday * WeekdayNameLength; 
         memcpy(wkDayStr,  WeekdayNames+index, WeekdayNameLength);
        wkDayStr[3] = '\0';
        
        if (TmDt->Day >=10) sprintf(Line1, "%s, %2d/%2d/%4d ", wkDayStr, TmDt->Day, TmDt->Month, TmDt->Year + 2000);
        else sprintf(Line1, "%s, % d/%2d/%4d ", wkDayStr, TmDt->Day, TmDt->Month, TmDt->Year + 2000);
        
        if (TmDt->Hour >=10) LCD_WriteStringAtPos(Line0, 0, 3);
        else LCD_WriteStringAtPos(Line0, 0, 4);
        LCD_WriteStringAtPos(Line1, 1, 0);   
    }
    else if ((Blink_Tick) && (WatchState == stWatch)){
        if (Blink_ON)//Blink pe ":" intre HH si SS
                LCD_WriteStringAtPos(" ", 0, 6);
        else LCD_WriteStringAtPos(":", 0, 6);
    }
    //La FastAdj tiparim separat ca sa se vada modificarea rapida
    else if ((BtnFlag[BtnU]==LPress)&&(FastAdj_Tick)){
        switch(WatchState){
            case stAdjSec   : sprintf(Line0, "%02d", TmDt->Second);
                               LCD_WriteStringAtPos(Line0, 0, 10);
                            break;
            case stAdjMin   :   sprintf(Line0, "%02d", TmDt->Minute);
                               LCD_WriteStringAtPos(Line0, 0, 7);
                            break;
            case stAdjHr    :   sprintf(Line0, "% 2d", TmDt->Hour);
                                if (TmDt->Hour >=10) LCD_WriteStringAtPos(Line0, 0, 3);
                                else LCD_WriteStringAtPos(Line0, 0, 4);
                            break;
            case stAdjDay   :
                               if (TmDt->Day>=10) sprintf(Line0, "%2d", TmDt->Day);
                               else sprintf(Line0, "% d", TmDt->Day);
                               LCD_WriteStringAtPos(Line0, 1, 5);
                            break; 
            case stAdjMonth :   sprintf(Line0, "%2d", TmDt->Month);
                               LCD_WriteStringAtPos(Line0, 1, 8);
                            break;
            case stAdjYear  :  sprintf(Line0, "%4d", TmDt->Year + 2000);
                               LCD_WriteStringAtPos(Line0, 1, 11);
                            break;
            default: break;
        } 
    }
    else if ((FastBlink_Tick)&& (BtnFlag[BtnU]!=LPress)){//Fast blink la ajustare lenta
        switch(WatchState){
            case stAdjSec   : if (!FastBlink_ON){
                               sprintf(Line0, "%02d", TmDt->Second);
                               LCD_WriteStringAtPos(Line0, 0, 10);
                              }
                              else LCD_WriteStringAtPos("  ", 0, 10);
                            break;
            case stAdjMin   : if (!FastBlink_ON){
                               sprintf(Line0, "%02d", TmDt->Minute);
                               LCD_WriteStringAtPos(Line0, 0, 7);
                              }
                              else LCD_WriteStringAtPos("  ", 0, 7);
                            break;
            case stAdjHr    : if (!FastBlink_ON){  
                                sprintf(Line0, "% 2d", TmDt->Hour);
                                if (TmDt->Hour >=10) LCD_WriteStringAtPos(Line0, 0, 3);
                                else LCD_WriteStringAtPos(Line0, 0, 4);
                              }
                              else LCD_WriteStringAtPos("  ", 0, 4);
                            break;
            case stAdjDay   : if (!FastBlink_ON){  
                               if (TmDt->Day>=10) sprintf(Line0, "%2d", TmDt->Day);
                               else sprintf(Line0, "% d", TmDt->Day);
                               LCD_WriteStringAtPos(Line0, 1, 5);
                              }
                              else LCD_WriteStringAtPos("  ", 1, 5);
                            break; 
            case stAdjMonth : if (!FastBlink_ON){  
                               sprintf(Line0, "%2d", TmDt->Month);
                               LCD_WriteStringAtPos(Line0, 1, 8);
                              }
                              else LCD_WriteStringAtPos("  ", 1, 8);
                            break;
            case stAdjYear  : if (!FastBlink_ON){ 
                               sprintf(Line0, "%4d", TmDt->Year + 2000);
                               LCD_WriteStringAtPos(Line0, 1, 11);
                              }
                              else LCD_WriteStringAtPos("    ", 1, 11);
                            break;
            default: break;
        }//la switch (WatchState)
    }//la else if ((FastBlink_Tick)&& (BtnFlag[BtnU]!=LPress))

 }


//Corpul ISR Timer1 ar fi trebuit sa fie in Timers.c
//Insa e aratat aici, pentru a vedea codul bazelor de timp ale ceasului
void __ISR(_TIMER_1_VECTOR, IPL1AUTO) Timer1ISR(void) 
{  
    //Numarare pentru baza de timp de 1 S
    TimebaseCount++;
    //Setam Flag-ul Blink
    if ((TimebaseCount % BLINK_PERIOD) == 0){
        Blink_ON = !Blink_ON;
        Blink_Tick = 1;
    }
    if ((TimebaseCount % FAST_BLINK_PERIOD) == 0){
        FastBlink_ON = !FastBlink_ON;
        FastBlink_Tick = 1;
    }
    
    //Setam flag-ul FastAdj_Tick
    if ((TimebaseCount % FAST_ADJ_PERIOD) == 0){
        FastAdj_Tick = 1;
    }
    
    //Baza de timp
    if (TimebaseCount >= TIMEBASE_TICKS){
        TimebaseCount = 0;
        Timebase_Tick = 1;
    }

    //A time tick for Beep at each milisecond
    Beep_Timebase_Tick = 1;
    
    WatchStateMachine();
    
    
    IFS0bits.T1IF = 0;      // clear Timer1 interrupt flag
}



void WatchStateMachine(){

    switch (WatchState){
        case stWatch    : if (BtnFlag[BtnC] == LPress){
                            WatchState = stAdjSec; 
                            //fortam ca afisarea sa inceapa cu blink pe elementul ajustat
                            FastBlink_ON = 1;
                            //Clear Flag
                            BtnFlag[BtnC] = BtnIdle;
                        }
        break;
        case stAdjSec   : 
                        //Iesire din meniu
                        if (BtnFlag[BtnC] == LPress){//Testam LPress BtnC = iesire din meniu
                            WatchState = stWatch;
                            //Clear Flag
                            BtnFlag[BtnC] = BtnIdle;
                        }
                      
                        //Schimbarea elementului, care se ajusteaza
                        else if ( (BtnFlag[BtnL]==Click)|| (BtnFlag[BtnL]==DblClick) || ((BtnFlag[BtnL]==LPress)&&(FastBlink_Tick)) ){
                            WatchState = stAdjMin;
                            FastBlink_ON = 1; //pentru ca urmatorul element de ajustat sa apara prima data "stins" 
                            FormatAndDisplayTime(&TimeDate);
                            //Clear Flags
                            if ((BtnFlag[BtnL]==Click)|| (BtnFlag[BtnL]==DblClick)) BtnFlag[BtnL] = BtnIdle;                            
                            if ((BtnFlag[BtnL]==LPress)&&(FastBlink_Tick)) FastBlink_Tick = 0;
                        }
                          //EP -> Schimbare elementului care se ajusteaza, spre dreapta (zile)
                        else if ( (BtnFlag[BtnR] == Click) || (BtnFlag[BtnR] == DblClick)
                                ||((BtnFlag[BtnR] == LPress) && (FastBlink_Tick)) ) {
                            WatchState = stAdjDay;
                            FastBlink_ON = 1;
                            FormatAndDisplayTime(&TimeDate);
                            
                            if ((BtnFlag[BtnR]==Click)|| (BtnFlag[BtnR]==DblClick)) BtnFlag[BtnR] = BtnIdle;                            
                            if ((BtnFlag[BtnR]==LPress)&&(FastBlink_Tick)) FastBlink_Tick = 0;
                        }
                // final implementare
                        
                        //Modificare element ajustat
                        else if ( (BtnFlag[BtnU]==Click) || (BtnFlag[BtnU]==DblClick) || ((BtnFlag[BtnU]==LPress)&&(FastAdj_Tick))  ){
                            if (TimeDate.Second >= 30){
                                    TimeDate.Second = 0; 
                                    breakTime(makeTime(&TimeDate) +60, &TimeDate);//Reactualizam ceasul
                                   }
                            else TimeDate.Second = 0;
                            breakTime(makeTime(&TimeDate), &TimeDate);//Reactualizam ceasul
                            FormatAndDisplayTime(&TimeDate);
                            //Clear Flags
                            if ((BtnFlag[BtnU]==Click) || (BtnFlag[BtnU]==DblClick)) BtnFlag[BtnU] = BtnIdle;                              
                            if ((BtnFlag[BtnU]==LPress)&&(FastAdj_Tick)) FastAdj_Tick = 0;
                        }
                        
                        //EP -> Decrementare element ajustat
                        else if ( (BtnFlag[BtnD]==Click) || (BtnFlag[BtnD]==DblClick) || ((BtnFlag[BtnD]==LPress)&&(FastAdj_Tick))  ){
                            
                            //EP -> Generare sunet de frecventa mai joasa (frecv, length, pauza, repeatTimes)
                            Beep(1000, 100, 100, 1);
                            
                            if (TimeDate.Second >= 30){
                                    TimeDate.Second = 0; 
                                   }
                            else {
                                TimeDate.Second = 0;
                                breakTime(makeTime(&TimeDate) -60, &TimeDate);
                            }
                            breakTime(makeTime(&TimeDate), &TimeDate);//Reactualizam ceasul
                            FormatAndDisplayTime(&TimeDate);
                            //Clear Flags
                            if ((BtnFlag[BtnD]==Click) || (BtnFlag[BtnD]==DblClick)) BtnFlag[BtnD] = BtnIdle;                              
                            if ((BtnFlag[BtnD]==LPress)&&(FastAdj_Tick)) FastAdj_Tick = 0;
                        }
                        //final decrementare
                        
        break;
        case stAdjMin   :
                        //Iesire din meniu
                        if (BtnFlag[BtnC] == LPress){//Testam LPress BtnC = iesire din meniu
                            WatchState = stWatch;
                            //Clear Flag
                            BtnFlag[BtnC] = BtnIdle;
                        }
                        //Schimbarea elementului, care se ajusteaza
                        else if ( (BtnFlag[BtnL]==Click)|| (BtnFlag[BtnL]==DblClick)|| ((BtnFlag[BtnL]==LPress)&&(FastBlink_Tick)) ){
                            WatchState = stAdjHr;
                            FastBlink_ON = 1; //pentru ca urmatorul element de ajustat sa apara prima data "stins" 
                            FormatAndDisplayTime(&TimeDate);
                            //Clear Flags
                            if ((BtnFlag[BtnL]==Click)|| (BtnFlag[BtnL]==DblClick)) BtnFlag[BtnL] = BtnIdle;                            
                            if ((BtnFlag[BtnL]==LPress)&&(FastBlink_Tick)) FastBlink_Tick = 0;
                        }
                        
                          //EP -> Schimbare elementului care se ajusteaza, spre dreapta (secunde)
                        else if ( (BtnFlag[BtnR] == Click) || (BtnFlag[BtnR] == DblClick)
                                ||((BtnFlag[BtnR] == LPress) && (FastBlink_Tick)) ) {
                            WatchState = stAdjSec;
                            FastBlink_ON = 1;
                            FormatAndDisplayTime(&TimeDate);
                            
                            if ((BtnFlag[BtnR]==Click)|| (BtnFlag[BtnR]==DblClick)) BtnFlag[BtnR] = BtnIdle;                            
                            if ((BtnFlag[BtnR]==LPress)&&(FastBlink_Tick)) FastBlink_Tick = 0;
                        }
                // final implementare

                        //Incrementare element ajustat
                        else if ( (BtnFlag[BtnU]==Click) || (BtnFlag[BtnU]==DblClick) || ((BtnFlag[BtnU]==LPress)&&(FastAdj_Tick))  ){
                            TimeDate.Minute = IncItem(TimeDate.Minute, 0, 59);
                            //Reactualizam ceasul, daca s-a incrementat cu Click
                            //sau la iesirea din LPress
                            if ((BtnFlag[BtnU]==Click)|| (BtnFlag[BtnU]==LpRelease))
                                breakTime(makeTime(&TimeDate), &TimeDate);
                            FormatAndDisplayTime(&TimeDate);
                            //Clear Flags
                            if ((BtnFlag[BtnU]==Click) || (BtnFlag[BtnU]==DblClick)) BtnFlag[BtnU] = BtnIdle;                              
                            if ((BtnFlag[BtnU]==LPress)&&(FastAdj_Tick)) FastAdj_Tick = 0;
                        }
                        
                        //EP -> Decrementare element ajustat
                            else if ( (BtnFlag[BtnD]==Click) || (BtnFlag[BtnD]==DblClick) || ((BtnFlag[BtnD]==LPress)&&(FastAdj_Tick))  ){
                            
                            //EP -> Generare sunet de frecventa mai joasa (frecv, length, pauza, repeatTimes)
                            Beep(1000, 100, 100, 1);
                                
                            TimeDate.Minute = DecItem(TimeDate.Minute, 0, 59);
                            //Reactualizam ceasul, daca s-a decrementat cu Click
                            //sau la iesirea din LPress
                            if ((BtnFlag[BtnD]==Click)|| (BtnFlag[BtnD]==LpRelease))
                                breakTime(makeTime(&TimeDate), &TimeDate);
                            FormatAndDisplayTime(&TimeDate);
                            //Clear Flags
                            if ((BtnFlag[BtnD]==Click) || (BtnFlag[BtnD]==DblClick)) BtnFlag[BtnD] = BtnIdle;                              
                            if ((BtnFlag[BtnD]==LPress)&&(FastAdj_Tick)) FastAdj_Tick = 0;
                        }
                      //final decrementare
                        
                        
                        
        break;
        case stAdjHr   :
                        //Iesire din meniu
                        if (BtnFlag[BtnC] == LPress){//Testam LPress BtnC = iesire din meniu
                            WatchState = stWatch;
                            //Clear Flag
                            BtnFlag[BtnC] = BtnIdle;
                        }
                        //Schimbarea elementului, care se ajusteaza
                        else if ( (BtnFlag[BtnL]==Click)|| (BtnFlag[BtnL]==DblClick) || ((BtnFlag[BtnL]==LPress)&&(FastBlink_Tick)) ){
                            WatchState = stAdjYear;
                            FastBlink_ON = 1;//pentru ca urmatorul element de ajustat sa apara prima data "stins" 
                            FormatAndDisplayTime(&TimeDate);
                            //Clear Flags
                            if ((BtnFlag[BtnL]==Click)|| (BtnFlag[BtnL]==DblClick)) BtnFlag[BtnL] = BtnIdle;                            
                            if ((BtnFlag[BtnL]==LPress)&&(FastBlink_Tick)) FastBlink_Tick = 0;
                        }
                        
                        
                          //EP -> Schimbare elementului care se ajusteaza, spre dreapta (minute)
                        else if ( (BtnFlag[BtnR] == Click) || (BtnFlag[BtnR] == DblClick)
                                ||((BtnFlag[BtnR] == LPress) && (FastBlink_Tick)) ) {
                            WatchState = stAdjMin;
                            FastBlink_ON = 1;
                            FormatAndDisplayTime(&TimeDate);
                            
                            if ((BtnFlag[BtnR]==Click)|| (BtnFlag[BtnR]==DblClick)) BtnFlag[BtnR] = BtnIdle;                            
                            if ((BtnFlag[BtnR]==LPress)&&(FastBlink_Tick)) FastBlink_Tick = 0;
                        }
                // final implementare
                        
                        //Incrementare element ajustat
                        else if ( (BtnFlag[BtnU]==Click) || (BtnFlag[BtnU]==DblClick) || ((BtnFlag[BtnU]==LPress)&&(FastAdj_Tick))  ){
                            TimeDate.Hour = IncItem(TimeDate.Hour, 0, 23);
                            //Reactualizam ceasul, daca s-a incrementat cu Click
                            //sau la iesirea din LPress
                            if ((BtnFlag[BtnU]==Click)|| (BtnFlag[BtnU]==LpRelease))
                                breakTime(makeTime(&TimeDate), &TimeDate);
                            FormatAndDisplayTime(&TimeDate);
                            //Clear Flags
                            if ((BtnFlag[BtnU]==Click) || (BtnFlag[BtnU]==DblClick)) BtnFlag[BtnU] = BtnIdle;                              
                            if ((BtnFlag[BtnU]==LPress)&&(FastAdj_Tick)) FastAdj_Tick = 0;
                        }
                        
                        //EP -> Decrementare element ajustat
                        else if ( (BtnFlag[BtnD]==Click) || (BtnFlag[BtnD]==DblClick) || ((BtnFlag[BtnD]==LPress)&&(FastAdj_Tick))  ){
                           
                            //EP -> Generare sunet de frecventa mai joasa (frecv, length, pauza, repeatTimes)
                            Beep(1000, 100, 100, 1);
                            
                            TimeDate.Hour = DecItem(TimeDate.Hour, 0, 23);
                            //Reactualizam ceasul, daca s-a decrementat cu Click
                            //sau la iesirea din LPress
                            if ((BtnFlag[BtnD]==Click)|| (BtnFlag[BtnD]==LpRelease))
                                breakTime(makeTime(&TimeDate), &TimeDate);
                            FormatAndDisplayTime(&TimeDate);
                            //Clear Flags
                            if ((BtnFlag[BtnD]==Click) || (BtnFlag[BtnD]==DblClick)) BtnFlag[BtnD] = BtnIdle;                              
                            if ((BtnFlag[BtnD]==LPress)&&(FastAdj_Tick)) FastAdj_Tick = 0;
                        }
                        //final decrementare
            
        break;
        case stAdjYear :
                        //Iesire din meniu
                        if (BtnFlag[BtnC] == LPress){//Testam LPress BtnC = iesire din meniu
                            WatchState = stWatch;
                            //Clear Flag
                            BtnFlag[BtnC] = BtnIdle;
                        }
                        //Schimbarea elementului, care se ajusteaza
                        else if ( (BtnFlag[BtnL]==Click)|| (BtnFlag[BtnL]==DblClick) || ((BtnFlag[BtnL]==LPress)&&(FastBlink_Tick)) ){
                            WatchState = stAdjMonth;
                            FastBlink_ON = 1;//pentru ca urmatorul element de ajustat sa apara prima data "stins" 
                            FormatAndDisplayTime(&TimeDate);
                            //Clear Flags
                            if ((BtnFlag[BtnL]==Click)|| (BtnFlag[BtnL]==DblClick)) BtnFlag[BtnL] = BtnIdle;                            
                            if ((BtnFlag[BtnL]==LPress)&&(FastBlink_Tick)) FastBlink_Tick = 0;
                        }
                        
                         //EP -> Schimbare elementului care se ajusteaza, spre dreapta (ore)
                        else if ( (BtnFlag[BtnR] == Click) || (BtnFlag[BtnR] == DblClick)
                                ||((BtnFlag[BtnR] == LPress) && (FastBlink_Tick)) ) {
                            WatchState = stAdjHr;
                            FastBlink_ON = 1;
                            FormatAndDisplayTime(&TimeDate);
                            
                            if ((BtnFlag[BtnR]==Click)|| (BtnFlag[BtnR]==DblClick)) BtnFlag[BtnR] = BtnIdle;                            
                            if ((BtnFlag[BtnR]==LPress)&&(FastBlink_Tick)) FastBlink_Tick = 0;
                        }
                // final implementare
                        
                        //Incrementare element ajustat
                        else if ( (BtnFlag[BtnU]==Click) || (BtnFlag[BtnU]==DblClick) || ((BtnFlag[BtnU]==LPress)&&(FastAdj_Tick))  ){
                            TimeDate.Year = IncItem(TimeDate.Year, 0, 99);
                            //Reactualizam ceasul, daca s-a incrementat cu Click
                            //sau la iesirea din LPress
                            if ((BtnFlag[BtnU]==Click)|| (BtnFlag[BtnU]==LpRelease))
                                breakTime(makeTime(&TimeDate), &TimeDate);
                            FormatAndDisplayTime(&TimeDate);
                            //Clear Flags
                            if ((BtnFlag[BtnU]==Click) || (BtnFlag[BtnU]==DblClick)) BtnFlag[BtnU] = BtnIdle;                              
                            if ((BtnFlag[BtnU]==LPress)&&(FastAdj_Tick)) FastAdj_Tick = 0;
                        }
                        
                        //EP -> Decrementare element ajustat
                        else if ( (BtnFlag[BtnD]==Click) || (BtnFlag[BtnD]==DblClick) || ((BtnFlag[BtnD]==LPress)&&(FastAdj_Tick))  ){
                            
                            //EP -> Generare sunet de frecventa mai joasa (frecv, length, pauza, repeatTimes)
                            Beep(1000, 100, 100, 1);
                            
                            TimeDate.Year = DecItem(TimeDate.Year, 0, 99);
                            //Reactualizam ceasul, daca s-a decrementat cu Click
                            //sau la iesirea din LPress
                            if ((BtnFlag[BtnD]==Click)|| (BtnFlag[BtnD]==LpRelease))
                                breakTime(makeTime(&TimeDate), &TimeDate);
                            FormatAndDisplayTime(&TimeDate);
                            //Clear Flags
                            if ((BtnFlag[BtnD]==Click) || (BtnFlag[BtnD]==DblClick)) BtnFlag[BtnD] = BtnIdle;                              
                            if ((BtnFlag[BtnD]==LPress)&&(FastAdj_Tick)) FastAdj_Tick = 0;
                        }
                        //final decrementare
        break;
        case stAdjMonth :
                        //Iesire din meniu
                        if (BtnFlag[BtnC] == LPress){//Testam LPress BtnC = iesire din meniu
                            WatchState = stWatch;
                            //Clear Flag
                            BtnFlag[BtnC] = BtnIdle;
                        }
                        //Schimbarea elementului, care se ajusteaza
                        else if ( (BtnFlag[BtnL]==Click)|| (BtnFlag[BtnL]==DblClick) || ((BtnFlag[BtnL]==LPress)&&(FastBlink_Tick)) ){
                            WatchState = stAdjDay;
                            FastBlink_ON = 1;//pentru ca urmatorul element de ajustat sa apara prima data "stins" 
                            FormatAndDisplayTime(&TimeDate);
                            //Clear Flags
                            if ((BtnFlag[BtnL]==Click)|| (BtnFlag[BtnL]==DblClick)) BtnFlag[BtnL] = BtnIdle;                            
                            if ((BtnFlag[BtnL]==LPress)&&(FastBlink_Tick)) FastBlink_Tick = 0;
                        }
                        
                        //EP -> Schimbare elementului care se ajusteaza, spre dreapta (ani)
                        else if ( (BtnFlag[BtnR] == Click) || (BtnFlag[BtnR] == DblClick)
                                ||((BtnFlag[BtnR] == LPress) && (FastBlink_Tick)) ) {
                            WatchState = stAdjYear;
                            FastBlink_ON = 1;
                            FormatAndDisplayTime(&TimeDate);
                            
                            if ((BtnFlag[BtnR]==Click)|| (BtnFlag[BtnR]==DblClick)) BtnFlag[BtnR] = BtnIdle;                            
                            if ((BtnFlag[BtnR]==LPress)&&(FastBlink_Tick)) FastBlink_Tick = 0;
                        }
                // final implementare
                        
                        //Incrementare element ajustat
                        else if ( (BtnFlag[BtnU]==Click) || (BtnFlag[BtnU]==DblClick) || ((BtnFlag[BtnU]==LPress)&&(FastAdj_Tick))  ){
                            TimeDate.Month = IncItem(TimeDate.Month, 1, 12);
                            //Reactualizam ceasul, daca s-a incrementat cu Click
                            //sau la iesirea din LPress
                            if ((BtnFlag[BtnU]==Click)|| (BtnFlag[BtnU]==LpRelease))
                                breakTime(makeTime(&TimeDate), &TimeDate);
                            FormatAndDisplayTime(&TimeDate);
                            //Clear Flags
                            if ((BtnFlag[BtnU]==Click) || (BtnFlag[BtnU]==DblClick)) BtnFlag[BtnU] = BtnIdle;                              
                            if ((BtnFlag[BtnU]==LPress)&&(FastAdj_Tick)) FastAdj_Tick = 0;
                        }
                        
                         //EP -> Decrementare element ajustat
                        else if ( (BtnFlag[BtnD]==Click) || (BtnFlag[BtnD]==DblClick) || ((BtnFlag[BtnD]==LPress)&&(FastAdj_Tick))  ){
                           
                            //EP -> Generare sunet de frecventa mai joasa (frecv, length, pauza, repeatTimes)
                            Beep(1000, 100, 100, 1);
                            
                            TimeDate.Month = DecItem(TimeDate.Month, 1, 12);
                            //Reactualizam ceasul, daca s-a decrementat cu Click
                            //sau la iesirea din LPress
                            if ((BtnFlag[BtnD]==Click)|| (BtnFlag[BtnD]==LpRelease))
                                breakTime(makeTime(&TimeDate), &TimeDate);
                            FormatAndDisplayTime(&TimeDate);
                            //Clear Flags
                            if ((BtnFlag[BtnD]==Click) || (BtnFlag[BtnD]==DblClick)) BtnFlag[BtnD] = BtnIdle;                              
                            if ((BtnFlag[BtnD]==LPress)&&(FastAdj_Tick)) FastAdj_Tick = 0;
                        }
                        //final decrementare
        break;      
        case stAdjDay  : 
                        //Iesire din meniu
                        if (BtnFlag[BtnC] == LPress){//Testam LPress BtnC = iesire din meniu
                            WatchState = stWatch;
                            //Clear Flag
                            BtnFlag[BtnC] = BtnIdle;
                        }
                        //Schimbarea elementului, care se ajusteaza
                        else if ( (BtnFlag[BtnL]==Click) || (BtnFlag[BtnL]==DblClick)|| ((BtnFlag[BtnL]==LPress)&&(FastBlink_Tick)) ){
                            WatchState = stAdjSec;
                            FastBlink_ON = 1;//pentru ca urmatorul element de ajustat sa apara prima data "stins" 
                            FormatAndDisplayTime(&TimeDate);
                            //Clear Flags
                            if ((BtnFlag[BtnL]==Click)|| (BtnFlag[BtnL]==DblClick)) BtnFlag[BtnL] = BtnIdle;                            
                            if ((BtnFlag[BtnL]==LPress)&&(FastBlink_Tick)) FastBlink_Tick = 0;
                        }
                        
                        //EP -> Schimbare elementului care se ajusteaza, spre dreapta (lunile)
                        else if ( (BtnFlag[BtnR] == Click) || (BtnFlag[BtnR] == DblClick)
                                ||((BtnFlag[BtnR] == LPress) && (FastBlink_Tick)) ) {
                            WatchState = stAdjMonth;
                            FastBlink_ON = 1;
                            FormatAndDisplayTime(&TimeDate);
                            
                            if ((BtnFlag[BtnR]==Click)|| (BtnFlag[BtnR]==DblClick)) BtnFlag[BtnR] = BtnIdle;                            
                            if ((BtnFlag[BtnR]==LPress)&&(FastBlink_Tick)) FastBlink_Tick = 0;
                        }
                // final implementare
                        
                        //Incrementare element ajustat
                        else if ( (BtnFlag[BtnU]==Click) || (BtnFlag[BtnU]==DblClick) || ((BtnFlag[BtnU]==LPress)&&(FastAdj_Tick))  ){
                            //luam separat ani bisecti, Feb 29
                            if ((TimeDate.Month == 2) && (LEAP_YEAR(TimeDate.Year)))
                                        TimeDate.Day = IncItem(TimeDate.Day, 1, 29);
                            else TimeDate.Day = IncItem(TimeDate.Day, 1, monthDays[TimeDate.Month-1]);
                            //Reactualizam ceasul, daca s-a incrementare cu Click
                            //sau la iesirea din LPress
                            if ((BtnFlag[BtnU]==Click)|| (BtnFlag[BtnU]==LpRelease))
                                breakTime(makeTime(&TimeDate), &TimeDate);
                            FormatAndDisplayTime(&TimeDate);
                            //Clear Flags
                            if ((BtnFlag[BtnU]==Click) || (BtnFlag[BtnU]==DblClick)) BtnFlag[BtnU] = BtnIdle;                              
                            if ((BtnFlag[BtnU]==LPress)&&(FastAdj_Tick)) FastAdj_Tick = 0;
                        }
                        
                          //EP -> Decrementare element ajustat
                        else if ( (BtnFlag[BtnD]==Click) || (BtnFlag[BtnD]==DblClick) || ((BtnFlag[BtnD]==LPress)&&(FastAdj_Tick))  ){
                        
                            //EP -> Generare sunet de frecventa mai joasa (frecv, length, pauza, repeatTimes)
                            Beep(1000, 100, 100, 1);
                            
                            //luam separat ani bisecti, Feb 29
                            if ((TimeDate.Month == 2) && (LEAP_YEAR(TimeDate.Year)))
                                        TimeDate.Day = DecItem(TimeDate.Day, 1, 29);
                            else TimeDate.Day = DecItem(TimeDate.Day, 1, monthDays[TimeDate.Month-1]);
                            //Reactualizam ceasul, daca s-a decrementat cu Click
                            //sau la iesirea din LPress
                            if ((BtnFlag[BtnD]==Click)|| (BtnFlag[BtnD]==LpRelease))
                                breakTime(makeTime(&TimeDate), &TimeDate);
                            FormatAndDisplayTime(&TimeDate);
                            //Clear Flags
                            if ((BtnFlag[BtnD]==Click) || (BtnFlag[BtnD]==DblClick)) BtnFlag[BtnD] = BtnIdle;                              
                            if ((BtnFlag[BtnD]==LPress)&&(FastAdj_Tick)) FastAdj_Tick = 0;
                        }
        break;         
        default: break;
    }
 
}

uint8_t IncItem(uint8_t Item, uint8_t Lower, uint8_t Upper){
    uint8_t Temp_Item = Item;
    
    Temp_Item++;
    if (Temp_Item > Upper) Temp_Item = Lower;
    
    return Temp_Item;
}
uint8_t DecItem(uint8_t Item, uint8_t Lower, uint8_t Upper){
    
    uint8_t Temp_Item = Item;
    
    //EP -> So we avoid getting incremented with 255
    if(Temp_Item != 0) { 
    Temp_Item--;
    }
    else {
        Temp_Item = Upper;
    }
    if (Temp_Item < Lower) Temp_Item = Upper;
    
    
    
    
    
    return Temp_Item;
    
}

