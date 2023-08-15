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

//the current sensor conversion formula is Vout = Voff + Iin x 400mV/A, where Voff = 2.5
//to avoid floats, we can use gains and exponent to reduce memory consumption
//Voffset = 512 (ADC equivalent value, 2.5 / 5 * 1024)
//current (mA)  = (vout - vref) * (1/0.4) * 1000
//current (mA) = (ADC_value - 512) * (5/1024) * (1/0.4) * 1000
//current (mA) = (ADC_value - 512) * 12.207
//gain of 3125, exponent of 8, gives real gain of 12.20703
//use signed ints as the calculated value can be negative   
#define CURRENT_SENSOR_GAIN         3125u 
#define CURRENT_SENSOR_EXPONENT     8u
#define CURRENT_SENSOR_OFFSET       512u 
    
#define SIZE_OF_ISENSOR_FILTER      16u     //size of FIFO filter, must be a power of 2, also change ISENSOR_SHIFT accordingly
#define ISENSOR_SHIFT               4u      //squareroot(SIZE_OF_ISENSOR_FILTER) = 4  
    
#define CURRENT_TRIP_LIMIT  3u              //max number of consecutive current trips before transitioning to a fault
                                            //allow >1 as switching inductor and turn on with high duty cycle causes overcurrent due to inrush but this is OK
    
volatile uint16_t latestIL = 0;             //variable containing latest IL1 sample, which is read once per PWM cycle if CCP1 interrupt used

uint16_t filteredIDS = 0;                   //filtered current measurements and FIFO
uint16_t filteredIL = 0;
uint16_t currentIDSFIFO[SIZE_OF_ISENSOR_FILTER];
uint16_t currentILFIFO[SIZE_OF_ISENSOR_FILTER];

bool tripIDS = 0;
bool tripIL = 0;

//variable counting number of consecutive current trips
uint8_t currentTripCount = 0;

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

