/*   opticalEncoder Example

      Test Program for Broadcom Incremental Encoder 500 ppr, AEDB-9140-A13 optical encoder

      Optical Encoder:
      https://uk.rs-online.com/web/p/motion-control-sensors/7967806

      Datasheet:
      https://docs.rs-online.com/34ce/0900766b812cdcb0.pdf



     Test software aims to count "Ticks" and Direction to prove hardware is functional.

     Can be expanded to return RPM of motor, and possibly absolute position in (degrees) given a calibration for starting position.

      Imogen Heard 25/04/2022

      Test for Supervisor PCB for practicable.io remote Lab Experiments


*/

/*  Encoder Specifics:

     1000 ppt encoder

     Therefore rotaryCount will be init at 1000, and its range constrained to 1000 +- 500
     to track absolute position



*/


#ifndef encoderFunctions_h
#define encoderFunctions_h







//#include <autoDelay.h>

#include "opticalEncoder.h"

#define INDEX_PIN 4    //Not sure what this pin is actually required for
#define PIN_A     3
#define PIN_B     2

#define PULSES_PER_REVOLUTION 1000
opticalEncoder OE(INDEX_PIN, PIN_A, PIN_B);




void encoderSetup() {
  Serial.begin(115200);
  attachInterrupt (digitalPinToInterrupt (PIN_A), isr , CHANGE);   // interrupt 0 is pin 2
  OE.encoderBegin(PULSES_PER_REVOLUTION);
  OE.plotHeader();
}


void encoderLoop() {                     // Order of operations is important here           
  OE.encoderDirection();                  // Handles direction and rotaryCount update after trigger by ISR  
  OE.calcHeading(rotaryCount);             // calculates heading angle in milliDegrees
  OE.calcRPM();                             // Uses milliDegrees traveled and period to calculate the RPM
  OE.plotEncoder();                         // Plot all the values to the serial monitor  #TODO: Update to cover new maths outputs and include direction bool clockwise. In begin we can have user setting to change polarity of bool clockwise incase up is reversed
  OE.resetFired();                          // reset fired now that all maths have been finished #TODO: Make sure all maths functions operate only if(fired); also plotEncoder?
  // testInputs();
}


// isr just calls the isr from inside the library
void isr(){
  OE.ISR();
}








#endif