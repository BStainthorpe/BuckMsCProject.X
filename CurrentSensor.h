/* 
 * File:   CurrentSensor.h
 * Author: Ben Stainthorpe
 *
 * Created on 20 January 2023, 16:11
 */

#ifndef CURRENTSENSOR_H
#define	CURRENTSENSOR_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "GPIO.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>   
#include "Global.h"

//the current sensor conversion formula is Vout = Vref + Iin x 350mV/A, where Vref = 2.175
//to avoid floats, we can use gains and mantissa to reduce memory consumption
//Voffset = 445 (ADC equivalent value, 2.175 / 5 * 1024)
//current (mA)  = (vout - vref) * (1/0.350) * 1000
//current (mA) = (ADC_value - 445) * 2857, no mantissa required
//use signed ints as the calculated value can be negative
    
#define CURRENT_SENSOR_GAIN         2857
#define CURRENT_SENSOR_MANTISSA     0
#define CURRENT_SENSOR_OFFSET       445    
#define SIZE_OF_ISENSOR_FILTER      16      //note there are variables in the filter functions which require change depending on this number    
    
volatile uint16_t latestIL = 0;       //variable containing latest IL1 sample, which is read once per PWM cycle
uint16_t filteredIDS = 0;
uint16_t filteredIL = 0;
uint16_t currentIDSFIFO[SIZE_OF_ISENSOR_FILTER];
uint16_t currentILFIFO[SIZE_OF_ISENSOR_FILTER];

bool tripIDS = 0;
bool tripIL = 0;
    
void initialiseCurrentSensors();
bool currentTripRead();
uint16_t readFilteredIDS();
uint16_t readFilteredIL();
void currentTripReset();
int16_t convertRawToMilliAmps(uint16_t rawvalue);


#ifdef	__cplusplus
}
#endif

#endif	/* CURRENTSENSOR_H */

