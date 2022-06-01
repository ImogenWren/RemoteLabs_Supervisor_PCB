/* opticalEncoder.cpp

     Small Library to handle Optical Encoders similar to:


    Broadcom Incremental Encoder 500 ppr, AEDB-9140-A13 optical encoder

    Optical Encoder:
    https://uk.rs-online.com/web/p/motion-control-sensors/7967806

    Datasheet:
    https://docs.rs-online.com/34ce/0900766b812cdcb0.pdf

          Software aims to count individual ticks, absolute postion (degrees),
          and track direction and velocity of the optical encoder wheel.


     Imogen Wren
     01/06/2022




*/

/*  Encoder Specifics:

     1000 ppt encoder

     Therefore rotaryCount will be init at 1000, and its range constrained to 1000 +- 500
     to track absolute position



*/

#include "opticalEncoder.h"


// Constructor
opticalEncoder::opticalEncoder(int16_t index_pin, int16_t pin_A, int16_t pin_B):
  indexPin(index_pin),
  pinA(pin_A),
  pinB(pin_B)
{
}


void opticalEncoder::encoderBegin(int16_t pulsePerRevolution) {
  pinMode(indexPin, INPUT);
  pinMode(pinA, INPUT);
  pinMode(pinB, INPUT);
  rotaryCount = pulsePerRevolution;
  ppr = pulsePerRevolution;
  lowerBound = (pulsePerRevolution / 2);
  upperBound = pulsePerRevolution + lowerBound;
  milliDegPerPulse = (360000 / ppr); // to avoid floats, degree per pulse is saved as a value *1000

}




// Function counts encoder pulses and determines the direction of the encoder
void opticalEncoder::encoderDirection() {
  prevCount = rotaryCount;
  if (fired)  {
    if (up) {
      rotaryCount++;
    } else {
      rotaryCount--;
    }
  //  fired = false;   // MOVED TO ITS OWN METHOD. IF THIS CAUSES PROBLEMS PUT BACK HERE
  }
  if (rotaryCount > prevCount) {          //
    clockwiseRotation = true;                      //
  } else if (rotaryCount < prevCount) {             // Not ideal as how does it cope with rollover?! // Will it work having this here first, then doing the rollaround?
    clockwiseRotation = false;
  }
  if (rotaryCount < lowerBound) {
    rotaryCount = upperBound;
  } else if (rotaryCount > upperBound) {
    rotaryCount = lowerBound;
  }
}



// Function to return the current position of the encoder in degrees from center position (1000)  (-180 to +180)deg

// 180/500 = 0.36 so each pulse is 0.36deg apart - makes sense 360/1000ppt should have seen that coming

int32_t opticalEncoder::calcHeading(int16_t encoderPos) {
  heading_milliDeg = (encoderPos - ppr) * milliDegPerPulse;
  // Serial.println(heading_deg);
  // floatToChar(heading_deg);    // THis returns a global variaable for now for ease No longer a float so not needed here
  return heading_milliDeg;
}




void opticalEncoder::plotHeader() {
  Serial.println("Ticks, Heading");
}



void opticalEncoder::plotEncoder() {
  float heading_deg = float((calcHeading(rotaryCount)) / 1000.0);
  Serial.println(heading_deg);
  floatToChar (heading_deg);                                                // Returns headingString as global variable 
  char buffer[64];
  sprintf(buffer, "%u, %s, %i ", rotaryCount, headingString, rpm);    // Needs %u for UNSIGNED
  Serial.println(buffer);
}



// Also saving some data as a char string to use later
void opticalEncoder::floatToChar (float finput) {
  // char *tmpSign = (finput < 0) ? "-" : "";         // Work out what sign to add
  float tmpVal = (finput < 0) ? finput * -1 : finput; // If less than 0, make positive

  int tmpInt1 = int(finput);   // cast to int to get before .point
  float tmpFrac = tmpVal - tmpInt1;  // Get the fraction
  int tmpInt2 = trunc(tmpFrac * 1000);   // Turn into an integer

  tmpInt2 = (tmpInt2 < 0) ? tmpInt2 * -1 : tmpInt2;
  // Then print as parts using sprintf - Note 0 padding for fractional bit
  //  sprintf(headingString, "%s%d.%04d", tmpSign, tmpInt1, tmpInt2);
  sprintf(headingString, "%d.%04d", tmpInt1, tmpInt2);
}




void opticalEncoder::testPlot(int A, int B, int C) {
  char buffer[64];
  sprintf(buffer, "%i, %i, %i ", A, B, C);
  Serial.println(buffer);
}



void opticalEncoder::testInputs() {
  int pinAstate = digitalRead(pinA);
  int pinBstate = digitalRead(pinB);
  int indexState = digitalRead(indexPin);
  testPlot(pinAstate, pinBstate, indexState);
  //testPlot(pinAstate, 0, 0);
}

  pinMode(indexPin, INPUT);
  pinMode(pinA, INPUT);
  pinMode(pinB, INPUT);

void opticalEncoder::calcRPM(){
 if (fired){                                                     // uses fired bool which is reset in encodeDirection function, so this must go before that one // actually order is all messes up, might work something different
 int32_t period =  sampleMicros - lastSampleMicros;
 int32_t travel = heading_milliDeg - last_heading_milliDeg;
lastSampleMicros = sampleMicros;
last_heading_milliDeg = heading_milliDeg;
  }
  // Got the data we need, now we just need maths

  int32_t full_revolution = 360000/travel;    // number of times this time period is needed to complete full revolution

  int32_t timeForFullRevolution = period*full_revolution;  // Say this is = to 0.5 seconds for 1 revolution = 2rps, if = 2 seconds for 1r = 0.5rps
  float rps = 1/timeForFullRevolution; // revolutions per second

  rpm = rps*60;

  Serial.println(rpm);
}

void opticalEncoder::resetFired(){
  fired = false;  
}

void opticalEncoder::ISR () {
  if (digitalRead (pinA)) {                 // if change is detected on interrupt pin
    up = !digitalRead (pinB);
  } else {
    up = digitalRead (pinB);
  }
  fired = true;
  sampleMicros = micros();
}
