/* 
 * File:  Global.h
 * Author: Ben Stainthorpe
 *
 * Created on 20 January 2023, 16:11
 */

#ifndef GLOBAL_H
#define	GLOBAL_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "GPIO.h"
#include "StateMachine.h"
    
//oscillator settings
#define CLOCK_FREQUENCY_SELECT  freq32M           //this should be left as 32MHz - lower frequencies limit PWM freq    
#define _XTAL_FREQ CLOCK_FREQUENCY_SELECT         //used by delay function
    
    
//List all outputs here to keep track - gpio defines are critical, pins are for indication
    //PWM
#define gpioPWMout                pinRA6
#define pinPWMout                 15
#define gpioSlotTest              pinRB4
#define pinSlotTest               10
#define gpioSlotTest2             pinRB5
#define pinSlotTest2              11
    
    //Digital IO
//Current Sensors
#define gpioCurrentTripIL         pinRA3
#define pinCurrentTripIL          2
#define gpioCurrentTripIDS        pinRA1
#define pinCurrentTripIDS         18
#define gpioOverCurrentClear      pinRB3
#define pinOverCurrentClear       9
//Control Select
#define gpioControlSelect         pinRB0
#define pinControlSelect          6
    
    //ADCs
//Potentiometers
#define gpioPotentiometerDuty     pinRB1
#define pinPotentiometerDuty      7
#define gpioPotentiometerFreq     pinRB2
#define pinPotentiometerFreq      8
//Current Sensors
#define gpioIDSCurrent            pinRA0
#define pinIDSCurrent             17
#define gpioILCurrent             pinRA2
#define pinILCurrent              1
//Voltage Sensor                  
#define gpioOutputVoltage         pinRA4
#define pinOutputVoltage          3
    
//the list of clock frequency options    
enum internalClockFreqSelec{
    freq31k,
    freq62k5,
    freq125k,
    freq250k,
    freq500k,
    freq1M,
    freq2M,
    freq4M,
    freq8M,
    freq16M,
    freq32M    
}; 

//global variable containing current clock frequency selection
uint32_t clockFrequency = 0;

#ifdef	__cplusplus
}
#endif

#endif	/* GLOBAL_H */

