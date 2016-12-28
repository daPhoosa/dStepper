/*
      uStepper -- Simple Stepper Driver Library
      Copyright (C) 2016  Phillip J Schmidt
      
         This program is free software: you can redistribute it and/or modify
         it under the terms of the GNU General Public License as published by
         the Free Software Foundation, either version 3 of the License, or
         (at your option) any later version.
         
         This program is distributed in the hope that it will be useful,
         but WITHOUT ANY WARRANTY; without even the implied warranty of
         MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
         GNU General Public License for more details.
         
         You should have received a copy of the GNU General Public License
         along with this program.  If not, see <http://www.gnu.org/licenses/>

 */

#include "uStepper.h"



uStepper::uStepper(float _stepsPerMM, int direction, float _tickRateHz, int _stepPin, int _dirPin, int _enablePin){
   
   MMPerStep  = 1.0f / _stepsPerMM; // [mm/step]
   stepsPerMM = _stepsPerMM;        // [steps/mm]
   
   tickPerMin = _tickRateHz * 60.0f;
   maxFeedRate = int(constrain((tickPerMin * MMPerStep) / 2.0f, 0.0f, 32767.0f)); // limit to one step every 4 ticks or 16bit int max (~546mm/s)
   minFeedRate = int(tickPerMin / (65537.0f * stepsPerMM)); // prevent 16bit int overflow on very low feed rates
   
   position = 0;
   tickPerStep = 65535;
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


void uStepper::setSpeed(int feedRate){  // pass in speed [MM/min]
   
   feedRate = constrain(feedRate, -maxFeedRate, maxFeedRate);
   
   if(feedRate < -minFeedRate) // avoid silly low speeds to prevent 16bit int overflow
   {
      digitalWrite(directionPin, REVERSE);
      moveDirection = Negative;
   }
   else if(feedRate > minFeedRate)
   {
      digitalWrite(directionPin, FORWARD);
      moveDirection = Positive;
   }
   else
   {
      moveDirection = Stopped;
      tickPerStep = 65535;
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
      offset += 0.0000306f; // avoid divide by zero (and overflow of 16bit int)
      ditherTotalSteps = int( 1.0f / offset + 0.5f); // add 0.5 to force correct up/down rounding
      ditherLongSteps = 1; // only one long step
   }
   else
   {
      offset = 1.0000306f - offset; 
      ditherTotalSteps = int( 1.0f / offset + 0.5f);
      ditherLongSteps  = ditherTotalSteps - 1;  // only one short step
   }
   
   // *** Debug Output ***
   //Serial.print(idealTickPerStep, 2); Serial.print("\t");
   //Serial.print(int(idealTickPerStep + 0.5f)); Serial.print("\t");
   //Serial.print(tickPerStep); Serial.print("\t");
   //Serial.print(ditherTotalSteps - ditherLongSteps); Serial.print("\t");  // small steps
   //Serial.print(ditherLongSteps); Serial.print("\t");                     // large steps
   //Serial.print(float(ditherLongSteps * (tickPerStep + 1) + (ditherTotalSteps - ditherLongSteps) * tickPerStep) / float(ditherTotalSteps), 2); Serial.print("\t"); // actual interpolated speed
   //Serial.print(maxFeedRate/60); Serial.print("\t");
   //Serial.print(minFeedRate); Serial.print("\t");

}


void uStepper::setMinVelocity(float minVel)
{
   minFeedRate = max(int(minVel + 0.5f), int(tickPerMin / (65537.0f * stepsPerMM)) );
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
   tickPerStep = 65535;
}


void uStepper::disable()
{
   digitalWrite(enablePin, LOW);
   moveDirection = Stopped;
   tickPerStep = 65535;
}






