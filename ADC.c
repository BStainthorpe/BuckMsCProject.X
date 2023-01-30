/* 
 * File:   ADC.c
 * Author: Ben Stainthorpe
 *
 * Created on 20 January 2023, 16:11
 */
#include "ADC.h"
#include "Global.h"

/*------------------------------------------------------------------------------
 Function: initialiseADCModule()
 *Use: This function sets up the module level registers for the ADC
------------------------------------------------------------------------------*/
void initialiseADCModule(){
    
    ADCON0 = 0b00000001; //ADC ON and Fosc/16 is selected
    //bit 7: unused
    //bit 6-2: channel select for reads
    //bit 1: A/D conversion status, 1 starts a conversion cycle
    //bit 0: 1, ADC Enabled
    
    ADCON1 = 0b10000000; // 
    //bit 7: 1, six MSB in ADRESH are 0
    //bit 6-4: 000 Conversion freq,Fosc/2
    //bit 3: unused
    //bit 2: 0, Vref- is Vss
    //bit 1-0: 00, Vref+ is Vdd
    
    
    
}

/*------------------------------------------------------------------------------
 Function: initialiseADCPin(gpioNumber)
 *Use: This function sets up the registers required for the gpio pin specified
------------------------------------------------------------------------------*/
void initialiseADCPin(const enum GPIO_PORTS gpioNumber){
        
    uint8_t portNumber = 0;     //the number of port gpio, ie RA1 = 1
    uint8_t portType = 0;       //the port type, ie A or B
    uint8_t gpioValid = 0;      //flag gpio as having valid ADC - prevent trying to set a pin which doesnt have an ADC
    
    if(gpioNumber < 4){                //RA0 to RA4
        portNumber = gpioNumber;           
        portType = GPIO_PORTA;
        gpioValid = 1;      
    }
    
    if((gpioNumber > 8u) && (gpioNumber <= 15u)){    //starting at RB1 (9), until RB7 (15).
        portNumber = gpioNumber - 8;                 //According to the GPIO enum in GPIO.h, first 8 are PORT A, last 8 are PORT B
        portType = GPIO_PORTB;    
        gpioValid = 1;
    }

    if(gpioValid){
        if(portType == GPIO_PORTA){
            TRISA |= (1 << (uint8_t)portNumber);            //set bit in TRISA to operate as input
            ANSELA |= (1 << (uint8_t)portNumber);           //we also need to set the bit in the ANSEL (analog) register as we want analog Input
        }
        else if(portType == GPIO_PORTB){                    //perform the same for PORTB
            TRISB |= (1 << (uint8_t)portNumber); 
            ANSELB |= (1 << (uint8_t)portNumber);
        } 
    }
}

/*------------------------------------------------------------------------------
 Function: readADCRaw(gpioNumber)
 *Use: This function reads the raw ADC value from the specified gpio pin
------------------------------------------------------------------------------*/
uint16_t readADCRaw(const enum GPIO_PORTS gpioNumber){
    uint8_t channel = 0;        //the ADC channel numbering, see datasheet
    uint8_t gpioValid = 0;      //flag gpio as having valid ADC - prevent trying to set a pin which doesnt have an ADC    
    
    if(gpioNumber < 4){
        channel = gpioNumber;  //RA0 to RA4 directly converts to ADC numbering
        gpioValid = 1;         //flag gpio as having valid ADC
    }
    if((gpioNumber > 8u) && (gpioNumber <= 15u)){  //starting at RB1 (9), until RB7 (15)
        channel = 12u - (gpioNumber - 8);          //convert the enum to standard port B numbering, then convert to ADC numbering (see datasheet)
        gpioValid = 1;                             //flag gpio as having valid ADC 
    }
    if(gpioValid){  
        //Only attempt to read if there is an ADC on the requested GPIO
        if(~ADCON0bits.GO_nDONE){                                //ensure we are not interrupting another read
            ADCON0 &= ~(0b01111100);                             //Clear ADC Channel Select
            ADCON0 |= (channel << 2);                            //Set to desired channel
            for(uint8_t i = 0; i < 8; i++);                      //insert a small time delay using a for loop to allow channel change
            
            ADCON0bits.GO_nDONE = 1;                             //Set the Conversion begin bit
            while(ADCON0bits.GO_nDONE);                          //Wait until the conversion finishes
            unsigned int returnValue = ((ADRESH<<8)+ADRESL);
            ADCON0 &= ~(0b01111100);                             //Clear channel select
            ADCON0 |= (DEFAULT_ADC << 2);                        //Set the channel select bits to default channel 
            for(uint8_t i = 0; i < 8; i++);                      //insert a small time delay using a for loop to allow channel change
            
            return returnValue;                                  //Return the result  
        }

    }  
}

/*------------------------------------------------------------------------------
 Function: readILCurrentADCRaw()
 *Use: This function reads the default ADC associated with IL current for a 
 * fast read time on CCP1 interrupt
------------------------------------------------------------------------------*/
uint16_t readILCurrentADCRaw(){        
    
    if(~ADCON0bits.GO_nDONE){           //ensure we are not interrupting another read
        ADCON0bits.GO_nDONE = 1;        //Set the Conversion begin bit
        while(ADCON0bits.GO_nDONE);     //Wait until the conversion finishes
        return ((ADRESH<<8)+ADRESL);    //Return the result   
    }
    
}


        