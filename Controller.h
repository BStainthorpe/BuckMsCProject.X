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
    
//closed loop control settings 
#define TARGET_VOLTAGE_MV_1         10000u         //target voltage in millivolts
#define TARGET_VOLTAGE_MV_2         16000u         //option to change target voltage for step response using CL_Enable Jumper
#define PID_OFFSET                  50u            //make the PI controller operate around 50% duty (negative output corresponds to 0-50%, positive 50-100%)
#define INTEGRAL_SCALED_LIMIT       1073741824u    //anti windup integrator limit for scaled value, calculate using max duty cycle (1024) scaled << DT_MANTISSA + VOLTAGE_MODE_KI_MANTISSA
#define VOLTAGE_MODE_CONTROL_PERIOD 79u            //79 corresponds to 100kHz
#define VOLTAGE_MODE_KP             18u            //GAIN OF 18/(2^10) = 0.017578 tuned using ziegler nichols
#define VOLTAGE_MODE_KP_MANTISSA    10u
#define VOLTAGE_MODE_KI             5u             //GAIN OF 5/(2^4) = 0.3125 tuned using ziegler nichols
#define VOLTAGE_MODE_KI_MANTISSA    4u
    
    
//unused
#define CURRENT_MODE_CONTROL_PERIOD 79u            //79 corresponds to 100kHz
#define CURRENT_MODE_KP             10u
#define CURRENT_MODE_KP_MANTISSA    0u
#define CURRENT_MODE_KI             0u 
#define CURRENT_MODE_KI_MANTISSA    10u
    
//need a DT multiplication of 1/490 = 0.00204
//to get required accuracy with gain/mantissa need to bit shift by 16, 0.00204 * 2^16 (65536) = 133.747
#define DT_GAIN     134u    
#define DT_MANTISSA 16u
    
//the output voltage is scaled according to rawValue = Vout * (100k/(100k+390k)) * 1024/5 
//to obtain milli volts from the raw value, mV = rawValue * 5/1024 * ((100k+390k)/100k) * 1000 = * 23.925 = (rawValue * 6100) >> 8, no offset required
#define VOLTAGE_SENSOR_GAIN         6100u
#define VOLTAGE_SENSOR_MANTISSA     8u
#define VOLTAGE_SENSOR_OFFSET       10u   //found to be a 237mV error, divide by 23.925 gives 10
    
#define SIZE_OF_VSENSOR_FILTER      16      //note there are variables in the filter functions which require change depending on this number 
    
uint16_t filteredVout = 0;
uint16_t voutFIFO[SIZE_OF_VSENSOR_FILTER];

typedef struct controllerVariables{
    int16_t error;
    int32_t integral;
    int32_t proportionalOutput;
    int32_t integralOutput;
    int64_t integralOutputScaled;
    int32_t sumOutput;
    int16_t previousError;           
};

uint16_t readFilteredVout();
int16_t convertRawToMilliVolts(uint16_t rawValue);
void controlRoutine();
void runCurrentModeControl();
void runVoltageModeControl();
void initialiseController();

#ifdef	__cplusplus
}
#endif

#endif	/* CONTROLLER_H */

