/*
 * Wardrobe-Dimmer-Firmware.c
 *
 * Created: 23/10/2017 13:25:24
 * Author : graym
 */ 

////////////////////// The following fuse bits should be programmed to set the system clock at 1MHz:
////////////////////// CKSEL = “0010”, SUT = “10”, and CKDIV8

#include <avr/io.h>
#include <stdbool.h>
#include <avr/interrupt.h>

// SysTick #defines
#define F_CPU		1000000ul			// CPU running at 1Mhz
#define CTC_Top		125					// Sets SysTick period
// Program #defines
#define potPin		1					// Potentiometer pin
#define pwmPin		4					// PWM output pin
#define rampTime	5000				// Time in ms to ramp up to PWM set point

// SysTick variables
volatile uint32_t SysTick;

// SysTick functions
void SysTick_Config(unsigned short duration) {
	TCCR1 &= ~( (1<<CS10) | (1<<CS11) | (1<<CS12) );	// Set /8 clock prescalar (1MHz/8 = 125kHz)
	TCCR1 |= (1<<CTC1);									// Set clear timer on compare match (CTC)
	OCR1A =	CTC_Top;									// Set the top of CTC on channel a
	TCNT1 =	0;											// Reset timer counter
	TIMSK |=(1<<OCIE1A);								// Output compare interrupt for channel a enabled
	sei();												// Enable interrupts
}

ISR(TIMER1_COMPA_vect) {			//tmr1 CTC / systick tmr
	SysTick++;
}

// ADC code
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
	uint8_t pwmSetPoint = 0;								// Ramping PWM output
	//int pwmMaxPoint = 200;								// Finishing PWM output
	uint8_t pwmMaxPoint = adc_Read();						// Read potentiometer setting for finishing PWM output
	//////////////// ADC read is 8 bit, no down scaling needed
	//uint8_t pwmMaxPoint = potReading / 4;					// Scale down to 0->256 for max PWM set point
	
	
	uint32_t tickStart = SysTick;						// Record ramping start time
    
    while (1) 
    {
		if(rampingFlag)
		{
			uint32_t timeElapsed = (SysTick - tickStart);				// Find time elapsed
			if(timeElapsed >= rampTime)									// If the ramp time has elapsed
			{
				rampingFlag = false;										// Clear flag to indicate ramping no longer in progress
				pwmSetPoint = pwmMaxPoint;									// Set pwm to max set point
			}
			else														// Otherwise set pwm accordingly
			{
				pwmSetPoint = (timeElapsed*pwmMaxPoint) / rampTime;			// Scale to max set point
			}
		}
		else															// If the ramp time has elapsed
		{
			pwmSetPoint = pwmMaxPoint;										// Then set pwm output at max set point
		}
		pwmSetPoint = pwmMaxPoint - pwmSetPoint;						// Invert because LM358 comparator is inverting
		pwmSetPoint += (256 - pwmMaxPoint);								// Start ramping down from 256 not max set point
		//analogWrite(pwmPin, pwmSetPoint);								// Set PWM output
    }
}

