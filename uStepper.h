/*
 *      uStepper
 *
 *
 */
 
#ifndef uStepper_h
   #define uStepper_h
   
   #include <arduino.h>

   class uStepper
   {
      public:
      
         uStepper(float _stepsPerMM, int direction, float _tickRateHz, int _stepPin, int _dirPin, int _enablePin);
         ~uStepper();
         
         void setSpeed(int32_t feedRate);
         float positionFloat();
         void setZero();
         void enable();
         void disable();
         inline void step();  // call from ISR


      private:
      
         void stepPulseOff();
         void stepPulseOn();
         
         boolean stepPinOn;
         
         float stepsPerMM, MMPerStep;
         int directionPin, stepPin, enablePin;
         int FORWARD, REVERSE;
         
         int ditherTotalSteps, ditherLongSteps, ditherCounter;
         
         int32_t maxFeedRate;
         
         volatile int32_t  position;
          
         unsigned int tickCounter, tickPerStep;
         float tickPerMin;
         
         enum moveDir_t {
            Positive,
            Negative,
            Stopped
         } moveDirection;

   };
   
   
   // defined in header to allow "inline" declaration
   inline void uStepper::step(){  // call from ISR
      
      // This is kept fast by only executing an increment and a comparison on most calls (depending on speed)
      tickCounter++;

      if(tickCounter >= tickPerStep) // send step to motor
      { 
         // This is executed twice:
         //    * first to set the tick pin high
         //    * second to set it low (on the following call)
         //
         // This insures sufficient time for the stepper controller to see the tick, without adding an artificial delay into the ISR
      
         if(stepPinOn)    // check if pin is already high 
         { 
            stepPulseOff();
         }
         else
         {
            stepPulseOn();
         }
      }
   }

   
#endif