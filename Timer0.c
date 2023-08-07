/* 
 * File:   Timer0.c
 * Author: Ben Stainthorpe
 *
 * Created on 20 January 2023, 16:11
 */

#include "Timer0.h"
#include "Global.h"
#include <xc.h> 

/*------------------------------------------------------------------------------
 Function: setupTimer0Interrupt()
 *Use: This function initialises the registers as required for timer0 to count 
 * with a prescaler of 64, and trigger the interrupt function in main
------------------------------------------------------------------------------*/
void setupTimer0Interrupt(){
    
    INTCONbits.GIE = 1;     //enable global interrupts
    INTCONbits.TMR0IE = 1;  //enable timer0 interrupt
    OPTION_REGbits.TMR0CS = 0;   //set TIMER0 to internal counter
    OPTION_REGbits.PSA = 0;      //set prescaler as active
    OPTION_REG = 0b101;         //set the prescaler as 64 this gives interrupt frequency as clockFrequency / (4*64*255) = 490Hz
    
}