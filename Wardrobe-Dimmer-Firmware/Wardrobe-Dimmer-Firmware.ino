#define potPin 1        // Potentiometer pin
#define pwmPin 4        // PWM output pin
#define rampTime 5000   // Time in ms to ramp up to PWM set point

bool rampingFlag = true;                          // Represents if ramping is in progress
int pwmSetPoint = 0;                              // Ramping PWM output
int potReading = analogRead(potPin);              // Read potentiometer setting
//int pwmMaxPoint = potReading / 4;                 // Scale down to 0->256 for max PWM set point
int pwmMaxPoint = 200;
uint32_t tickStart = millis();                    // Record ramping start time

void setup() {
  // put your setup code here, to run once:
  pinMode(pwmPin, OUTPUT);                          // Initialise PWM pin as output
  digitalWrite(pwmPin, HIGH);                       // Initialise PWM pin high to hold mosfet off (LM358 comparator is inverting)
}

void loop() {
  // put your main code here, to run repeatedly:
  if(rampingFlag)
  {
    uint32_t timeElapsed = (millis() - tickStart);          // Find time elapsed
    if(timeElapsed >= rampTime)                             // If the ramp time has elapsed
    {
      rampingFlag = false;                                    // Clear flag to indicate ramping no longer in progress
      pwmSetPoint = pwmMaxPoint;                              // Set pwm to max set point
    }
    else                                                    // Otherwise set pwm accordingly
    {
      pwmSetPoint = (timeElapsed*pwmMaxPoint) / rampTime;     // Scale to max set point
    }
  }
  else
  {
    pwmSetPoint = pwmMaxPoint;
  }
  pwmSetPoint = pwmMaxPoint - pwmSetPoint;                  // Invert because LM358 comparator is inverting
  pwmSetPoint += (256 - pwmMaxPoint);                       // Start ramping from 256 not max set point
  analogWrite(pwmPin, pwmSetPoint);                         // Set PWM output
}
