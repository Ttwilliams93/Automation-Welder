#include <SPI.h>
#include <Controllino.h>
//T.WILLIAMS 9.20.2018
//Automation Machine Sketch
//Pins
int PowerPin = CONTROLLINO_AO0; //Provides power to the light sensor
int LightSensorPin = CONTROLLINO_AI12; //Reads light sensor
int ResetPin = CONTROLLINO_AI0; //Variable for retracting air cylinder manually
int LimitSwitchFakeSignal = CONTROLLINO_R6; //Signal to retract the air cylinder
int StepperPin = CONTROLLINO_DO3; //Pin for stepping motor
int DirectionPin = CONTROLLINO_DO4; //Pin for changing direction of motor
int LimitSwitchSignal = CONTROLLINO_AI1; //Pin for reading the extension limit switch
//Variables
int LightSensorReading; //Variable for light sensor reading
long StepCountFWD = 0; //Variable for recording step count forwards
long StepCountBWD = 0; //Variable for recording step count backwards
long StepCount = 0;
unsigned long CurrentTime;
unsigned long PreviousStepTime;
int LimitSwitchStatus; //Variable for limit switch status
int PulseWidthMicros = 10;
int MotorStepDelayFWD = 125; //Step Delay in micros
int MotorStepDelayBWD = 125;
int LightThreshold = 130;
int MaxStepDistance = 16000; //10 Steps per distance step, .00012" per step
void setup() { //Runs once
  // Serial.begin(9600);
  pinMode(ResetPin, INPUT);
  pinMode(LightSensorPin, INPUT);
  pinMode(LimitSwitchSignal, INPUT);
  pinMode(PowerPin, OUTPUT);
  pinMode(StepperPin, OUTPUT);
  pinMode(DirectionPin, OUTPUT);
  digitalWrite(LimitSwitchFakeSignal, LOW);
  delay(10);
  pinMode(LimitSwitchFakeSignal, OUTPUT);
  analogWrite(PowerPin, 1023);
}
/*FUNCTION:
  AIR CYLINDER AUTOMATICALLY EXTENDS FROM AUTOMATION MACHINE (AS PER AUTOMATION MACHINE LADDER LOGIC)
  WHEN AIR CYLINDER EXTENDS, THE EXTENSION LIMIT SENSOR SENDS HIGH SIGNAL AND THE CONTROLLINO READS IT.
  WHEN EXTENSION LIMIT GOES HIGH, THE MOTOR BEGINS MOVING FORWARDS AND COUNTING STEPS.
  AFTER DETECTING A WELD(SPARK WITH THE LIGHT SENSOR), THE MOTOR DIRECTION SIGNAL REVERSES,
  THE CONTROLLINO SENDS A RETRACTION SIGNAL TO THE AUTOMATION MACHINE, AND THE
  MOTOR STEPS BACK THE NUMBER OF STEPS IT JUST WENT FORWARDS TO RESET TO THE HOME POSITION.
*/
void loop() {
  LimitSwitchStatus = digitalRead(LimitSwitchSignal);
  if (LimitSwitchStatus == HIGH) { //Preparation for entering the while loop
    ResetStepCounts();
    PreviousStepTime = micros();
    while (LimitSwitchStatus == HIGH) {
      if (digitalRead(DirectionPin) == HIGH) {
        digitalWrite(DirectionPin, LOW);
        delay(10);
      }
      CurrentTime = micros();
      TakeStep(MotorStepDelayFWD);
      StepCountFWD = StepCount;
      LightSensorReading = analogRead(LightSensorPin);
      if (StepCountFWD >= MaxStepDistance) {             //IF DISTANCE TRAVELED BECOMES TOO HIGH(NO WELD OCCURS), RETRACT THE CYLINDER
        digitalWrite(LimitSwitchFakeSignal, HIGH);
        LimitSwitchStatus = LOW;
        digitalWrite(DirectionPin, HIGH);
        delay(10);
        StepCount = 0;
        MoveMotorBack();
      }
      if (LightSensorReading > LightThreshold) {
        Serial.print(LightSensorReading);
        Serial.println(" Sensor Reading");
        digitalWrite(LimitSwitchFakeSignal, HIGH);
        LimitSwitchStatus = LOW;
        digitalWrite(DirectionPin, HIGH);
        StepCount = 0;
        MoveMotorBack();
        // Serial.print(StepCountFWD);
        // Serial.println(" Steps FWD");
        // Serial.print(StepCountBWD);
        //Serial.println(" Steps BWD");
      }
    }
    digitalWrite(LimitSwitchFakeSignal, LOW);
    delay(1500);
  }
}
void TakeStep(int MotorDelay) {
  if (abs(CurrentTime - PreviousStepTime) >= MotorDelay) {
    digitalWrite(StepperPin, HIGH);
    delayMicroseconds(PulseWidthMicros); //Send StepSignal
    digitalWrite(StepperPin, LOW);
    PreviousStepTime = micros();
    StepCount = StepCount + 1;
  }
}
void MoveMotorBack() {
  while (StepCountBWD < StepCountFWD) {
    CurrentTime = micros();
    TakeStep(MotorStepDelayBWD);
    StepCountBWD = StepCount;
  }
}
void ResetStepCounts() {
  StepCount = 0;
  StepCountFWD = 0;
  StepCountBWD = 0;
}
