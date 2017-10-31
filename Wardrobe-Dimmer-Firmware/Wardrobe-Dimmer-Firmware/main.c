/*
 * Wardrobe-Dimmer-Firmware.c
 *
 * Created: 23/10/2017 13:25:24
 * Author : graym
 */ 

////////// The fuse bits in the target device should be programmed to use the internal RC 8MHz oscillator with the /8 option (CKDIV8)

#define F_CPU		1000000ul

////////// INCLUDES //////////
#include <avr/io.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <util/delay.h>

////////// DEFINES //////////
// Program #defines
#define rampTime	5000						// Time in ms to ramp up to PWM set point
#define pwmMin		80							// ADC counts representing minimum pot setting
#define pwmMax		175							// ADC counts representing maximum pot setting
#define pwmRange	(pwmMax - pwmMin)			// Range of usable adc counts
#define adcRes		256							// Number of adc counts
#define pwmSF		(adcRes*100 / pwmRange)		// Scale factor for pwm
// SysTick #defines
#define CTC_Top		125							// Sets SysTick period

////////// VARIABLES //////////
// Program variables
typedef enum state
{
	STATE_RAMP,				// Ramping up from off to set brightness
	STATE_FOLLOW			// Ramping complete, just tracking dimmer pot
} State_t;

// SysTick variables
volatile uint32_t SysTick = 0;

////////// FUNCTIONS //////////
// ADC functions
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

// PWM functions
void pwm_Initialise(void)
{
	DDRB   &=   ~(1 << PB1);							// Set pwm pin as input (keep output turned off until PWM has started to avoid initial low pulse which causes LEDs to flash)
	PORTB  |=   (1 << PB1);								// Set internal pullup
	TCCR0B |=   (1<<CS01);								// Set prescalar to /8 (125kHz timer clock, ~500Hz PWM frequency)
	OCR0B   = 0;										// Initialise to zero duty cycle
	TCCR0A |= ( (1<<WGM00) | (1<<WGM01)  );				// Set PWM fast mode
	TCNT0   = 0;										// Reset counter
	TCCR0A |=   (1 << COM0B0) | (1 << COM0B1);			// Set OC0A/OC0B on Compare Match, clear OC0A/OC0B at BOTTOM (inverting mode)
	
	_delay_ms(5);										// Delay before turning on pwm output to avoid initial low pulse
	
	DDRB   |=   (1 << PB1);								// Turn on pwm output
}

/* OCR0B register is 8-bits, set PWM from 0-255 */
void pwm_Set(uint8_t setPoint)
{
	OCR0B = setPoint;									// Set pwm output compare register to desired value
}

uint8_t pwm_GetSetPoint(void)
{
	uint8_t reading = adc_Read();											// Read the pot
	if(reading > pwmMax)													// If it's higher than the expected max, set to max
	{
		reading = pwmMax;
	}
	else if(reading < pwmMin)												// If it's lower than the expected min, set to min
	{
		reading = pwmMin;
	}
	uint32_t readingScaled = ( ( (reading - pwmMin) * pwmSF ) / 100 );		// Scale the reading
	return (uint8_t)readingScaled;
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

int main(void)
{
	pwm_Initialise();
	adc_Initialise();
	SysTick_Config();
	
	uint8_t pwmSetPoint = 0;								// Ramping PWM output
	uint8_t pwmMaxPoint = pwm_GetSetPoint();				// Read potentiometer setting for finishing PWM output
															// ADC read is 8 bit, no down scaling needed
	uint32_t tickStart = SysTick;							// Record ramping start time
    
	State_t state = STATE_RAMP;
	
	while(1)
	{
		switch(state)
		{
			case STATE_RAMP:
			{
				uint32_t timeElapsed = (SysTick - tickStart);				// Find time elapsed
				if(timeElapsed >= rampTime)									// If the ramp time has elapsed
				{
					state = STATE_FOLLOW;										// Change state to following
					pwmSetPoint = pwmMaxPoint;									// Set pwm to max set point
				}
				else														// Otherwise set pwm accordingly
				{
					pwmSetPoint = ( (timeElapsed*pwmMaxPoint) / rampTime );	// Scale to max set point
				}
				break;
			}
			case STATE_FOLLOW:
			{
				pwmSetPoint = pwm_GetSetPoint();							// Read pot for pwm setting
				break;
			}
			default:
			{
				while(1);
			}
		}
		pwm_Set(pwmSetPoint);												// Set PWM output
	}
}

