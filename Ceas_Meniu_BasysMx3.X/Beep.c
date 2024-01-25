
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

#include "Timers.h"
#include "config.h"
#include "Beep.h"
#include <xc.h>
#include <sys/attribs.h>

enum BeepStates{stStop = 0, stBeep, stPause};
volatile enum BeepStates BeepState = stStop;

volatile uint16_t BeepFrequency_Hz = 0;
volatile uint16_t BeepLength_ms = 0;
volatile uint16_t BeepTimeCounter = 0;
volatile uint16_t BeepPauseTime_ms = 0;
volatile uint16_t PauseTimeCounter = 0;
volatile uint16_t BeepRepeatTimes = 1;
//flag to start Beep
volatile uint8_t StartBeep = 0;

extern uint8_t Beep_Timebase_Tick;


//AUDIO_ConfigurePins() pentru a seta OC1 la pinul de iesire audio
void AUDIO_ConfigurePins()
{
    // Configure AUDIO output as digital output.
    tris_A_OUT = 0;
    //Mapam pinul A_OUT = RPB14R: PORTB, PIN 14 la OC1 
    rp_A_OUT = 0x0C; // 1100 = OC1
    // disable analog (set pins as digital)
    ansel_A_OUT = 0;
}


//Corpul ISR Timer3 ar fi trebuit sa fie in Timers.c
//Insa e mutat aici, deoarece corpul lui contine temporizarea sunetului
void __ISR(_TIMER_3_VECTOR, IPL7AUTO) Timer3ISR(void) 
{  
    //Prima data decrementam, apoi verificam, daca a ajuns la 0
    if (!(--BeepTimeCounter)){//numaratorul a ajuns la 0
        //Oprim Timer 3 - automat se opreste sunetul
        T3CONbits.ON = 0;
    }
    
     IFS0bits.T3IF = 0;      // clear Timer3 interrupt flag
}

void Beep(uint16_t Frequency_Hz, uint16_t Length_ms, uint16_t PauseTime_ms, uint8_t RepeatTimes){
    
    if (BeepState == stStop){//configuram doar daca nu este beep in derulare
        //coduri de protectie
        //Actualizam variabilele globale
        if (Frequency_Hz < 100) BeepFrequency_Hz = 100;
        else if (Frequency_Hz > 20000) BeepFrequency_Hz = 20000;
        else BeepFrequency_Hz = Frequency_Hz;
        
        if (Length_ms == 0) BeepLength_ms = 1;
        else BeepLength_ms = Length_ms;
        
        if (PauseTime_ms == 0) BeepPauseTime_ms = 1;
        else BeepPauseTime_ms = PauseTime_ms;
        //PauseTimeCounter se va decrementa pe Timer5 - perioada de 1ms
        PauseTimeCounter = PauseTime_ms;
        
        //calculam BeepTimeCounter: Durata sunetului in ms = Perioada Timer in ms * BeepTimeCounter 
        //=> BeepTimeCounter = (Durata sunetului) / (Perioada Timer in ms) = ((Durata Sunetului) * (Frecventa Timer in Hz))/1000
        BeepTimeCounter = (BeepLength_ms * BeepFrequency_Hz) / 1000;
        
        //Cod de protectie: La RepeatTimes = 0 nu pornim Beep
        if (RepeatTimes > 0){
            BeepRepeatTimes = RepeatTimes;
           //Pornim automatul secvential BeepFSM():
            StartBeep = 1;
            //Reconfiguram Timer - va porni sunetul
            ReConfigureTimer3(Frequency_Hz);
        }
        
    }//if (BeepState == stStop)  
}


//BeepFSM() va fi  rulat in ISR Timer 5 - perioada de 1ms
void BeepFSM(){
    switch(BeepState){
        case stStop: if (StartBeep){
                        BeepState = stBeep;
                        StartBeep = 0;
                    }
                    break;
        case stBeep:if (!BeepTimeCounter){//daca s-a terminat perioada curenta de Beep
                        if(!(--BeepRepeatTimes)){//testam,daca s-a terminat numarul de rapetari
                            BeepState = stStop;
                        }
                        else BeepState = stPause;
                    }
                    break;
        case stPause:if (!(--PauseTimeCounter)){//decrementam si testam daca s-a terminat pauza
                        //Atunci, pornim din nou Beep, cu daatele din variabilele globale
                        BeepTimeCounter = (BeepLength_ms * BeepFrequency_Hz) / 1000;
                        PauseTimeCounter = BeepPauseTime_ms;
                         //Reconfiguram Timer - va porni sunetul
                        ReConfigureTimer3(BeepFrequency_Hz);                       
                        BeepState = stBeep;
                    }
                    break;
        default: break;    
    }
}