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
#include "PWM.h"

struct controllerVariables voltageModeVariables = {0, 0, 0, 0, 0, 0};
struct controllerVariables currentModeVariables = {0, 0, 0, 0, 0, 0};

/*------------------------------------------------------------------------------
 Function: initialiseController()
 *Use: This function initialises the voltage sensor
------------------------------------------------------------------------------*/
void initialiseController(){
    initialiseGPIO(gpioOutputVoltage, GPIO_Input);
    initialiseADCPin(gpioOutputVoltage);
}

/*------------------------------------------------------------------------------
 Function: readFilteredVout()
 *Use: This function obtains a new ADC sample and performs the moving average 
 * FIFO filter for the Vout sensor, returning the filtered value
------------------------------------------------------------------------------*/
uint16_t readFilteredVout(){
    for(uint8_t i=0; i<SIZE_OF_VSENSOR_FILTER-1; i++) voutFIFO[i] = voutFIFO[i+1];       //shift all values in array to next 
    voutFIFO[SIZE_OF_VSENSOR_FILTER-1] =  readADCRaw(gpioOutputVoltage);                 //take the newest sample
    uint32_t sumOfSamples = 0;
            for(uint8_t i=0; i<SIZE_OF_VSENSOR_FILTER; i++) sumOfSamples += voutFIFO[i];
    
    return (sumOfSamples >> 4); //shift by 4 bits to divide by 16
}

/*------------------------------------------------------------------------------
 Function: convertRawToMilliVolts(rawValue)
 *Use: This function converts a raw voltage ADC value to Milli volts according
 * to the potential divider on the output voltage
------------------------------------------------------------------------------*/
int16_t convertRawToMilliVolts(uint16_t rawValue){
    int16_t offsetted = (int16_t)(rawValue) - VOLTAGE_SENSOR_OFFSET; //subtract the offset obtained from calibration
    int32_t vsenseMult = ((int32_t)(((int32_t) offsetted) * VOLTAGE_SENSOR_GAIN));
    int16_t returnValuedV = (int16_t) (vsenseMult >> VOLTAGE_SENSOR_MANTISSA);
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
        setPeriod = VOLTAGE_MODE_CONTROL_PERIOD;
        //add 50% duty offset to the output of PID controller to allow positive and negative output 
        setDuty = (uint16_t) (((uint32_t)(((uint16_t) PID_OFFSET) * setPeriod)) /  25)  + voltageModeVariables.sumOutput;
    }
    if(currentState == currentModeControl){     //decides whether to run current mode control
        runCurrentModeControl();                    //NO CODE YET WRITTEN FOR CURRENT MODE
        setPeriod = CURRENT_MODE_CONTROL_PERIOD;
        //add 50% duty offset to the output of PID controller to allow positive and negative output 
        setDuty = (uint16_t) (((uint32_t)(((uint16_t) PID_OFFSET) * setPeriod)) /  25)  + currentModeVariables.sumOutput;
    }
    //limit duty cycle between specified min and max values. Divide by 25 as MAX_DUTY is in %, and 100% duty corresponds to 4*period
    uint16_t maxDuty = (uint16_t) (((uint32_t)(((uint16_t) MAX_DUTY) * setPeriod)) /  25);
    uint16_t minDuty = (uint16_t) (((uint32_t)(((uint16_t) MIN_DUTY) * setPeriod)) /  25);
    if(setDuty > maxDuty) setDuty = maxDuty;
    if(setDuty < minDuty) setDuty = minDuty;
    if(setDuty < 0) setDuty = minDuty;
   
}

/*------------------------------------------------------------------------------
 Function: runVoltageModeControl()
 *Use: This function runs the voltage mode control method and sets the variables
 * setDuty and setPeriod accordingly
------------------------------------------------------------------------------*/
void runVoltageModeControl(){
    
   //Obtain latest voltage reading in millivolts
   uint16_t newVoltage = convertRawToMilliVolts(filteredVout);
   
   //calculate the latest error value, use the second target voltage value if jumper has been removed
   if(readGPIO(gpioControlSelect)) voltageModeVariables.error = TARGET_VOLTAGE_MV_2 - newVoltage;
   else voltageModeVariables.error = TARGET_VOLTAGE_MV_1 - newVoltage;
   
   //calculate integral component using gain and bit shift to do .dt multiplication, avoiding floating points
   int64_t integralMult = ((int64_t) (VOLTAGE_MODE_KI * ((int64_t) voltageModeVariables.error) )) * DT_GAIN;
   //dont perform the bit shift until after calculating the cumulative value otherwise precision (fractional values) will be permanently lost
   voltageModeVariables.integral = integralMult;
   voltageModeVariables.integralOutputScaled = (voltageModeVariables.integralOutputScaled + voltageModeVariables.integral);
 
   //anti windup for integrator, limit integral component to reasonable values, using a limit which has been scaled up to match the pre-shifted value
   if(voltageModeVariables.integralOutputScaled > (INTEGRAL_SCALED_LIMIT)){
       voltageModeVariables.integralOutputScaled = (INTEGRAL_SCALED_LIMIT);
   }
   //anti windup for negative values
   if(voltageModeVariables.integralOutputScaled < 0){
        if(abs(voltageModeVariables.integralOutputScaled) > (INTEGRAL_SCALED_LIMIT)){
                voltageModeVariables.integralOutputScaled = (int64_t) (0 -(INTEGRAL_SCALED_LIMIT));
        }
   }
   
   //finally scale accordingly
   voltageModeVariables.integralOutput = voltageModeVariables.integralOutputScaled >> (DT_MANTISSA + VOLTAGE_MODE_KI_MANTISSA);
   
   //calculate proportional component, sum with integral to obtain output, record previous error (unused))
   int64_t propMult = (int32_t) (VOLTAGE_MODE_KP * ((int32_t) voltageModeVariables.error));
   voltageModeVariables.proportionalOutput = propMult >> VOLTAGE_MODE_KP_MANTISSA;
   
   voltageModeVariables.sumOutput = voltageModeVariables.integralOutput + voltageModeVariables.proportionalOutput;
   voltageModeVariables.previousError = voltageModeVariables.error;
}

/*------------------------------------------------------------------------------
 Function: runCurrentModeControl()
 *Use: This function runs the current mode control method and sets the variables
 * setDuty and setPeriod accordingly
------------------------------------------------------------------------------*/
void runCurrentModeControl(){
    int16_t newCurrent = convertRawToMilliAmps(filteredIL);
}
