#include <ESP32Servo.h>

Servo myservo;  // create servo object to control a servo
// 16 servo objects can be created on the ESP32

// Recommended PWM GPIO pins on the ESP32 include 2,4,12-19,21-23,25-27,32-33
const int servoPin = 18;
const int potPin = 34;

int updateIntervalMs = 100;
long updateAtMs = 0;

#define TAPE_SIZE 100
int tape[TAPE_SIZE];
int tapeHeadPos = 0;

typedef enum {
  RECORD,
  PLAY,
  INACTIVE
} State;
State currentState = INACTIVE;

int lastPotReading = 0;
int potReadingChangedThreshold = 50;

long lastChangeAtMs = 0;
int noChangeRecordingTimeoutMS = 1000;

void setup() {
  Serial.begin(9600);

  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);
  myservo.attach(servoPin, 100, 2500);

  lastPotReading = analogRead(potPin);
  currentState = INACTIVE;
}

void loop() {
  if (millis() > updateAtMs) {
    updateAtMs = millis() + updateIntervalMs;
    
    switch(currentState) {
      case RECORD:
        Serial.println("RECORD");
        tape[tapeHeadPos] = analogRead(potPin);
        myservo.write(map(tape[tapeHeadPos], 0, 4095, 0, 180));
        break;
      case PLAY:
        Serial.println("PLAY");
        myservo.write(map(tape[tapeHeadPos], 0, 4095, 0, 180));
        break;
      case INACTIVE:
        Serial.println("INACTIVE");
        break;
    }
    tapeHeadPos = (tapeHeadPos + 1) % TAPE_SIZE;

    updateState();
  }
}

void updateState() { 
  int potReading = analogRead(potPin);
  
  if(abs(potReading-lastPotReading) > potReadingChangedThreshold) {
    //Serial.println("CHANGE");
    lastChangeAtMs = millis();

    switch(currentState) {
      case RECORD:    break;
      case PLAY:
        currentState = RECORD;
        break;
      case INACTIVE:
        currentState = RECORD;
        break;
    }
  }
  else if((millis() - lastChangeAtMs) > noChangeRecordingTimeoutMS) {
    //Serial.println("TIMEOUT");
    switch(currentState) {
      case RECORD:
        currentState = PLAY;
        break;
      case PLAY:      break;
      case INACTIVE:  break;
    }
  }
  lastPotReading = potReading;
}
