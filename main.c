/* 
 * File:   main.c
 * Author: Ben Stainthorpe
 *
 * Created on 10 January 2023, 15:17
 */

//config bits that are part-specific for the PIC16F1829
#pragma config FOSC=INTOSC, WDTE=OFF, PWRTE=OFF, MCLRE=OFF, CP=OFF, CPD=OFF, BOREN=ON, CLKOUTEN=OFF, IESO=OFF, FCMEN=OFF
#pragma config WRT=OFF, PLLEN=OFF, STVREN=OFF, LVP=OFF

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <xc.h>                                     //PIC hardware mapping
#include "PWM.h"
#include "Timer0.h"
#include "Global.h"
#include "GPIO.h"
#include "CurrentSensor.h"
#include "Controller.h"
#include "ADC.h"
#include "Potentiometer.h"
#include "StateMachine.h"

volatile bool timerSlotHalf = 0;
volatile bool timerSlotQuarter = 0;

void setupInternalOscillator(const enum internalClockFreqSelec selectedFreq);

/*------------------------------------------------------------------------------
 Function: Tick980Hz()
 *Use: This function is executed when any of the global
 * interrupts are triggered. The cause of the interrupt can be determined using
 * if statements.
 * TM0IF_bit: this corresponds to a timer0 interrupt
 * 980Hz frequency is dependent on clock being 32MHz
------------------------------------------------------------------------------*/
void __interrupt() Tick980Hz(void){      //This function is called on each interrupt, 980Hz frequency is dependent on clock being 32MHz
    
    if (TMR0IF_bit) {   //Check if Timer0 has caused the interrupt. Timer 0 interrupt operates at 980Hz or every 1ms
       
    //Timer Interrupt Slots:     Timing Graph:                        Functions:
    //980Hz interrupt            |------|------|------|------|        currentTripRead() setPWMDutyandPeriod()
    //490Hz Slot 1:              1-------------1-------------1        controlRoutine()
    //490Hz Slot 2:              -------2-------------2-------        readFilteredVout() readFilteredIL()
    //245Hz Slot 3:              -------3---------------------        runPotScaling()
    //245Hz Slot 4:              ---------------------4-------        readFilteredDutyPot() readFilteredFreqPot()
       
        if(currentTripRead() == 1){
            transToOverCurrentFault();
        }
        setPWMDutyandPeriod(setDuty, setPeriod);
        
       //each half slot occurs at 490Hz or every 2ms
        if(timerSlotHalf == false){
            //slot 1------------------------------------------------------------
            writeGPIO(gpioSlotTest, 0); //write in Slot 3 and Clear in Slot 1 gives a 25% duty PWM at 245Hz on RB4
            controlRoutine();
        }
        if(timerSlotHalf == true){
            //slot 2------------------------------------------------------------
            filteredIL = readFilteredIL();
            //filteredIDS = readFilteredIDS();     
            filteredVout = readFilteredVout();
            
            //each quarter slot occurs at 245Hz or every 4ms
            if(timerSlotQuarter == false){
                //slot 3--------------------------------------------------------
                writeGPIO(gpioSlotTest, 1); //write in Slot 3 and Clear in Slot 1 gives a 25% duty PWM at 245Hz on RB4
                runPotScaling();
            }
            
            if(timerSlotQuarter == true){
                //slot 4--------------------------------------------------------
                filteredDutyPot = readFilteredDutyPot();
                filteredFreqPot = readFilteredFreqPot();               
            }           
          
            timerSlotQuarter = ~timerSlotQuarter;
        }

        timerSlotHalf = ~timerSlotHalf;
        INTCONbits.TMR0IF = 0;         // clear interrupt flag

    }
    
    if(CCP1IF_bit){
        latestIL = readILCurrentADCRaw();   //fast function for reading the IL current
        PIR1bits.CCP1IF = 0;               //clear the interrupt flag
    }

}

/*------------------------------------------------------------------------------
 Function: main()
 *Use: The main application entry point, performs the initialisation functions
 * and then enters an infinite while loop, as main control functions are 
 * executed in the interrupt function
------------------------------------------------------------------------------*/
int main(int argc, char** argv) {
    
    setupInternalOscillator(CLOCK_FREQUENCY_SELECT);
    
    setupPWM();
    setupTimer0Interrupt();  
    initialiseADCModule();
    initialiseCurrentSensors();
    initialisePotentiometers();
    initialiseGPIO(gpioSlotTest, GPIO_Output);
    if(~readGPIO(gpioControlSelect)){                        //read pin which selects closed loop or open loop pot controlled
        if(CONTROL_METHOD)  transToVoltageModeControl();
        if(~CONTROL_METHOD)  transToCurrentModeControl(); //option here to hard code voltage mode or current mode 
    }
    else transToPotControl();

    while(1){           //infinite loop to hold uC in operation
        
    }
    return (EXIT_SUCCESS);
}

/*------------------------------------------------------------------------------
 Function: setupInternalOscillator(selectedFreq)
 *Use: This function sets up the prescaler registers to achieve the specified
 * frequency passed
------------------------------------------------------------------------------*/
void setupInternalOscillator(const enum internalClockFreqSelec selectedFreq){
    //IRCF bits contains frequency selection
    //SPLLEN bit enables 4x multiplier to achieve 32MHz
    //for this application we need 32MHz to achieve acceptable PWM up to 500kHz
    switch(selectedFreq){                
        case freq31k:  OSCCONbits.IRCF = 0b0000; OSCCONbits.SPLLEN = 0; clockFrequency = 31000;    break;
        case freq62k5: OSCCONbits.IRCF = 0b0100; OSCCONbits.SPLLEN = 0; clockFrequency = 62500;    break;
        case freq125k: OSCCONbits.IRCF = 0b0101; OSCCONbits.SPLLEN = 0; clockFrequency = 125000;   break;
        case freq250k: OSCCONbits.IRCF = 0b0110; OSCCONbits.SPLLEN = 0; clockFrequency = 250000;   break;
        case freq500k: OSCCONbits.IRCF = 0b0111; OSCCONbits.SPLLEN = 0; clockFrequency = 500000;   break;
        case freq1M:   OSCCONbits.IRCF = 0b1011; OSCCONbits.SPLLEN = 0; clockFrequency = 1000000;  break;
        case freq2M:   OSCCONbits.IRCF = 0b1100; OSCCONbits.SPLLEN = 0; clockFrequency = 2000000;  break;
        case freq4M:   OSCCONbits.IRCF = 0b1101; OSCCONbits.SPLLEN = 0; clockFrequency = 4000000;  break;
        case freq8M:   OSCCONbits.IRCF = 0b1110; OSCCONbits.SPLLEN = 0; clockFrequency = 8000000;  break;
        case freq16M:  OSCCONbits.IRCF = 0b1111; OSCCONbits.SPLLEN = 0; clockFrequency = 16000000; break;
        case freq32M:  OSCCONbits.IRCF = 0b1110; OSCCONbits.SPLLEN = 1; clockFrequency = 32000000; break;
    }
}

