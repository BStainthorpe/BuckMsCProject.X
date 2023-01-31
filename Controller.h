/* 
 * File:   Controller.h
 * Author: Ben Stainthorpe
 *
 * Created on 23 January 2023, 15:34
 */

#ifndef CONTROLLER_H
#define	CONTROLLER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "GPIO.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>   
#include "Global.h"    
    
//the output voltage is scaled according to rawValue = Vout * (100k/(100k+390k)) * 1024/5 
//to obtain deci volts from the raw value, dV = rawValue * 5/1024 * ((100k+390k)/100k) * 10 = (rawValue * 245) >> 10, no offset required
#define VOLTAGE_SENSOR_GAIN         245u
#define VOLTAGE_SENSOR_MANTISSA     10u
#define VOLTAGE_SENSOR_OFFSET       0u
    
#define SIZE_OF_VSENSOR_FILTER      16      //note there are variables in the filter functions which require change depending on this number    
    
uint16_t filteredVout = 0;
uint16_t voutFIFO[SIZE_OF_VSENSOR_FILTER];

uint16_t readFilteredVout();
int16_t convertRawToDeciVolts(uint16_t rawValue);


#ifdef	__cplusplus
}
#endif

#endif	/* CONTROLLER_H */

