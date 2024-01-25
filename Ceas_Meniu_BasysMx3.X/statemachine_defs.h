/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    config_bits.h

  @Summary
    Brief description of the file.

  @Description
    Bitii de configurare ale microcontrolerului
 */
/* ************************************************************************** */

#ifndef _CONFIG_BITS_H    /* Guard against multiple inclusion */
#define _CONFIG_BITS_H


/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

    // 'C' source line config statements


    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif


// PIC32MX370F512L Configuration Bit Settings



#pragma config JTAGEN = OFF     
#pragma config FWDTEN = OFF      

/* ------------------------------------------------------------ */
/*						Configuration Bits		                */
/* ------------------------------------------------------------ */
// Device Config Bits in  DEVCFG1:	
#pragma config FNOSC =	FRCPLL
#pragma config FSOSCEN =	OFF
#pragma config POSCMOD =	XT
#pragma config OSCIOFNC =	ON
#pragma config FPBDIV =     DIV_2

// Device Config Bits in  DEVCFG2:	
#pragma config FPLLIDIV =	DIV_2
#pragma config FPLLMUL =	MUL_20
#pragma config FPLLODIV =	DIV_1

//// DEVCFG3
//#pragma config USERID = 0xFFFF          // Enter Hexadecimal value (Enter Hexadecimal value)
//#pragma config FSRSSEL = PRIORITY_7     // Shadow Register Set Priority Select (SRS Priority 7)
//#pragma config PMDL1WAY = ON            // Peripheral Module Disable Configuration (Allow only one reconfiguration)
//#pragma config IOL1WAY = ON             // Peripheral Pin Select Configuration (Allow only one reconfiguration)
//
//// DEVCFG2
//#pragma config FPLLIDIV = DIV_2         // PLL Input Divider (2x Divider)
//#pragma config FPLLMUL = MUL_20         // PLL Multiplier (20x Multiplier)
//#pragma config FPLLODIV = DIV_1         // System PLL Output Clock Divider (PLL Divide by 1)
//
//// DEVCFG1
//#pragma config FNOSC = FRCPLL           // Oscillator Selection Bits (Fast RC Osc with PLL)
//#pragma config FSOSCEN = OFF            // Secondary Oscillator Enable (Disabled)
//#pragma config IESO = ON                // Internal/External Switch Over (Enabled)
//#pragma config POSCMOD = XT             // Primary Oscillator Configuration (XT osc mode)
//#pragma config OSCIOFNC = ON            // CLKO Output Signal Active on the OSCO Pin (Enabled)
//#pragma config FPBDIV = DIV_2           // Peripheral Clock Divisor (Pb_Clk is Sys_Clk/2)
//#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor Selection (Clock Switch Disable, FSCM Disabled)
//#pragma config WDTPS = PS1048576        // Watchdog Timer Postscaler (1:1048576)
//#pragma config WINDIS = OFF             // Watchdog Timer Window Enable (Watchdog Timer is in Non-Window Mode)
//#pragma config FWDTEN = OFF             // Watchdog Timer Enable (WDT Disabled (SWDTEN Bit Controls))
//#pragma config FWDTWINSZ = WINSZ_25     // Watchdog Timer Window Size (Window Size is 25%)
//
//// DEVCFG0
//#pragma config DEBUG = OFF              // Background Debugger Enable (Debugger is Disabled)
//#pragma config JTAGEN = OFF             // JTAG Enable (JTAG Disabled)
//#pragma config ICESEL = ICS_PGx1        // ICE/ICD Comm Channel Select (Communicate on PGEC1/PGED1)
//#pragma config PWP = OFF                // Program Flash Write Protect (Disable)
//#pragma config BWP = OFF                // Boot Flash Write Protect bit (Protection Disabled)
//#pragma config CP = OFF                 // Code Protect (Protection Disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.




#endif /* _CONFIG_BITS_H */

/* *****************************************************************************
 End of File
 */
