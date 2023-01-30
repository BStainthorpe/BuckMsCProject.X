/* 
 * File:   PWM.c
 * Author: Ben Stainthorpe
 *
 * Created on 20 January 2023, 16:11
 */
#include "PWM.h"
#include "Global.h"
#include <math.h>

/*------------------------------------------------------------------------------
 Function: setupPWM()
 *Use: This function initialises the registers as required for a PWM to be
 * generated from timer2 onto pin RA6, with no prescaler
------------------------------------------------------------------------------*/
void setupPWM(){
    
    CCP1CON = 0b00001100;          //Compare Mode: Initialise ECCPx pin high, clear output on compare match CCPxIF
    
    PR2 = 0;                       //Period setting for timer 2, initialise as 0     
    CCPR1L = 0;                    //duty setting for timer 2, initialise as 0

    APFCON0bits.P1DSEL = 1;        //setup Alternate Pin Function Register bit for PWM output P1D on RA6
    PSTR1CONbits.STR1D = 1;        //Steer P1D from CCP1
    
    T2CONbits.T2CKPS = 0b00;       //1:1 prescaler on timer 1
    PIE1bits.TMR2IE = 0;
    T2CONbits.TMR2ON = 1;          //start timer2 for the PWM 
    
    INTCONbits.PEIE = 1;           //enable peripheral interrupts
    PIE1bits.CCP1IE = 1;           //enable an interrupt on CCP1 match event (falling edge of PWM)  
    
    //PIE1bits.TMR2IE = 1; //enable interrupt on PWM cycle finish to trigger ADC read in interrupt routine 
    
    initialiseGPIO(gpioPWMout, GPIO_Output);    //set the corresponding RA6 gpio as a digital output
}

/*------------------------------------------------------------------------------
 Function: setPWMdutyandPeriod(dutyCycle, period)
 *Use: This function initialises the registers as required for a PWM to output
 * specified period and duty period in raw register values
 * frequency is determined by the clock frequency
 * freq = clockFrequency / ((PR2 + 1) * 4)
 * Duty cycle is given by the below formula
 * DutyCycle = (CCPR1L:CCP1CON<5:4> / (4*PR2+1) * 100
 * CCPR1L:CCP1CON<5:4> = (4*PR2 * (DutyCycle(%)) / 100) - 1
------------------------------------------------------------------------------*/
void setPWMDutyandPeriod(uint16_t dutyCycle, uint8_t period){
    PR2 = period;
    CCPR1L = dutyCycle >> 2;
    CCP1CONbits.DC1B0 = dutyCycle & 1;
    CCP1CONbits.DC1B1 = (dutyCycle & 2) > 1;
}

/*------------------------------------------------------------------------------
 Function: setPWMPeriod(period)
 *Use: This function initialises the registers as required for a PWM to output
 * specified user period in raw register values
 * frequency is determined by the clock frequency
 * freq = clockFrequency / ((PR2 + 1) * 4)
 * PR2 = (clockFrequency / (4*freq)) - 1
------------------------------------------------------------------------------*/
void setPWMPeriod(uint8_t period){
    PR2 = period;
}