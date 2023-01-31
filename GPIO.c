/* 
 * File:   GPIO.c
 * Author: Ben Stainthorpe
 *
 * Created on 20 January 2023, 16:11
 */

#include "Global.h"
#include "GPIO.h"
#include <stdbool.h>


/*------------------------------------------------------------------------------
 Function: initialiseGPIO(gpioNumber, direction)
 *Use: This function initialises the registers as required for the specified
 * GPIO pin to be an input or output
------------------------------------------------------------------------------*/
void initialiseGPIO(const enum GPIO_PORTS gpioNumber, uint8_t direction){
    uint8_t portNumber = 0;
    uint8_t portType = 0;
    if(gpioNumber > 7){      //According to the GPIO enum in GPIO.h, first 8 are PORT A, last 8 are PORT B
        portNumber = gpioNumber - 8;
        portType = GPIO_PORTB;      
    }
    else{
        portNumber = gpioNumber;
        portType = GPIO_PORTA;
    }
         
    if(portType == GPIO_PORTA){
        if(direction){
            TRISA |= (1 << (uint8_t)portNumber);            //if direction=1, set bit in TRISA to operate as input
            ANSELA &= ~(1 << (uint8_t)portNumber);          //we also need to clear the bit in the ANSEL (analog) register as we want digital IO
        }
        else if(~direction) TRISA &= ~(1 << (uint8_t)portNumber);       //if direction =0, set bit in TRISA as 0 to operate as output
    }
    else if(portType == GPIO_PORTB){                        //perform the same for PORTB
        if(direction){
            TRISB |= (1 << (uint8_t)portNumber); 
            ANSELB &= ~(1 << (uint8_t)portNumber);
        }
        else if(~direction) TRISB &= ~(1 << (uint8_t)portNumber);
    }
   
}

/*------------------------------------------------------------------------------
 Function: writeGPIO(gpioNumber, writeValue)
 *Use: This function writes the passed binary value (zero = 0, non zero = 1)
 * to the requested GPIO pin
------------------------------------------------------------------------------*/
void writeGPIO(const enum GPIO_PORTS gpioNumber, uint8_t writeValue){
    uint8_t portNumber = 0;     //the number of port gpio, ie RA1 = 1
    uint8_t portType = 0;       //the port type, ie A or B
    if(gpioNumber > 7){
        portNumber = gpioNumber - 8;
        portType = GPIO_PORTB;
    }
    else{
        portNumber = gpioNumber;
        portType = GPIO_PORTA;
    }
         
    if(portType == GPIO_PORTA){     //clear or write the bit in LATA to write output as desired
        if(writeValue) LATA |= (1 << (uint8_t)portNumber);                     
        else if(~writeValue) LATA &= ~(1 << (uint8_t)portNumber);
    }
    else if(portType == GPIO_PORTB){
        if(writeValue) LATB |= (1 << (uint8_t)portNumber); 
        else if(~writeValue) LATB &= ~(1 << (uint8_t)portNumber);
    }
   
}

/*------------------------------------------------------------------------------
 Function: readGPIO(gpioNumber)
 *Use: This function reads the value on the requested GPIO pin
------------------------------------------------------------------------------*/
bool readGPIO(const enum GPIO_PORTS gpioNumber){
    uint8_t portNumber = 0;     //the number of port gpio, ie RA1 = 1
    uint8_t portType = 0;       //the port type, ie A or B
    if(gpioNumber > 7){
        portNumber = gpioNumber - 8;
        portType = GPIO_PORTB;
    }
    else{
        portNumber = gpioNumber;
        portType = GPIO_PORTA;
    }
         
    if(portType == GPIO_PORTA){         //obtain the state of the digital input from PORTA and return from function
        uint8_t readA = PORTA;
        uint8_t returnValueA = ((readA >> ((uint8_t) portNumber)) & 1u); 
        return returnValueA;  
    }
    else if(portType == GPIO_PORTB){
        uint8_t readB = PORTB;
        uint8_t returnValueB = ((readB >> ((uint8_t) portNumber)) & 1u); 
        return returnValueB;
    }
   
}