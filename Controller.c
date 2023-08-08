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

#if CONTROL_METHOD == VOLTAGE_MODE_CONTROL
struct controllerVariables voltageModeVariables = {0, 0, 0, 0, 0, 0};
#endif

#if CONTROL_METHOD == CURRENT_MODE_CONTROL
struct controllerVariables currentModeVariables = {0, 0, 0, 0, 0, 0};
#endif

/*------------------------------------------------------------------------------
 Function: initialiseController()
 *Use: This function initialises the voltage sensor
------------------------------------------------------------------------------*/
void initialiseController(){
    initialiseGPIO(gpioOutputVoltage, GPIO_Input);
    initialiseADCPin(gpioOutputVoltage);
    integratorScaledLimit = (int64_t) ((int64_t) (INTEGRAL_LIMIT) << (VOLTAGE_MODE_KI_MANTISSA + DT_MANTISSA));
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
    for(uint8_t i=0; i<SIZE_OF_VSENSOR_FILTER; i++) sumOfSamples += voutFIFO[i];         //add all samples together
    
    return (sumOfSamples >> VSENSOR_SHIFT); //shift bits to divide by number of samples to get mean
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
    
    int16_t setDuty_unreg = 0;
    
    
    if(currentState == voltageModeControl){      //decides whether to run voltage mode control
#if CONTROL_METHOD == VOLTAGE_MODE_CONTROL
        runVoltageModeControl();
        setPeriod = VOLTAGE_MODE_CONTROL_PERIOD;
        //add 50% duty offset to the output of PID controller to allow positive and negative output 
        setDuty_unreg = (int16_t) (((uint32_t)(((uint16_t) PID_OFFSET) * setPeriod)) /  25)  + voltageModeVariables.sumOutput;
#endif
    }
    if(currentState == currentModeControl){     //decides whether to run current mode control
#if CONTROL_METHOD == CURRENT_MODE_CONTROL
        runCurrentModeControl();                    //NO CODE YET WRITTEN FOR CURRENT MODE
        setPeriod = CURRENT_MODE_CONTROL_PERIOD;
        //add 50% duty offset to the output of PID controller to allow positive and negative output 
        setDuty_unreg = (uint16_t) (((uint32_t)(((uint16_t) PID_OFFSET) * setPeriod)) /  25)  + currentModeVariables.sumOutput;
#endif
    }
    //limit duty cycle between specified min and max values. Divide by 25 as MAX_DUTY is in %, and 100% duty corresponds to 4*period
    uint16_t maxDuty = (uint16_t) (((uint32_t)(((uint16_t) MAX_DUTY) * setPeriod)) /  25);
    uint16_t minDuty = (uint16_t) (((uint32_t)(((uint16_t) MIN_DUTY) * setPeriod)) /  25);
    
    setDuty = setDuty_unreg;
    
    if(setDuty_unreg < 0) setDuty = minDuty;
    else if(setDuty_unreg >= 0){
        if(setDuty_unreg < minDuty) setDuty = minDuty;
        else if(setDuty_unreg > maxDuty) setDuty = maxDuty;
    }
   
}

/*------------------------------------------------------------------------------
 Function: runVoltageModeControl()
 *Use: This function runs the voltage mode control method and sets the variables
 * setDuty and setPeriod accordingly
------------------------------------------------------------------------------*/
void runVoltageModeControl(){
 
#if CONTROL_METHOD == VOLTAGE_MODE_CONTROL
    
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
   if(voltageModeVariables.integralOutputScaled > (integratorScaledLimit)){
       voltageModeVariables.integralOutputScaled = (integratorScaledLimit);
   }
   //anti windup for negative values
   if(voltageModeVariables.integralOutputScaled < 0){
        if(abs(voltageModeVariables.integralOutputScaled) > (integratorScaledLimit)){
                voltageModeVariables.integralOutputScaled = (int64_t) (0 -(integratorScaledLimit));
        }
   }
   
   //finally scale accordingly
   voltageModeVariables.integralOutput = voltageModeVariables.integralOutputScaled >> (DT_MANTISSA + VOLTAGE_MODE_KI_MANTISSA);
   
   //calculate proportional component, sum with integral to obtain output, record previous error (unused))
   int64_t propMult = (int32_t) (VOLTAGE_MODE_KP * ((int32_t) voltageModeVariables.error));
   voltageModeVariables.proportionalOutput = propMult >> VOLTAGE_MODE_KP_MANTISSA;
   
   voltageModeVariables.sumOutput = voltageModeVariables.integralOutput + voltageModeVariables.proportionalOutput;
   voltageModeVariables.previousError = voltageModeVariables.error;
   
#endif
}

/*------------------------------------------------------------------------------
 Function: runCurrentModeControl()
 *Use: This function runs the current mode control method and sets the variables
 * setDuty and setPeriod accordingly
------------------------------------------------------------------------------*/
void runCurrentModeControl(){
    
// NOTE - this controller has not been tested, or properly designed, the PI controller template from Voltage Mode has simply been copied across
    
#if CONTROL_METHOD == CURRENT_MODE_CONTROL
    int16_t newCurrent = convertRawToMilliAmps(filteredIL); 
   
   //calculate the latest error value, use the second target current value if jumper has been removed
   if(readGPIO(gpioControlSelect)) currentModeVariables.error = TARGET_CURRENT_MA_2 - newCurrent;
   else currentModeVariables.error = TARGET_CURRENT_MA_1 - newCurrent;
   
   //calculate integral component using gain and bit shift to do .dt multiplication, avoiding floating points
   int64_t integralMult = ((int64_t) (CURRENT_MODE_KI * ((int64_t) currentModeVariables.error) )) * DT_GAIN;
   //dont perform the bit shift until after calculating the cumulative value otherwise precision (fractional values) will be permanently lost
   currentModeVariables.integral = integralMult;
   currentModeVariables.integralOutputScaled = (currentModeVariables.integralOutputScaled + currentModeVariables.integral);
 
   //anti windup for integrator, limit integral component to reasonable values, using a limit which has been scaled up to match the pre-shifted value
   if(currentModeVariables.integralOutputScaled > (integratorScaledLimit)){
       currentModeVariables.integralOutputScaled = (integratorScaledLimit);
   }
   //anti windup for negative values
   if(currenteModeVariables.integralOutputScaled < 0){
        if(abs(currentModeVariables.integralOutputScaled) > (integratorScaledLimit)){
                currentModeVariables.integralOutputScaled = (int64_t) (0 -(integratorScaledLimit));
        }
   }
   
   //finally scale accordingly
   currentModeVariables.integralOutput = currentModeVariables.integralOutputScaled >> (DT_MANTISSA + CURRENT_MODE_KI_MANTISSA);
   
   //calculate proportional component, sum with integral to obtain output, record previous error (unused))
   int64_t propMult = (int32_t) (CURRENT_MODE_KP * ((int32_t) currentModeVariables.error));
   currentModeVariables.proportionalOutput = propMult >> CURRENT_MODE_KP_MANTISSA;
   
   currentModeVariables.sumOutput = currentModeVariables.integralOutput + currentModeVariables.proportionalOutput;
   currentModeVariables.previousError = currentModeVariables.error;
   
#endif
}
