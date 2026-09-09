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
const unsigned int motorOut = 4;
const unsigned int soundSensorPin = A8;
const unsigned int ultrasonicTriggerPin = 14;
const unsigned int ultrasonicEchoPin = 15;
const unsigned long sensorWindowMs = 10UL;
const unsigned long spikeThreshold = 20UL;
const unsigned long spikeRefractoryMs = 100UL;
const unsigned int distanceAverageSamples = 10;
const float maxDistanceCm = 30.0;

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

String serialCmd = "";
bool allTestMode = false;
unsigned long motorRunUntilMs = 0;

unsigned long sensorWindowStartMs = 0;
int sensorMinValue = 4095;
int sensorMaxValue = 0;
long sensorSum = 0;
unsigned long sensorSampleCount = 0;
unsigned long kibbleCount = 0;
unsigned long lastSpikeMs = 0;
bool spikeArmed = true;
float distanceSamples[distanceAverageSamples] = {};
float distanceSampleSum = 0;
unsigned int distanceSampleIndex = 0;
unsigned int distanceSampleCount = 0;

Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *myMotor = AFMS.getMotor(motorOut);

void handleSerialCommands() {
  while (Serial.available()) {
    char ch = Serial.read();

    if (ch == '\n' || ch == '\r') {
      if (serialCmd.equalsIgnoreCase("RUN")) {
        allTestMode = false;
        motorRunUntilMs = 0;
        myMotor->run(RELEASE);
        Serial.println("Normal feeder mode enabled");
      } else if (serialCmd.equalsIgnoreCase("MT")) {
        allTestMode = false;
        startMotor(5);
        Serial.println("MT | MOTOR ON | 5 seconds");
      } else if (serialCmd.equalsIgnoreCase("ALLT")) {
        allTestMode = true;
        sensorWindowStartMs = millis();
        kibbleCount = 0;
        lastSpikeMs = 0;
        spikeArmed = true;
        distanceSampleSum = 0;
        distanceSampleIndex = 0;
        distanceSampleCount = 0;
        for (unsigned int i = 0; i < distanceAverageSamples; i++) {
          distanceSamples[i] = 0;
        }
        resetSensorWindow();
        Serial.println("ALLT | time_ms|peak_to_peak|kibble_count|distance_cm|motor_active");
      } else if (allTestMode && isDurationCommand(serialCmd)) {
        startMotor(serialCmd.toFloat());
      } else if (serialCmd.length() > 0) {
        Serial.print("Unknown command: ");
        Serial.println(serialCmd);
      }
      serialCmd = "";
      break;
    } else {
      serialCmd += ch;
    }
  }
}

bool isDurationCommand(const String &value) {
  if (value.length() == 0) return false;
  bool decimalSeen = false;
  for (unsigned int i = 0; i < value.length(); i++) {
    char ch = value.charAt(i);
    if (ch == '.' && !decimalSeen) {
      decimalSeen = true;
    } else if (ch < '0' || ch > '9') {
      return false;
    }
  }
  return true;
}

void startMotor(float seconds) {
  unsigned long durationMs = (unsigned long)(seconds * 1000.0);
  motorRunUntilMs = millis() + durationMs;
  myMotor->setSpeed(150);
  myMotor->run(FORWARD);
}

bool motorIsActive() {
  return (long)(motorRunUntilMs - millis()) > 0;
}

void updateMotor() {
  if (!motorIsActive()) {
    motorRunUntilMs = 0;
    myMotor->run(RELEASE);
  }
}

void resetSensorWindow() {
  sensorMinValue = 4095;
  sensorMaxValue = 0;
  sensorSum = 0;
  sensorSampleCount = 0;
}

float readDistanceCm() {
  digitalWrite(ultrasonicTriggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(ultrasonicTriggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(ultrasonicTriggerPin, LOW);

  unsigned long echoDuration = pulseIn(ultrasonicEchoPin, HIGH, 30000);
  if (echoDuration > 0) {
    float distanceCm = echoDuration * 0.0343 / 2.0;
    if (distanceCm > maxDistanceCm) distanceCm = maxDistanceCm;
    distanceSampleSum -= distanceSamples[distanceSampleIndex];
    distanceSamples[distanceSampleIndex] = distanceCm;
    distanceSampleSum += distanceCm;
    distanceSampleIndex = (distanceSampleIndex + 1) % distanceAverageSamples;
    if (distanceSampleCount < distanceAverageSamples) distanceSampleCount++;
  }

  return distanceSampleCount > 0 ? distanceSampleSum / distanceSampleCount : 0;
}

void runAllTest() {
  unsigned long now = millis();
  int value = analogRead(soundSensorPin);
  if (value < sensorMinValue) sensorMinValue = value;
  if (value > sensorMaxValue) sensorMaxValue = value;
  sensorSum += value;
  sensorSampleCount++;

  if ((now - sensorWindowStartMs) >= sensorWindowMs) {
    unsigned int peakToPeak = sensorMaxValue - sensorMinValue;
    if (peakToPeak < spikeThreshold) {
      spikeArmed = true;
    } else if (spikeArmed && (now - lastSpikeMs) >= spikeRefractoryMs) {
      kibbleCount++;
      lastSpikeMs = now;
      spikeArmed = false;
    }

    float distanceCm = readDistanceCm();
    Serial.print(now);
    Serial.print("|");
    Serial.print(peakToPeak);
    Serial.print("|");
    Serial.print(kibbleCount);
    Serial.print("|");
    Serial.print(distanceCm, 2);
    Serial.print("|");
    Serial.println(motorIsActive() ? 1 : 0);

    resetSensorWindow();
    sensorWindowStartMs = millis();
  }
}

void runStartupTests() {
  Serial.println("LED test");
  for (int i = 0; i <= 3; i++) {
    digitalWrite(ledPin, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
    delay(500);
  }

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

void setup() {
  Serial.begin(115200);
  Serial.println("Coach's feeder program");
  Serial.println("Commands: MT, ALLT, RUN");

  timeStartDay = millis();
  timeLastFeed = millis();

  pinMode(ultrasonicTriggerPin, OUTPUT);
  pinMode(ultrasonicEchoPin, INPUT);
  digitalWrite(ultrasonicTriggerPin, LOW);
  pinMode(ledPin, OUTPUT);

  Serial.println("Initializing motor shield");

  if (!AFMS.begin()) {
    Serial.println("Could not find Adafruit Motor Shield V2. Check I2C/wiring/power.");
    while (1);
  }

  Serial.println("Motor shield found");
  myMotor->setSpeed(150);
  myMotor->run(RELEASE);

  Serial.println("The motor will remain idle until MT or a numeric ALLT command is entered.");
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
  
  Serial.println("Feedings today:");
  Serial.print(feedCounterDay);
  Serial.println("\n");

}

void loop() {
  handleSerialCommands();
  updateMotor();

  if (allTestMode) {
    runAllTest();
    return;
  }

  unsigned long now = millis();

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
