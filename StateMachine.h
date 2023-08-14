/* 
 * File:   StateMachine.h
 * Author: Ben Stainthorpe
 *
 * Created on 31 January 2023, 16:31
 */

#ifndef STATEMACHINE_H
#define	STATEMACHINE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "GPIO.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>   
#include "Global.h"  
#include "PWM.h""

//the list of states in state machine
enum stateMachine{
    initialising,
    potControl,
    voltageModeControl,
    currentModeControl,
    overCurrentFault
};

enum stateMachine currentState = 0;  //initialising by default

void transToInitialising();
void transToPotControl();
void transToVoltageModeControl();
void transToCurrentModeControl();
void transToOverCurrentFault();

#ifdef	__cplusplus
}
#endif

#endif	/* STATEMACHINE_H */

