
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */


#include "Buttons.h"
#include "Beep.h"
#include <xc.h>
#include <sys/attribs.h>


volatile enum BtnFsmStates BtnFSMState[NumButtons];

volatile enum BtnValues BtnVal[NumButtons];

volatile enum BtnFlagTypes BtnFlag[NumButtons];

volatile unsigned short BtnTimeCount[NumButtons];


void __ISR(_CHANGE_NOTICE_VECTOR, IPL4AUTO) ChangeNoticeISR(void){
    uint32_t Temp;
    //Test BtnU = RB1, BtnL = RB0
    if (IFS1bits.CNBIF){
        if (CNSTATBbits.CNSTATB1){//BtnU changed
            if (BtnTimeCount[BtnU] >= STABLE_TIME_TICKS){
                BtnVal[BtnU] = (PORTBbits.RB1)? BtnPressed : BtnReleased;
            }
            BtnTimeCount[BtnU]=0;        
        }
        if (CNSTATBbits.CNSTATB0){//BtnL changed
            if (BtnTimeCount[BtnL] >= STABLE_TIME_TICKS){
                BtnVal[BtnL] = (PORTBbits.RB0)? BtnPressed : BtnReleased;
            }
            BtnTimeCount[BtnL]=0;             
        }
        // EP -> implementation of the Right Button, pin RB8, port B
        if(CNSTATBbits.CNSTATB8) { //BtnR changed
            if(BtnTimeCount[BtnR] >= STABLE_TIME_TICKS) {
                BtnVal[BtnR] = (PORTBbits.RB8) ? BtnPressed : BtnReleased;
        }
            BtnTimeCount[BtnR] = 0;
        }
        //Read port to clear CNSTAT bits
        Temp = PORTB;
        //Interrupt Acknowledge pentru Change Notice B
        IFS1bits.CNBIF = 0;
    }
    
    //Test BtnC = RF0
    if (IFS1bits.CNFIF){
        if(CNSTATFbits.CNSTATF0){//BtnC changed
            if (BtnTimeCount[BtnC] >= STABLE_TIME_TICKS){
                BtnVal[BtnC] = (PORTFbits.RF0)? BtnPressed : BtnReleased;
            }
            BtnTimeCount[BtnC]=0;          
        }
        //Read port to clear CNSTAT bits
        Temp = PORTF;
        //Interrupt Acknowledge pentru Change Notice B
        IFS1bits.CNFIF = 0;
    }
    
    //Test BtnD = RA15
    if (IFS1bits.CNAIF){
        if(CNSTATAbits.CNSTATA15){//BtnC changed
            if (BtnTimeCount[BtnD] >= STABLE_TIME_TICKS){
                BtnVal[BtnD] = (PORTAbits.RA15)? BtnPressed : BtnReleased;
            }
            BtnTimeCount[BtnD]=0;          
        }
        //Read port to clear CNSTAT bits
        Temp = PORTA;
        //Interrupt Acknowledge pentru Change Notice B
        IFS1bits.CNAIF = 0;
    }
} 

void BtnFSM(enum Buttons Btn){
    switch (BtnFSMState[Btn]){
    case stBtnIdle: if (BtnVal[Btn] == BtnPressed){
                        BtnFSMState[Btn] = stBtnPressed;
                        Beep(2000, 50, 50, 1);
                    }
                    break;
    case stBtnPressed: if (BtnVal[Btn] != BtnPressed){
                            BtnFSMState[Btn] = stBtnClick;
                            BtnFlag[Btn] = Click;
                        }
                      else
                        if(BtnTimeCount[Btn] == LONGPRESS_TICKS){
                               BtnFSMState[Btn] = stBtnLongPress;
                               Beep(2000, 200, 50, 1);
                               BtnFlag[Btn] = LPress;
                        }
                    break;
    case stBtnClick: if (BtnTimeCount[Btn] >= DBLCLICK_TIMEOUT_TICKS)
                                BtnFSMState[Btn] = stBtnIdle;
                         else if(BtnVal[Btn] == BtnPressed){
                             BtnFSMState[Btn] = stBtnDblClick;
                             Beep(2500, 50, 50, 2);
                             BtnFlag[Btn] = DblClick;
                         }
                         break;
    case stBtnDblClick: if (BtnVal[Btn] == BtnReleased)
                        BtnFSMState[Btn] = stBtnIdle;
                        break;
    case stBtnLongPress: if (BtnVal[Btn] == BtnReleased){
                            BtnFSMState[Btn] = stBtnIdle;
                            BtnFlag[Btn] = LpRelease;
                        }
                       break;
    default : break;
    } 
}



void Init_Buttons(){
    enum Buttons Btn;
    //Initializam toate variabilele legate de butoane
    for (Btn = BtnC; Btn < NumButtons; Btn++){
        BtnFSMState[Btn]= stBtnIdle;
        BtnVal[Btn]=BtnReleased;
        BtnFlag[Btn]=BtnIdle;
        BtnTimeCount[Btn]=0;
    } 

    //Validam intreruperile CNINT
    IEC1bits.CNBIE = 1; //BtnU si BtnL si BtnR
    IEC1bits.CNFIE = 1; //BtnC
    IEC1bits.CNAIE = 1; //BtnD
    //Prioritate si subprioritate
    IPC8bits.CNIP = 4;
    IPC8bits.CNIS = 2;
    
    //BtnU = RB1
    CNENBbits.CNIEB1 = 1;
    //BtnL = RB0
    CNENBbits.CNIEB0 = 1;
    //EP -> BtnR = RB8
    CNENBbits.CNIEB8 = 1;
    //BtnC = RF0
    CNENFbits.CNIEF0 = 1;
    //BtnD = RA15
    CNENAbits.CNIEA15 = 1;
    
    //Pornim modulele CNCON
    //pe porturile B, F si A
    CNCONBbits.ON = 1;
    CNCONFbits.ON = 1;
    CNCONAbits.ON = 1;
    
    
}


