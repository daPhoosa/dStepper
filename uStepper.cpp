/*
 *      uStepper
 *
 *
 */

#include "uStepper.h"



uStepper::uStepper(float _stepsPerMM, int direction, float _tickRateHz, int _stepPin, int _dirPin, int _enablePin){
   
   MMPerStep  = 1.0f / _stepsPerMM; // [mm/step]
   stepsPerMM = _stepsPerMM;        // [steps/mm]
   
   tickPerMin = _tickRateHz * 60.0f;
   maxFeedRate = int32_t(tickPerMin * MMPerStep) / 8;
   
   position = 0;
   tickPerStep = 2e9;
   moveDirection = Stopped;
   ditherCounter = 1;
   
   // Config Step Pin
   stepPin = _stepPin;
   pinMode(stepPin, OUTPUT);
   digitalWrite(stepPin, LOW);
   stepPinOn = false;
   
   // Config Direction Pin
   directionPin = _dirPin;
   pinMode(directionPin, OUTPUT);
   digitalWrite(directionPin, LOW);
   
   if(direction > 0){
      FORWARD = 1;
      REVERSE = 0;
   }else{
      FORWARD = 0;
      REVERSE = 1;      
   }

   // Config Enable Pin
   enablePin = _enablePin;
   pinMode(enablePin, OUTPUT);
   digitalWrite(enablePin, LOW);

}


uStepper::~uStepper()
{
}


void uStepper::setSpeed(int32_t feedRate){  // pass in speed [MM/min]
   
   feedRate = constrain(feedRate, -maxFeedRate, maxFeedRate);
   
   if(feedRate < -6) // avoid silly low speeds to prevent 16bit int overflow
   {
      digitalWrite(directionPin, REVERSE);
      moveDirection = Negative;
   }
   else if(feedRate > 6)
   {
      digitalWrite(directionPin, FORWARD);
      moveDirection = Positive;
   }
   else
   {
      moveDirection = Stopped;
      tickPerStep = 2e9;
      return;  // exit now
   }
   
   float feedRateABS = float(abs(feedRate));
   
   float stepsPerMin = feedRateABS * stepsPerMM;  // [MM/min] * [step/MM] = [steps/min]

   float idealTickPerStep = tickPerMin / stepsPerMin;  // [tick/min] / [step/min] = [tick/step]

   tickPerStep = long(idealTickPerStep); 
   
   // compute step dithering
   float offset = idealTickPerStep - float(tickPerStep); // should be a decimal between 0 and 1
   
   if (offset < 0.5f)
   {
      offset += 0.000031f; // avoid divide by zero (and overflow of 16bit int)
      ditherTotalSteps = int( 1.0f / offset + 0.5f); // add 0.5 to force correct up/down rounding
      ditherLongSteps = 1; // only one long step
   }
   else
   {
      offset = 1.000031f - offset; 
      ditherTotalSteps = int( 1.0f / offset + 0.5f);
      ditherLongSteps  = ditherTotalSteps - 1;  // only one short step
   }
   
   //Serial.print(idealTickPerStep, 2); Serial.print("\t");
   //Serial.print(int(idealTickPerStep + 0.5f)); Serial.print("\t");
   Serial.print(tickPerStep); Serial.print("\t");
   //Serial.print(ditherTotalSteps - ditherLongSteps); Serial.print("\t");
   //Serial.print(ditherLongSteps); Serial.print("\t");
   //Serial.print(float(ditherLongSteps * (tickPerStep + 1) + (ditherTotalSteps - ditherLongSteps) * tickPerStep) / float(ditherTotalSteps), 2); Serial.print("\t");
   //Serial.print(maxFeedRate); Serial.print("\t");

}


void uStepper::stepPulseOff()
{
   digitalWrite(stepPin, LOW);   // set pin low
   stepPinOn = false;

   // Dither control: interpolate speed by alternating between faster and slower "real" speeds
   if(ditherCounter > ditherLongSteps)
   {
      tickCounter = 1; // faster short step time 
      //Serial.print("[S] ");
      if(ditherCounter >= ditherTotalSteps) ditherCounter = 0;
   }
   else
   {
      tickCounter = 0; // slower long step time
      //Serial.print("[-L-] ");
   }
   ditherCounter++;   
}


void uStepper::stepPulseOn()
{
   if(moveDirection == Positive)
   {
      position++;
      digitalWrite(stepPin, HIGH);
      stepPinOn = true;
   }
   else if(moveDirection == Negative)
   {
      position--;
      digitalWrite(stepPin, HIGH);
      stepPinOn = true;
   }
   else
   {
      tickCounter = 0; // reset to avoid running repeatedly
      // no step pulse is sent if stopped
   }   
}


float uStepper::positionFloat()
{
   return float(position) * MMPerStep;
}


void uStepper::setZero()
{
   position = 0;
}


void uStepper::enable()
{
   digitalWrite(enablePin, HIGH);
   moveDirection = Stopped;
   tickPerStep = 2e9;
}


void uStepper::disable()
{
   digitalWrite(enablePin, LOW);
   moveDirection = Stopped;
   tickPerStep = 2e9;
}






