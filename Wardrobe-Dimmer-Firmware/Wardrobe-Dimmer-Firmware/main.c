/*
 * Wardrobe-Dimmer-Firmware.c
 *
 * Created: 23/10/2017 13:25:24
 * Author : graym
 */ 

////////// The fuse bits in the target device should be programmed to use the internal RC 8MHz oscillator with the /8 option (CKDIV8)

////////// INCLUDES //////////
#include <avr/io.h>
#include <stdbool.h>
#include <avr/interrupt.h>

////////// DEFINES //////////
// Program #defines
#define potPin		1					// Potentiometer pin
#define pwmPin		4					// PWM output pin
#define rampTime	5000				// Time in ms to ramp up to PWM set point
// SysTick #defines
#define F_CPU		1000000ul			// CPU running at 1Mhz
#define CTC_Top		125					// Sets SysTick period

////////// VARIABLES //////////
// SysTick variables
volatile uint32_t SysTick;

////////// FUNCTIONS //////////
// PWM functions
void pwm_Initialise(void)
{
	DDRB   |=   (1 << PB1);								// Set pwm pin as output
	PORTB  |=   (1 << PB1);								// Set pwm pin high (LM358 comparator is inverted so this turns off mosfet)
	TCCR0A |=   (1 << COM0B1);							// Clear OC0B (PB1) on Compare Match, set OC0B at BOTTOM (non-inverting mode)
	TCCR0A |= ( (1<<WGM00) | (1<<WGM01)  );				// Set PWM fast mode
	TCCR0B |=   (1<<CS01);								// Set prescalar to /8 (125kHz timer clock, ~500Hz PWM frequency)
}

/* OCR0B register is 8-bits, set PWM from 0-255 */
void pwm_Set(uint8_t setPoint)
{
	OCR0B = setPoint;									// Set pwm output compare register to desired value
}

// SysTick functions
void SysTick_Config(void)
{	
	TCCR1 |= (1<<CS12);									// Set /8 clock prescalar (1MHz/8 = 125kHz)
	TCCR1 |= (1<<CTC1);									// Set clear timer on compare match (CTC)
	OCR1C =	CTC_Top;									// Set the top of CTC on channel c
	TCNT1 =	0;											// Reset timer counter
	TIMSK |= (1<<OCIE1A);								// Output compare interrupt for channel a enabled
	sei();												// Enable interrupts
}

ISR(TIMER1_COMPA_vect)
{
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
	SysTick_Config();
	DDRB   |=   (1 << PB3);								// Set pin as output
	PORTB  &=   ~(1 << PB3);							// Set pin low
	
	//adc_Initialise();
	//pwm_Initialise();
	
	
	while(1);
	
	
	/*
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
	*/
}

