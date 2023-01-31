/* 
 * File:   Controller.c
 * Author: Ben Stainthorpe
 *
 * Created on 20 January 2023, 16:11
 */
#include "Global.h"
#include "Controller.h"
#include "GPIO.h"
#include "GPIO.h"
#include "ADC.h"
#include "CurrentSensor.h"
#include "StateMachine.h"

/*------------------------------------------------------------------------------
 Function: readFilteredVout()
 *Use: This function obtains a new ADC sample and performs the moving average 
 * FIFO filter for the Vout sensor, returning the filtered value
------------------------------------------------------------------------------*/
uint16_t readFilteredVout(){
    for(uint8_t i=0; i<SIZE_OF_VSENSOR_FILTER-1; i++) voutFIFO[i] = voutFIFO[i+1];       //shift all values in array to next 
    voutFIFO[SIZE_OF_VSENSOR_FILTER-1] =  readADCRaw(gpioOutputVoltage);                //take the newest sample
    uint32_t sumOfSamples = 0;
    for(uint8_t i=0; i<SIZE_OF_VSENSOR_FILTER; i++) sumOfSamples += voutFIFO[i];
    
    return (sumOfSamples >> 4); //shift by 4 bits to divide by 16
}

/*------------------------------------------------------------------------------
 Function: convertRawToDeciVolts(rawValue)
 *Use: This function converts a raw voltage ADC value to deci volts according
 * to the potential divider on the output voltage
------------------------------------------------------------------------------*/
int16_t convertRawToDeciVolts(uint16_t rawValue){
    int16_t offsetted = (int16_t)(rawValue - VOLTAGE_SENSOR_OFFSET); //subtract the offset to obtain a neg or pos value
    int16_t returnValuedV = (int32_t)(offsetted * VOLTAGE_SENSOR_GAIN) >> VOLTAGE_SENSOR_MANTISSA;
    return returnValuedV;
}

/*------------------------------------------------------------------------------
 Function: controlRoutine()
 *Use: This function checks the state machine and runs voltage or current mode
 * control if in the correct state
------------------------------------------------------------------------------*/
void controlRoutine(){
    
    if(currentState == voltageModeControl){      //decides whether to run voltage mode control
        runVoltageModeControl();
    }
    if(currentState == currentModeControl){     //decides whether to run current mode control
        runCurrentModeControl();
    }
}

/*------------------------------------------------------------------------------
 Function: runVoltageModeControl()
 *Use: This function runs the voltage mode control method and sets the variables
 * setDuty and setPeriod accordingly
------------------------------------------------------------------------------*/
void runVoltageModeControl(){
    int16_t newVoltage = convertRawToDeciVolts(filteredVout);
}

/*------------------------------------------------------------------------------
 Function: runCurrentModeControl()
 *Use: This function runs the current mode control method and sets the variables
 * setDuty and setPeriod accordingly
------------------------------------------------------------------------------*/
void runCurrentModeControl(){
    int16_t newCurrent = convertRawToMilliAmps(filteredIL);
}