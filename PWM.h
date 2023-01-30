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
void setupPWM();
void setPWMDutyandPeriod(uint16_t dutyCycle, uint8_t period);
void setPWMPeriod(uint8_t period);
        
#ifdef	__cplusplus
}
#endif

#endif	/* PWM_H */

