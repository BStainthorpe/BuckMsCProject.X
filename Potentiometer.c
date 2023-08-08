/* 
 * File:   Potentiometer.c
 * Author: Ben Stainthorpe
 *
 * Created on 20 January 2023, 16:11
 */

#include "Potentiometer.h"
#include "PWM.h"

/*------------------------------------------------------------------------------
 Function: initialisePotentiometers()
 *Use: This function sets up the required ADC gpio pins for the potentiometers
------------------------------------------------------------------------------*/
void initialisePotentiometers(){
    initialiseADCPin(gpioPotentiometerDuty);
    initialiseADCPin(gpioPotentiometerFreq);
}

/*------------------------------------------------------------------------------
 Function: readFilteredDutyPot()
 *Use: This function obtains a new ADC sample and performs the moving average 
 * FIFO filter for the duty cycle control pot, returning the filtered value
------------------------------------------------------------------------------*/
uint16_t readFilteredDutyPot(){
    for(uint8_t i=0; i<SIZE_OF_POT_FILTER-1; i++) dutyPotFIFO[i] = dutyPotFIFO[i+1];       //shift all values in array to next 
    dutyPotFIFO[SIZE_OF_POT_FILTER-1] =  readADCRaw(gpioPotentiometerDuty);                //take the newest sample
    uint32_t sumOfSamples = 0;
    for(uint8_t i=0; i<SIZE_OF_POT_FILTER; i++) sumOfSamples += dutyPotFIFO[i];    //sum all samples
    
    return (sumOfSamples >> POT_SENSOR_SHIFT); //shift bits to divide by number of samples to get mean
}

/*------------------------------------------------------------------------------
 Function: readFilteredFreqPot()
 *Use: This function obtains a new ADC sample and performs the moving average 
 * FIFO filter for the frequency control pot, returning the filtered value
------------------------------------------------------------------------------*/
uint16_t readFilteredFreqPot(){
    for(uint8_t i=0; i<SIZE_OF_POT_FILTER-1; i++) freqPotFIFO[i] = freqPotFIFO[i+1];       //shift all values in array to next 
    freqPotFIFO[SIZE_OF_POT_FILTER-1] =  readADCRaw(gpioPotentiometerFreq);                //take the newest sample
    uint32_t sumOfSamples = 0;
    for(uint8_t i=0; i<SIZE_OF_POT_FILTER; i++) sumOfSamples += freqPotFIFO[i];    //sum all samples
    
    return (sumOfSamples >> POT_SENSOR_SHIFT); //shift bits to divide by number of samples to get mean
}

/*------------------------------------------------------------------------------
 Function: runPotScaling()
 *Use: This function uses the moving average pot values and performs the ADC
 * scaling and necessary gain and offsets, before setting the new period
 * and duty cycle. The POT_SET_DIVIDER define is used to reduce the frequency
 * of execution of this function
------------------------------------------------------------------------------*/
void runPotScaling(){
    
    if(currentState == potControl){
        potSetCount++;
        //use potSetCount and POT_SET_DIVIDER to reduce the frequency of pot reads to: Slot 4 Freq / POT_SET_DIVIDER, this reduces jitter in signal
        if(potSetCount == POT_SET_DIVIDER){
            //to calculate the required period we scale according to the min and max periods, shift by 10 bits to perform ADC scaling (1024 max ADC value) 
            setPeriod = ((uint32_t)((filteredFreqPot - POT_OFFSET) * (uint32_t)(MAX_PERIOD_FROM_POT - MIN_PERIOD_FROM_POT)) >> 10 ) + MIN_PERIOD_FROM_POT;
            //100% duty equates to a duty value of 4*setPeriod, therefore only shift by 8 bits to perform ADC scaling (>>10) and also multiply by 4
            setDuty =  (uint32_t)((uint32_t)((filteredDutyPot-POT_OFFSET) * (uint32_t)setPeriod )) >> 8;

            //limit duty cycle between specified min and max values. Divide by 25 as MAX_DUTY is in %, and 100% duty corresponds to 4*period
            uint16_t maxDuty = (uint16_t) (((uint32_t)(((uint16_t) MAX_DUTY) * setPeriod)) /  25);
            uint16_t minDuty = (uint16_t) (((uint32_t)(((uint16_t) MIN_DUTY) * setPeriod)) /  25);
            if(setDuty > maxDuty) setDuty = maxDuty;
            if(setDuty < minDuty) setDuty = minDuty;

            potSetCount = 0;        //reset, begin counting again for next pot calculation
        }
    }  
}
