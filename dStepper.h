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

#ifndef dStepper_h
   #define dStepper_h

   #include <arduino.h>

   class dStepper
   {
      public:

         dStepper(float _stepsPerMM, int direction, float _tickRateHz, int _stepPin, int _dirPin, int _enablePin);
         ~dStepper();

         void setSpeed(float feedRate);
         void setTickRateHz(const uint32_t &  t_tickRateHz);
         void setPositionMM( float posFloat );
         void setPositionSteps(const int32_t & posInt);
         void setMinSpeed( float s );

         float getPositionMM();
         int32_t getPositionSteps();
         float getSpeed();

         void enable();
         void disable();

         inline void step();  // call from ISR


      private:

         float stepperConstant;
         float tickRateHz;
         float stepsPerMM, MMPerStep;

         float feedRate, maxFeedRate, minFeedRate;

         int directionPin, stepPin, enablePin;
         int FORWARD, REVERSE;
         bool stepPinOn;

         volatile uint16_t ticksPerStep, tickCounter;

         volatile int32_t position;

         bool enabled;

         const static float MAX_UINT_16 = powf( 2.0f, 16.0f );

         enum move_direction_t
         {
            positive,
            negative,
            stopped
         } moveDirection;

   };


   // defined in header to allow "inline" declaration
   inline void dStepper::step()     // call from ISR
   {
      uint16_t prev = tickCounter;

      if( stepPinOn )
      {
         digitalWrite( stepPin, LOW );   // set pin low
         stepPinOn = false;
      }

      if( moveDirection == positive )     // POSITIVE
      {
         tickCounter += ticksPerStep;
         if( tickCounter < prev )
         {
            digitalWrite( stepPin, HIGH );
            stepPinOn = true;
            position++;
         }
      }
      else if( moveDirection == negative )// NEGATIVE
      {
         tickCounter -= ticksPerStep;
         if( tickCounter > prev )
         {
            digitalWrite( stepPin, HIGH );
            stepPinOn = true;
            position--;
         }
      }
   }


#endif