/* 
 * File:   CurrentSensor.c
 * Author: Ben Stainthorpe
 *
 * Created on 20 January 2023, 16:11
 */
#include "Global.h"
#include "CurrentSensor.h"
#include "GPIO.h"
#include "GPIO.h"
#include "ADC.h"

/*------------------------------------------------------------------------------
 Function: initialiseCurrentSensors()
 *Use: This function initialises the pins required for the current sensors
------------------------------------------------------------------------------*/
void initialiseCurrentSensors(){
    initialiseGPIO(gpioCurrentTripIL, GPIO_Input);
    initialiseGPIO(gpioCurrentTripIDS, GPIO_Input);
    initialiseADCPin(gpioIDSCurrent);
    initialiseADCPin(gpioILCurrent);
    initialiseGPIO(gpioOverCurrentClear, GPIO_Output);
    currentTripReset();        //initially set to 0 to turn off MOSFET and clear overcurrent faults 
}

/*------------------------------------------------------------------------------
 Function: currentTripRead()
 *Use: This function reads the current trip digital IO to detect overcurrent
------------------------------------------------------------------------------*/
bool currentTripRead(){
    tripIDS = ~readGPIO(gpioCurrentTripIDS); //flags as 1 if there has been an IDS trip
    tripIL = ~readGPIO(gpioCurrentTripIL);   //flags as 1 if there has been an IL trip
    return (tripIL || tripIDS);    //if either pin drops to 0, giving a flag of 1, a fault has occurred, return 1
}

/*------------------------------------------------------------------------------
 Function: readFilteredIDS()
 *Use: This function obtains a new ADC sample and performs the moving average 
 * FIFO filter for the IDS sensor, returning the filtered value
------------------------------------------------------------------------------*/
uint16_t readFilteredIDS(){
    for(uint8_t i=0; i<SIZE_OF_ISENSOR_FILTER-1; i++) currentIDSFIFO[i] = currentIDSFIFO[i+1];       //shift all values in array to next 
    currentIDSFIFO[SIZE_OF_ISENSOR_FILTER-1] =  readADCRaw(gpioIDSCurrent);                //take the newest sample
    uint32_t sumOfSamples = 0;
    for(uint8_t i=0; i<SIZE_OF_ISENSOR_FILTER; i++) sumOfSamples += currentIDSFIFO[i];
    
    return (sumOfSamples >> 4); //shift by 4 bits to divide by 16
}

/*------------------------------------------------------------------------------
 Function: readFilteredIL()
 *Use: This function obtains a new ADC sample and performs the moving average 
 * FIFO filter for the IL sensor, returning the filtered value
------------------------------------------------------------------------------*/
uint16_t readFilteredIL(){
    for(uint8_t i=0; i<SIZE_OF_ISENSOR_FILTER-1; i++) currentILFIFO[i] = currentILFIFO[i+1];       //shift all values in array to next 
    currentILFIFO[SIZE_OF_ISENSOR_FILTER-1] =  latestIL;                //take the newest sample from interrupt
    uint32_t sumOfSamples = 0;
    for(uint8_t i=0; i<SIZE_OF_ISENSOR_FILTER; i++) sumOfSamples += currentILFIFO[i];
    
    return (sumOfSamples >> 4); //shift by 4 bits to divide by 16
}

/*------------------------------------------------------------------------------
 Function: currentTripReset()
 *Use: This function resets the current trip by holding the reset pin low for 
 * 20uS, which turns off the MOSFETs in series with the Current Sensor reset pins
 * before turning them back on again
------------------------------------------------------------------------------*/
void currentTripReset(){
    writeGPIO(gpioOverCurrentClear, 0);
     __delay_us(20);
    writeGPIO(gpioOverCurrentClear, 1); 
}

/*------------------------------------------------------------------------------
 Function: convertRawToMilliAmps(rawValue)
 *Use: This function converts a raw current ADC value to milli Amps according
 * to the current sensor gain function, note the output is signed
------------------------------------------------------------------------------*/
int16_t convertRawToMilliAmps(uint16_t rawValue){
    int16_t offsetted = (int16_t)(rawValue - CURRENT_SENSOR_OFFSET); //subtract the offset to obtain a neg or pos value
    int16_t returnValuemA = (int32_t)(offsetted * CURRENT_SENSOR_GAIN) >> CURRENT_SENSOR_MANTISSA;
    return returnValuemA;
}