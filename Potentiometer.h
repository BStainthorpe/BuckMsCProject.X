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

//potentiometer generator settings
#define MIN_PERIOD_FROM_POT    15u                //PR2 = (clockFrequency / (4*freq)) - 1 corresponds to 500,000 Hz
#define MAX_PERIOD_FROM_POT    180u               //corresponds to 50kHz with some added extra for contingency (so 50kHz can definitely be achieved)
    
#define SIZE_OF_POT_FILTER  16      //size of FIFO filter, must be a power of 2, also change POT_SENSOR_SHIFT accordingly
#define POT_SENSOR_SHIFT    4u      //squareroot(SIZE_OF_POT_FILTER) = 4  
    
    
#define POT_OFFSET          51      //experimentally obtained minimum pot value 51
#define POT_MAX             1019    //experimentally obtained max pot value
#define POT_SET_DIVIDER     32      //control the rate of execution for setting the freq and duty from the POT - this value gives a rate of: Slot 4 Freq / POT_SET_DIVIDER
                                    //this results in a low update rate 
#define POT_GAIN            270     //correct the pot according to min and max
#define POT_EXPONENT        8       // gain = 1024 / (1019 - 51)) = 1.05567, gain 270, exponent 8, 270 / 256 = 1.054

    
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

