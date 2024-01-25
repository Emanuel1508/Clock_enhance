
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

#include <sys/attribs.h>
#include "Timers.h"
#include "Buttons.h"
#include "Beep.h"

//din Buttons.c
extern enum BtnFsmStates BtnFSMState[NumButtons];

extern enum BtnValues BtnVal[NumButtons];

extern enum BtnFlagTypes BtnFlag[NumButtons];

extern unsigned short BtnTimeCount[NumButtons];


//din Beep.c
extern uint16_t BeepFrequency_Hz;
extern uint16_t BeepLength_ms;
extern uint32_t BeepTimeCounter;
extern uint16_t BeepPauseTime_ms;
extern uint16_t PauseTimeCounter;
extern uint16_t BeepRepeatTimes;
//flag to start Beep
extern uint8_t StartBeep;

void ConfigureTimer1(unsigned short Frequency){
    
    TMR1 = 0;
    PR1 = (int)(((float)(float)PB_FRQ/Frequency) - 0.5);
    
    T1CONbits.TCKPS = 0;     //1:1 prescale value
    T1CONbits.TGATE = 0;     //not gated input (the default)
    T1CONbits.TCS = 0;       //PCBLK input (the default)
    IPC1bits.T1IP = 1;      // interrupt priority
    IPC1bits.T1IS = 1;      // interrupt subpriority
    IEC0bits.T1IE = 1;      // enable Timer1 interrupt    
    IFS0bits.T1IF = 0;      // clear Timer1 interrupt flag
    
    T1CONbits.ON = 1;        //turn on Timer1
  
}


//Corpul ISR Timer1 ar fi trebuit sa fie aici
//Insa e mutat in programul principal (Ceas_Meniu.c), deoarece corpul lui contine codul bazelor de timp ale ceasului




void ConfigureTimer5(unsigned short Frequency){
    
    PR5 = (int)(((float)(float)PB_FRQ/Frequency) - 0.5);
    
    //Timer1 si Timer5 au aceeasi perioada.
    //Daca vor la acelasi timp (decalat de doar cateva instructiuni)
    //atunci vor genera intreruperi unul dupa altul, la distante mici
    //Din acest motiv pornim TMR5 de la 1/4 din perioada, ca sa decalam intreruperile
    TMR5 = (int) PR5/4;
    
    T5CONbits.TCKPS = 0;     //1:1 prescale value
    T5CONbits.TGATE = 0;     //not gated input (the default)
    T5CONbits.TCS = 0;       //PCBLK input (the default)
    IPC5bits.T5IP = 2;      // interrupt priority
    IPC5bits.T5IS = 1;      // interrupt subpriority
    IEC0bits.T5IE = 1;      // enable Timer5 interrupt    
    IFS0bits.T5IF = 0;      // clear Timer5 interrupt flag
    
    T5CONbits.ON = 1;        //turn on Timer1
  
}

//ISR Timer5 Pentru detectarea actiunilor butoanelor
//si rularea BeepFSM(), daca se elibereaza comanda Beep;
void __ISR(_TIMER_5_VECTOR, IPL2AUTO) Timer5ISR(void) 
{  
 enum Buttons Btn;
     //Testarea actiunilor butoanelor
     for (Btn = BtnU; Btn < NumButtons; Btn++){
        BtnTimeCount[Btn]++;
       BtnFSM(Btn);
    }
    
    //Automatul scvential pentru Beep
    BeepFSM();
 
     
    IFS0bits.T5IF = 0;      // clear Timer5 interrupt flag
}


//Timer3 pentru generare sunet
void ConfigureTimer3(unsigned short Frequency){
    
    T3CONbits.ON = 0;
    TMR3 = 0;
    if (Frequency > 650){
        T3CONbits.TCKPS = 0;     //1:1 prescale value
        PR3 = (int)(((float)(float)PB_FRQ/Frequency) - 0.5);
    }
    else{
        T3CONbits.TCKPS = 3;     //1:8 prescale value
        PR3 = (int)(((float)(float)PB_FRQ/(Frequency * 8)) - 0.5);
    }
    
    T3CONbits.TGATE = 0;     //not gated input (the default)
    T3CONbits.TCS = 0;       //PCBLK input (the default)
    
    IPC3bits.T3IP = 7;      // interrupt priority
    IPC3bits.T3IS = 3;      // interrupt subpriority
    IEC0bits.T3IE = 1;      // enable Timer3 interrupt    
    IFS0bits.T3IF = 0;      // clear Timer3 interrupt flag
    
    //Configure OC1 too
    OC1CONbits.ON = 0;       // Turn off OC1 while doing setup.
    OC1CONbits.OCM = 6;      // PWM mode on OC1; Fault pin is disabled
    OC1CONbits.OCTSEL = 1;   // Timer3 is the clock source for this Output Compare module
    OC1CONbits.ON = 1;       // Start the OC1 module  
    
    OC1RS = (int) PR3/2;     // Factor de umplere 50%
    
    //Don't start the timer yet
    
}
void ReConfigureTimer3(unsigned short Frequency){
    
    T3CONbits.ON = 0;
    TMR3 = 0;
    if (Frequency > 650){
        T3CONbits.TCKPS = 0;     //1:1 prescale value
        PR3 = (int)(((float)(float)PB_FRQ/Frequency) + 0.5);
    }
    else{
        T3CONbits.TCKPS = 3;     //1:8 prescale value
        PR3 = (int)(((float)(float)PB_FRQ/(Frequency * 8)) + 0.5);
    }
    
   //Configure OC1 too
    OC1CONbits.ON = 0;       // Turn off OC1 while doing setup.
    OC1RS = (int) PR3/2;     // Factor de umplere 50%
    OC1CONbits.ON = 1;       // Start the OC1 module  
    
    //Pornim Timer - va porni si sunetul
    T3CONbits.ON = 1;
    
    
}

//Corpul ISR Timer3 ar fi trebuit sa fie aici
//Insa e mutat in Beep.c, deoarece corpul lui contine temporizarea sunetului
