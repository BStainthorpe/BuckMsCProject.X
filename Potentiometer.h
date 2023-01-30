/* 
 * File:   Potentiometer.h
 * Author: Ben Stainthorpe
 *
 * Created on 23 January 2023, 15:33
 */

#ifndef POTENTIOMETER_H
#define	POTENTIOMETER_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "Global.h"
#include "GPIO.h"
#include "ADC.h"
#include <stdbool.h>
    
#define SIZE_OF_POT_FILTER  16      //note there are variables in the filter functions which require change depending on this number
#define POT_OFFSET          45      //experimentally obtained minimum pot value
#define POT_SET_DIVIDER     32      //control the rate of execution for setting the POT - this value gives a rate of: Slot 4 Freq / POT_SET_DIVIDER
    
uint8_t potSetCount = 0;

void initialisePotentiometers();
uint16_t readFilteredDutyPot();
uint16_t readFilteredFreqPot();
void runPotScaling();

uint16_t filteredFreqPot = 0;
uint16_t filteredDutyPot = 0;
uint16_t freqPotFIFO[SIZE_OF_POT_FILTER];
uint16_t dutyPotFIFO[SIZE_OF_POT_FILTER];


#ifdef	__cplusplus
}
#endif

#endif	/* POTENTIOMETER_H */

