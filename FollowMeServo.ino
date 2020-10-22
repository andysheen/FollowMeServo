#include <ESP32Servo.h>

Servo myservo;

const int servoPin = 13;
const int potPin = 25;
const int ledPin = 2;
const int buttonPin = 0;

int updateIntervalMs = 100;
long updateAtMs = 0;

#define TAPE_SIZE 50
int tape[TAPE_SIZE];
int tapeHeadPos = 0;

typedef enum {
  RECORD,
  PLAY,
  INACTIVE
} State;
State currentState = INACTIVE;

int lastInputValue = 0;
int inputValueChangedThreshold = 5;

long lastChangeAtMs = 0;
int noChangeRecordingTimeoutMS = 1000;

void setup() {
  Serial.begin(115200);
  pinMode(26, OUTPUT);
  digitalWrite(26, 1);
  pinMode(33, OUTPUT);
  digitalWrite(33, 0);
  
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);

  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);
  myservo.attach(servoPin, 100, 2500);

  lastInputValue = -1;
  currentState = INACTIVE;
}

void loop() {
  if (millis() > updateAtMs) {
    Serial.println(analogRead(potPin));
    updateAtMs = millis() + updateIntervalMs;

    digitalWrite(ledPin, tapeHeadPos == 0);

    int inputValue = min(255, (int)(map(analogRead(potPin), 0, 4095, 0, 255) + map(digitalRead(buttonPin), 1, 0, 0, 255)));
    updateState(inputValue);

    switch(currentState) {
      case RECORD:
        Serial.println("RECORD");
        record(inputValue);
        break;
      case PLAY:
        Serial.println("PLAY");
        play(tape[tapeHeadPos]);
        break;
      case INACTIVE:
        Serial.println("INACTIVE");
        break;
    }
    tapeHeadPos = (tapeHeadPos + 1) % TAPE_SIZE;
  }
}

void record(int inputValue) {
  tape[tapeHeadPos] = inputValue;
  play(inputValue);
}

void play(int inputValue) {
  myservo.write(map(inputValue, 0, 255, 0, 180));
}

void updateState(int inputValue) {
  if(lastInputValue >= 0) {
    if(abs(inputValue-lastInputValue) > inputValueChangedThreshold) {
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
  }
  lastInputValue = inputValue;
}
