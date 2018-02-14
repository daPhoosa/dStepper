/*
      dStepper -- Simple Stepper Driver Library
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

#include "dStepper.h"


// ***************************
//     PUBLIC FUNCTIONS
// ***************************


dStepper::dStepper(float t_stepsPerMM, int t_direction, float t_tickRateHz, int t_stepPin, int t_dirPin, int t_enablePin){
   
   stepsPerMM = t_stepsPerMM;
   MMPerStep  = 1.0f / stepsPerMM;
   setTickRateHz( t_tickRateHz );

   ticksPerStep = 0;

   // Config Step Pin
   stepPin = t_stepPin;
   pinMode(stepPin, OUTPUT);
   digitalWrite(stepPin, LOW);
   stepPinOn = false;

   // Config Direction Pin
   directionPin = t_dirPin;
   pinMode(directionPin, OUTPUT);
   digitalWrite(directionPin, LOW);

   if( t_direction > 0 )
   {
      FORWARD = 1;
      REVERSE = 0;
   }
   else
   {
      FORWARD = 0;
      REVERSE = 1;      
   }

   // Config Enable Pin
   enablePin = t_enablePin;
   pinMode(enablePin, OUTPUT);
   disable();

}


dStepper::~dStepper()
{
}


void dStepper::setSpeed(float t_feedRate)    // pass in speed [mm/s]
{
   if( enabled )
   {
      feedRate = constrain( t_feedRate, -maxFeedRate, maxFeedRate );
   }
   else
   {
      feedRate = 0.0f;
   }
   
   if( feedRate < 0.0f )     // reverse
   {
      uint16_t tps = uint16_t( stepperConstant * -feedRate );

      noInterrupts();
      digitalWrite(directionPin, REVERSE);
      moveDirectionPositive = true;
      ticksPerStep = tps;
      interrupts();
   }
   else                     // forward
   {
      uint16_t tps = uint16_t( stepperConstant * feedRate );

      noInterrupts();
      digitalWrite(directionPin, FORWARD);
      moveDirectionPositive = false;
      ticksPerStep = tps;
      interrupts();
   }
}


void dStepper::setPosition(const float & posFloat)
{
   int32_t posInt;

   if( posFloat > 0.0f )
   {
      posInt = int32_t( posFloat * stepsPerMM + 0.5f );
   }
   else
   {
      posInt = int32_t( posFloat * stepsPerMM - 0.5f );
   }
   
   noInterrupts();
   position = posInt;
   interrupts();
}


void dStepper::setPosition(const int32_t & posInt)
{
   noInterrupts();
   position = posInt;
   interrupts();
}


void dStepper::setTickRateHz(const uint32_t & t_tickRateHz)
{
   tickRateHz = float(t_tickRateHz);
   maxFeedRate = 0.5f * tickRateHz * MMPerStep; // max feed rate is when sending a pulse every other tick
   stepperConstant = float(1UL << 15) / maxFeedRate;
}


float dStepper::getPositionMM()
{
   int32_t temp;
      
   noInterrupts();
   temp = position;
   interrupts();
   
   return float(temp) * MMPerStep;
}


int32_t dStepper::getPositionSteps()
{
   noInterrupts();
   int32_t temp = position;
   interrupts();
   
   return temp;
}


float dStepper::getSpeed() // return velocity in mm/s
{
   return feedRate;
}


void dStepper::enable()
{
   digitalWrite(enablePin, LOW);
   ticksPerStep = 0;
   enabled = true;
}


void dStepper::disable()
{
   digitalWrite(enablePin, HIGH);
   ticksPerStep = 0;
   enabled = false;
}

