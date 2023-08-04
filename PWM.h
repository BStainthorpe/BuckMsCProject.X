/* 
 * File:   PWM.h
 * Author: Ben Stainthorpe
 *
 * Created on 10 January 2023, 15:33
 */

#ifndef PWM_H
#define	PWM_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <xc.h>                                     //PIC hardware mapping
     
#define MIN_DUTY               10
#define MAX_DUTY               90    
    
//variables for setting duty and period
uint8_t setPeriod = 0;
uint16_t setDuty = 0;
uint8_t prevPeriod = 0; 
uint16_t prevDuty = 0;

void setupPWM();
void setPWMDutyandPeriod(uint16_t dutyCycle, uint8_t period);
void setPWMPeriod(uint8_t period);


        
#ifdef	__cplusplus
}
#endif

#endif	/* PWM_H */

