/*
Copyright (c) Simeon Simeonov
 Author : Simeon Simeonov
 Name : Motor PWM driver
 Ver: 1.0.0

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// ensure this library description is only included once
#ifndef MOTOR_H
#define MOTOR_H

// library interface description
class Motor {
public:
  // constructors:
  Motor(int pwm_frequency, int motor_pwm_pin, int motor_dir_pin);

  // speed setter method ( PWM duty set) :
  void speed(signed int motor_speed);

  //
  void setFrequency(long whatFreq);

private:
  int frequency;
  int direction;        // Direction of rotation
  int duty;          // Motor Speed 
  int periodPWM;
  int timerBase;
  int timerAB;
  int analog_res;

  // motor ports & pin numbers:
  int motor_pwm_pin;
  int motor_dir_pin;
  int motor_dir_port;

  bool initialized;
};

#endif



