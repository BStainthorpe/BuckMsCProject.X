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

//select the closed loop control method    
#define VOLTAGE_MODE_CONTROL    1
#define CURRENT_MODE_CONTROL    0
#define CONTROL_METHOD          VOLTAGE_MODE_CONTROL       //used to guide state machine to enter voltage or current mode control,
                                                           //and compile switches removes code to reduce memory consumption
    
//closed loop control settings 
#define PID_OFFSET                  50u            //make the PI controller operate around 50% duty (negative output corresponds to 0-50%, positive 50-100%)
#define INTEGRAL_LIMIT              512u           //anti windup integrator limit, use 50% of max possible output (1024 / 2 = 512), this is then scaled up 
                                                   //to << DT_MANTISSA + VOLTAGE_MODE_KI_MANTISSA
//need a DT multiplication of 1/245 = 0.0040816
//to get required accuracy with gain/mantissa need to bit shift by 16, 0.0040816 * 2^16 (65536) = 267.49
#define DT_GAIN     267u    //GAIN of 267 / (2^16) = 0.004074  
#define DT_MANTISSA 16u
    
//voltage mode specific settings    
#define TARGET_VOLTAGE_MV_1         12000u         //target voltage in millivolts
#define TARGET_VOLTAGE_MV_2         16000u         //option to change target voltage for step response using CL_Enable Jumper
#define VOLTAGE_MODE_CONTROL_PERIOD 79u            //79 corresponds to 100kHz
#define VOLTAGE_MODE_KP             18u            //GAIN OF 18/(2^10) = 0.017578 tuned using ziegler nichols
#define VOLTAGE_MODE_KP_MANTISSA    10u
#define VOLTAGE_MODE_KI             15u            //GAIN OF 15/(2^6) = 0.234 tuned using ziegler nichols
#define VOLTAGE_MODE_KI_MANTISSA    6u             
        
//unused - current mode control code is not tested or prepared
#define TARGET_CURRENT_MA_1         0u         //target current in milliamps
#define TARGET_CURRENT_MA_2         0u         //option to change target current for step response using CL_Enable Jumper
#define CURRENT_MODE_CONTROL_PERIOD 79u            //79 corresponds to 100kHz
#define CURRENT_MODE_KP             10u
#define CURRENT_MODE_KP_MANTISSA    0u
#define CURRENT_MODE_KI             0u 
#define CURRENT_MODE_KI_MANTISSA    10u
    
//the output voltage is scaled according to rawValue = Vout * (100k/(100k+390k)) * 1024/5 
//to obtain milli volts from the raw value, mV = rawValue * 5/1024 * ((100k+390k)/100k) * 1000 = * 23.925 = (rawValue * 6100) >> 8, no offset required
#define VOLTAGE_SENSOR_GAIN         6100u
#define VOLTAGE_SENSOR_MANTISSA     8u
#define VOLTAGE_SENSOR_OFFSET       0u
    
#define SIZE_OF_VSENSOR_FILTER      16u      //must be a power of 2, also change VSENSOR_SHIFT accordingly
#define VSENSOR_SHIFT               4u       //squareroot(SIZE_OF_VSENSOR_FILTER) = 4
    
uint16_t filteredVout = 0;                  //filtered Vout measurements and FIFO
uint16_t voutFIFO[SIZE_OF_VSENSOR_FILTER];

typedef struct controllerVariables{         //template for control method variables       
    int16_t error;
    int32_t integral;
    int32_t proportionalOutput;
    int32_t integralOutput;
    int64_t integralOutputScaled;
    int32_t sumOutput;
    int16_t previousError;           
};

int64_t integratorScaledLimit = 0;          //variable for integrator limit scaled up, calculated in controller initialisation function

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

