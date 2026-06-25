/*
 Cat Feeder Logic

Setup: restarts the 24 hour period and resets the counters
Use the millis() function to track time
store time since beginning of the 24 hour period
store time since last feeding
store feeding count in the past 24 hours
store feeding count in the past 6 hours
check feeding criteria, if met turn on the LED in the button and enable feeding.
if automatic feeding criteria is met, deliver a feeding
 
 */

#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

// Declare constants for wiring
const unsigned int ledPin = 13;
const unsigned int buttonPin = 10;
const unsigned int motorOut = 3;

// Declare program constants and variables
const unsigned int maxFeedDay = 8;
const unsigned long feedDelayInterval = 1000UL * 60 * 5; // 5 minute delay
const unsigned long feedTimes[] = {
  1000UL * 60 * 5,
  1000UL * 60 * 15,
  1000UL * 60 * 20,
  1000UL * 60 * 25
};
const unsigned int feedScheduleCount = sizeof(feedTimes) / sizeof(feedTimes[0]);
const unsigned long dayInterval = 1000UL * 60 * 60 * 24;

unsigned long timeStartDay = 0;
unsigned long timeLastFeed = 0;
unsigned long timeElapseDay = 0;
unsigned long timeElapseLastFeed = 0;

unsigned int feedCounterDay = 0;
unsigned int nextAutoFeedIndex = 0;

bool isFeedDelay = false;
bool canFeed = true;

unsigned int buttonState = 0;
unsigned int lastButtonState = HIGH;

Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *myMotor = AFMS.getMotor(motorOut);


void setup() {
  Serial.begin(9600);
  Serial.println("Coach's feeder program");

  timeStartDay = millis();
  timeLastFeed = millis();

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  Serial.println("Initializing motor");

  if (!AFMS.begin()) {
    Serial.println("Could not find Adafruit Motor Shield V2. Check I2C/wiring/power.");
    while (1);
  }

  Serial.println("Motor shield found");
  myMotor->setSpeed(150);
  myMotor->run(RELEASE);

  // run tests
  // blink the LED
  Serial.println("LED test");
  for (int i = 0; i <= 3; i++) {
    digitalWrite(ledPin, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
    delay(500);
  }

  // run the motor in short bursts
  Serial.println("Motor test");
  for (int i = 0; i <= 3; i++) {
    Serial.println(i);
    myMotor->setSpeed(150);
    myMotor->run(FORWARD);
    delay(500);
    myMotor->run(RELEASE);
    delay(500);
  }
}

void feed() {
 digitalWrite(ledPin, LOW);
 // activate motor
 Serial.println("Feeding!");
 myMotor->run(FORWARD);
 delay(1000 * 5);
 myMotor->run(RELEASE);
 
 // update variables
 timeLastFeed = millis();
 feedCounterDay++;

 delay(1000);
}

void verboseUpdate() {
  Serial.println("<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>");

  Serial.println("Button State:");
  Serial.print(buttonState);
  Serial.println("\n");

  Serial.println("Last Button State:");
  Serial.print(lastButtonState);
  Serial.println("\n");
  
  Serial.println("Feedings today:");
  Serial.print(feedCounterDay);
  Serial.println("\n");

}

void loop() {
  unsigned long now = millis();

  buttonState = digitalRead(buttonPin);
  timeElapseDay = now - timeStartDay;
  timeElapseLastFeed = now - timeLastFeed;

  isFeedDelay = (timeElapseLastFeed < feedDelayInterval);

  if (feedCounterDay < maxFeedDay && !isFeedDelay) {
    canFeed = true;
    digitalWrite(ledPin, HIGH);
  } else {
    canFeed = false;
    digitalWrite(ledPin, LOW);
  }

  if (buttonState == LOW && lastButtonState == HIGH && canFeed) {
    feed();
  }
  lastButtonState = buttonState;

  if (nextAutoFeedIndex < feedScheduleCount && timeElapseDay >= feedTimes[nextAutoFeedIndex]) {
    Serial.print("Auto slot reached idx=");
    Serial.print(nextAutoFeedIndex);
    Serial.print(" elapsed_ms=");
    Serial.println(timeElapseDay);

    if (feedCounterDay < maxFeedDay && !isFeedDelay) {
      Serial.println("Auto feeding now");
      feed();
      nextAutoFeedIndex++;
    } else {
      Serial.print("Auto feed blocked: max/day=");
      Serial.print(feedCounterDay);
      Serial.print(" delay=");
      Serial.println(isFeedDelay ? "true" : "false");
    }
  }

  if (timeElapseDay >= dayInterval) {
    timeStartDay = now;
    feedCounterDay = 0;
    nextAutoFeedIndex = 0;
  }
}
