/* 
 * File:   GPIO.h
 * Author: Ben Stainthorpe
 *
 * Created on 20 January 2023, 16:11
 */

#ifndef GPIO_H
#define	GPIO_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
    
#include <xc.h>                                     //PIC hardware mapping

//defines for code readability
#define GPIO_Output 0 
#define GPIO_Input  1
  
    
#define GPIO_PORTA 0
#define GPIO_PORTB 1
    
//enum containing all GPIO ports - subtract 7 from GPIO Bs to get PORT B number
enum GPIO_PORTS{
    pinRA0,
    pinRA1,
    pinRA2,
    pinRA3,
    pinRA4,
    pinRA5,
    pinRA6,
    pinRA7,
    pinRB0,
    pinRB1,
    pinRB2,
    pinRB3,
    pinRB4,
    pinRB5,
    pinRB6,
    pinRB7
};

void initialiseGPIO(const enum GPIO_PORTS gpioNumber, uint8_t direction);
void writeGPIO(const enum GPIO_PORTS gpioNumber, uint8_t writeValue);
bool readGPIO(const enum GPIO_PORTS gpioNumber);

#ifdef	__cplusplus
}
#endif

#endif	/* GPIO_H */

