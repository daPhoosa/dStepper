# **dStepper**
Simple Dithering Stepper Driver Library

* Designed for interupt driven stepping
* Pulse dithering for more accurate speed control and smoother high speed operation
* Efficient functions will perform well with either 8 bit or 32 bit microcontrollers
* Works with almost any stepper driver that accepts step and direction input (see: stepStick)
* Handles setup and control of hardware pins 
* Acceleration ramping and path planning are not performed by this library ( see `EasyMove` for basic 3 axis motion control )

## Basic Usage Documentation

### Object Creation:
```
dStepper your_motor( stepsPerMM, direction, tickRateHz, stepPin, directionPin, enablePin);
```
* stepsPerMM -- usually defined by motor steps, pully size and micro-stepping ( A 200 step motor, with 20T GT2 pully with 1/16 microstepping will have 80 steps per mm)
* direction -- changing between 0 and 1 will change the direction of rotation
* tickRateHz -- how often is the step() function called per second
* stepPin -- which hardware pin is used to send the step pulse
* directionPin -- which hardware pin is used to send teh direction signal
* enablePin -- which hardware pin us used to enable/disable the stepper driver

### Enable/Disable Stepper
```
your_motor.enable();
your_motor.disable();
```

### Set Motor Speed:
```
your_motor.setSpeed( speed );
```
* Speed is in MM per min
* Negative speed values will reverse motor direction
* The direction pin is controlled from this function
* Should be called after enable() has been called
* Will operate at the request speed until called again with a different speed
```
your_motor.setSpeedByPositionMM( targetPositionMM );
your_motor.setSpeedByPositionMM( targetPositionMM, Hz );
```
* Pass in the target position at regular intervals and this function will set the motor speed to minimize error
* The target position must move smoothly to prevent unreasonable motor speed requests
* The starting motor position should be set to starting motion control position using `setPositionMM()`
* Using the second option and passing the expected motion control frequency saves ~40us per motor compared to first option which computes it each execution


### Set Motor Position:
```
your_motor.setPositionMM( position );
```
* Passing a float will set the position in MM
```
your_motor.setPositionSteps( position );
```
* Passing an integer will set the position in steps

### Get Motor Position:
```
float position = your_motor.getPositionMM();       
uint32_t position = your_motor.getPositionSteps(); 
```
* MM are returned as a 32bit float
* Steps are return as an unsigned 32 bit int


### Sending Step Pulses
```
your_motor.step();
```
* A step is only output when needed.  If the speed is set to zero, no pulses will be sent.
* This should be run from an interrupt for best performance.  For the less experienced user, a library that can make this easier is: FrequencyTimer2 https://github.com/PaulStoffregen/FrequencyTimer2
* The call rate should match the rate specified at object creation (tickRateHz)
* Jitter from imperfect call rates will not cause adverse software function but may affect motor function/smoothness
* This has very low computational requrement.


