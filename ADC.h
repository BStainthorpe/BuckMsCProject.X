/* 
 * File:   ADC.h
 * Author: Ben Stainthorpe
 *
 * Created on 23 January 2023, 15:32
 */

#ifndef ADC_H
#define	ADC_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
    
#include <xc.h>                                     //PIC hardware mapping
#include "GPIO.h"
    
#define DEFAULT_ADC 0b010     //the ADC read select is set to RA2 by default to reduce speed of reads
#define MAX_ADC_VALUE 1023u

void initialiseADCPin(const enum GPIO_PORTS gpioNumber);
void initialiseADCModule();
uint16_t readADCRaw(const enum GPIO_PORTS gpioNumber);
uint16_t readILCurrentADCRaw();

#ifdef	__cplusplus
}
#endif

#endif	/* ADC_H */

