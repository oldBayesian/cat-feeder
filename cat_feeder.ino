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

// Declare constants for wiring
const unsigned int ledPin = 13;
const unsigned int buttonPin = 7;
const unsigned int motorOut = 3;

// Declare program constants and variables
const unsigned int normalFeedDay = 4;
const unsigned int maxFeedDay = 6;
const unsigned int maxFeedDayPart = 2;
const unsigned long feedTime0 = 1000UL * 60 * 60 * 6; // feeding at 6a
const unsigned long feedTime1 = 1000UL * 60 * 60 * 7; // feeding at 7a
const unsigned long feedTime2 = 1000UL * 60 * 60 * 17; // feeding at 5p
const unsigned long feedTime3 = 1000UL * 60 * 60 * 18; // feeding at 6p

const unsigned long dayPartInterval = (1000UL * 60 * 60 * 12); // day parts are 12 hours
const unsigned long feedDelayInterval = (1000UL * 10 * 1); // 5 minute delay

unsigned long timeStartDay = 0;
unsigned long timeElapseDay = 0;
unsigned long timeStartDayPart = 0;
unsigned long timeElapseDayPart = 0;
unsigned long timeLastFeed = 0;
unsigned long timeElapseLastFeed = 0;

unsigned int feedCounterDay = 0;
unsigned int feedCounterDayPart = 0;

bool isFeedDelay = false;
bool isFeedTime = false;
bool canFeed = true;

unsigned int buttonState = 0;
unsigned int lastButtonState = 0;
bool buttonPress = false;

Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *myMotor = AFMS.getMotor(motorOut);


void setup() {
  Serial.begin(9600);
  Serial.println("Coach's feeder program");
  
  // start timer for the 24 clock
  timeStartDay = millis();

  // setup button and LED
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);

  // setup motor
  AFMS.begin();
}


void feed() {
 // activate motor
 Serial.println("Feeding!");
 myMotor->run(FORWARD);
 delay(1000 * 10);
 myMotor->run(RELEASE);
 
 // update variables
 timeLastFeed = millis();
 feedCounterDay++;
 feedCounterDayPart++;

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

  Serial.println("Feedings in day part:");
  Serial.print(feedCounterDayPart);
  Serial.println("\n");
}

void loop() {
  // detect button state
  buttonState = digitalRead(buttonPin);

  // verboseUpdate();

  // determine eligibility for feeding
  timeElapseDay = millis() - timeStartDay;
  timeElapseDayPart = millis() - timeStartDayPart;
  timeElapseLastFeed = millis() - timeLastFeed;

// manual feeding logic start
  if (timeElapseLastFeed < feedDelayInterval) {
    isFeedDelay = true;
    //Serial.println("Feed Delay Active");
  } else {
    isFeedDelay = false;
    //Serial.println("Feed Delay Inactive");
  }

  if (feedCounterDay <= maxFeedDay and feedCounterDayPart <= maxFeedDayPart and not isFeedDelay) {
    canFeed = true;
    digitalWrite(ledPin, HIGH);
    //Serial.println("Ready to feed");
  } else {
    canFeed = false;
    digitalWrite(ledPin, LOW);
    //Serial.println("Not Ready to feed");
  }

  if (buttonState != lastButtonState and canFeed) {
    if (buttonState == HIGH and feedCounterDay <= maxFeedDay and feedCounterDayPart <= maxFeedDayPart and not isFeedDelay) {
      feed();
      //verboseUpdate();
    }
  }
  lastButtonState = buttonState;
// end of manual feeding logic
  

// hard coded feeding times; ignores manual feeding logic
  if (feedCounterDayPart < (maxFeedDayPart - 1) and timeElapseDay > feedTime0) {
    feed(); 
  }
  if (feedCounterDayPart < maxFeedDayPart and timeElapseDay > feedTime1) {
    feed(); 
  }
  if (feedCounterDayPart < (normalFeedDay -1) and timeElapseDay > feedTime2) {
    feed(); 
  }
  if (feedCounterDayPart < normalFeedDay and timeElapseDay > feedTime3) {
    feed(); 
  }

  if (timeElapseDay > dayPartInterval) {
    feedCounterDayPart = 0; // with only 2 day parts, we can ignore the feedCounter for the day part when during the latter day part
  }
  
}
