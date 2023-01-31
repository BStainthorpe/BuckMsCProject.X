/* 
 * File:   StateMachine.c
 * Author: Ben Stainthorpe
 *
 * Created on 31 January 2023, 16:31
 */

#include "Global.h"
#include "StateMachine.h"
#include "GPIO.h"
#include "GPIO.h"
#include "ADC.h"

/*------------------------------------------------------------------------------
 Function: transToPotControl()
 *Use: This function sets the state to pot control and performs any setup
------------------------------------------------------------------------------*/
void transToPotControl(){
    currentState = potControl;
}

/*------------------------------------------------------------------------------
 Function: transToVoltageModeControl()
 *Use: This function sets the state to voltage mode control and performs any
 * setup
------------------------------------------------------------------------------*/
void transToVoltageModeControl(){
    currentState = voltageModeControl;
}

/*------------------------------------------------------------------------------
 Function: transToCurrentModeControl()
 *Use: This function sets the state to current mode control and performs any
 * setup
------------------------------------------------------------------------------*/
void transToCurrentModeControl(){
    currentState = currentModeControl;
}

/*------------------------------------------------------------------------------
 Function: transToOverCurrentFault()
 *Use: This function sets the state to over current fault and clears the PWM
 * outputs
------------------------------------------------------------------------------*/
void transToOverCurrentFault(){
    setDuty = 0;    //turn off PWM
    setPeriod = 0;    
    currentState = overCurrentFault;
}