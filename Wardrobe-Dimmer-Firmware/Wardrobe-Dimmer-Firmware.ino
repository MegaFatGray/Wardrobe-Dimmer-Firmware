#define potPin 1
#define pwmPin 4

void setup() {
  // put your setup code here, to run once:
  pinMode(pwmPin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int potReading = analogRead(potPin);              // Read potentiometer setting
  int pwmSetPoint = potReading / 4;                 // Scale down to 0->256 for PWM output
  analogWrite(pwmPin, pwmSetPoint);                 // Set PWM output
}
