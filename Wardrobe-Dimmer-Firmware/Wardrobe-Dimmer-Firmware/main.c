/*
 * Wardrobe-Dimmer-Firmware.c
 *
 * Created: 23/10/2017 13:25:24
 * Author : graym
 */ 

#include <avr/io.h>
#include <stdbool.h>

#define potPin 1        // Potentiometer pin
#define pwmPin 4        // PWM output pin
#define rampTime 5000   // Time in ms to ramp up to PWM set point

//////////////////////////////////////////////////////// The following fuse bits should be programmed to set the system clock at 1MHz:
//////////////////////////////////////////////////////// CKSEL = “0010”, SUT = “10”, and CKDIV8

void adc_Initialise(void)
{
	// Clear registers before initialising
	ADMUX = 0;
	ADCSRA = 0;
	// Initialise registers
	//ADMUX &= ~( (1<<REFS0) | (1<<REFS1) );			// Select VCC as ADC reference voltage
	ADMUX |= (1<<ADLAR);								// Select left adjusted data output (allows reading of single register (ADCH); 8-bit resolution is fine)
	ADMUX |= (1<<MUX0);									// Select ADC1 channel
	ADCSRA |= ( (1<<ADPS0) | (1<<ADPS1) );				// Select /8 prescalar, giving 125kHz ADC clock
	ADCSRA |= (1<<ADEN);								// Enable ADC
	
}

uint8_t adc_Read(void)
{
	///////////////// implement: averaging
	static bool firstPass = true;						// Flag for first pass through function
	ADCSRA |= (1<<ADSC);								// Start a single conversion
	while(ADCSRA & (1<<ADSC)) {}						// Wait for ADSC bit to clear when conversion is complete
	uint8_t reading = ADCH;								// Read conversion result
	
	if(firstPass)										// If this is the first pass through, discard the result and take another
	{
		firstPass = false;								// Clear flag
		ADCSRA |= (1<<ADSC);							// Start a single conversion
		while(ADCSRA & (1<<ADSC)) {}					// Wait for ADSC bit to clear when conversion is complete
		reading = ADCH;									// Read conversion result
	}
	return reading;
}

int main(void)
{
	bool rampingFlag = true;							// Represents if ramping is in progress
	int pwmSetPoint = 0;								// Ramping PWM output
	int pwmMaxPoint = 200;								// Finishing PWM output
	//int potReading = adc_Read;							// Read potentiometer setting for finishing PWM output
	//int pwmMaxPoint = potReading / 4;						// Scale down to 0->256 for max PWM set point
	
	//uint32_t tickStart = millis();                    // Record ramping start time
    
    while (1) 
    {
    }
}

