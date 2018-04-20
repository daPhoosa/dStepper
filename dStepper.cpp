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
   setMinSpeed( 0.0f );
   setPositionSteps( 0L );

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
   if( !enabled ||                     // not enabled
       abs(t_feedRate) < minFeedRate ) // feed rate below minimum
   {
      feedRate      = 0.0f;
      ticksPerStep  = 0;
      moveDirection = stopped;
      return;
   }

   if( t_feedRate < 0.0f )     // REVERSE
   {
      feedRate     = max( t_feedRate, -maxFeedRate );  // constrain
      uint16_t tps = uint16_t( stepperConstant * -feedRate );
      //Serial.print(feedRate); Serial.print("\t"); Serial.println(tps);

      noInterrupts();
      if( moveDirection != negative ) // only set when a direction change happens, saves time
      {
         digitalWrite(directionPin, REVERSE);
         moveDirection = negative;
      }
      ticksPerStep = tps;
      interrupts();
   }
   else                      // FORWARD
   {
      feedRate     = min( t_feedRate, maxFeedRate );  // constrain
      uint16_t tps = uint16_t( stepperConstant * feedRate );
      //Serial.print(feedRate); Serial.print("\t"); Serial.println(tps);

      noInterrupts();
      if( moveDirection != positive ) // only set when a direction change happens, saves time
      {
         digitalWrite(directionPin, FORWARD);
         moveDirection = positive;
      }
      ticksPerStep = tps;
      interrupts();
   }
}


void dStepper::setSpeedByPostionMM( float targetPosMM )
{
   uint32_t timeNow = micros();
   float Hz = 1000000.0f / float( timeNow - lastUpdateTime ); // compute observed frequency
   lastUpdateTime = timeNow;

   setSpeedByPostionMM( targetPosMM, Hz );
}


void dStepper::setSpeedByPostionMM( float targetPosMM, float Hz )
{
   float newSpeed = ( targetPosMM + targetPosMM - targetPosPrev - getPositionMM() ) * Hz; //  = speed + error

   setSpeed( newSpeed );

   targetPosPrev  = targetPosMM;
}


void dStepper::setPositionMM( float posFloat )
{
   posFloat *= stepsPerMM; // convert to steps

   int32_t posInt = posFloat;      // whole steps
   if( posFloat < 0.0f ) posInt--;

   uint16_t posFrac = (posFloat - float(posInt)) * MAX_UINT_16; // fractional step

   noInterrupts();
   position    = posInt;
   tickCounter = posFrac;
   interrupts();
}


void dStepper::setPositionSteps(const int32_t & posInt)
{
   noInterrupts();
   position    = posInt;
   tickCounter = 0; // set to start of step
   interrupts();
}


void dStepper::setTickRateHz(const uint32_t & t_tickRateHz)
{
   tickRateHz = float(t_tickRateHz);
   maxFeedRate = 0.5f * tickRateHz * MMPerStep; // max feed rate is when sending a pulse every other tick
   stepperConstant = powf( 2.0f, 15.0f ) / maxFeedRate;
}


void dStepper::setMinSpeed( float s )
{
   minFeedRate = max( s, 0.0f );
}


float dStepper::getPositionMM()
{
   noInterrupts();
   int32_t  fullStep = position;
   uint16_t fracStep = tickCounter;
   interrupts();

   return ( float(fullStep) + float(fracStep) * (1.0f / MAX_UINT_16) ) * MMPerStep;
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

