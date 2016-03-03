/*
 Author : Simeon Simeonov
 Name : Motor PWM driver
 Ver: 1.0.0
 */

#include "Energia.h"

#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_timer.h"
#include "inc/hw_ints.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/timer.h"

#include "wiring_private.h"			// PWM_Write

#include "Motors.h"

#define PWM_MODE 0x20A

/*
 * Motor constructor - frequency, PWM pin and Dir pin.
 * Sets which wires should control the motor.
 */
Motor::Motor(int pwm_frequency, int motor_pwm_pin, int motor_dir_pin) {
  this->frequency = pwm_frequency;        // the PWM frequency
  this->direction = 0;        // motor direction
  this->duty = 1;            // pwm duty mimimun is 1. Strop the motor
  this->analog_res = 256;    // One byte resolution : 0 - 255

  // Tiva Launchpad pins for the motor control connection:
  this->motor_pwm_pin = motor_pwm_pin;
  this->motor_dir_pin = motor_dir_pin;

  //-------------- setup the pins on the microcontroller: ------------
  pinMode(this->motor_dir_pin, OUTPUT);
  uint8_t bit = digitalPinToBitMask(this->motor_dir_pin); // get pin bit
  uint8_t port = digitalPinToPort(this->motor_dir_pin);   // get pin port
  this->motor_dir_port =  (uint32_t) portBASERegister(port);
  this->motor_dir_pin = bit;

  //----------------------------------------------------------

  bit = digitalPinToBitMask(this->motor_pwm_pin); // get pin bit
  port = digitalPinToPort(this->motor_pwm_pin);   // get pin port
  uint8_t timer = digitalPinToTimer(this->motor_pwm_pin);
  uint32_t portBase = (uint32_t) portBASERegister(port);
  uint32_t offset = timerToOffset(timer);

  this->timerBase = getTimerBase(offset);
  this->timerAB = TIMER_A << timerToAB(timer);

  if (port == NOT_A_PORT)
    return; 	// pin on timer?

#ifdef __TM4C1294NCPDT__
  this->periodPWM = F_CPU/this->frequency;
#else
  this->periodPWM = SysCtlClockGet() / this->frequency;
#endif

  if (offset > TIMER3) {
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER0 + offset - 4);
  } 
  else {
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0 + offset);
  }

  ROM_GPIOPinConfigure(timerToPinConfig(timer));
  ROM_GPIOPinTypeTimer((long unsigned int) portBase, bit);

  //
  // Configure for half-width mode, allowing timers A and B to
  // operate independently
  //
  HWREG(this->timerBase + TIMER_O_CFG) = 0x04;

  if (this->timerAB == TIMER_A) {
    HWREG(this->timerBase + TIMER_O_CTL) &= ~TIMER_CTL_TAEN;
    HWREG(this->timerBase + TIMER_O_TAMR) = PWM_MODE;
  } 
  else {
    HWREG(this->timerBase + TIMER_O_CTL) &= ~TIMER_CTL_TBEN;
    HWREG(this->timerBase + TIMER_O_TBMR) = PWM_MODE;
  }
  ROM_TimerLoadSet(this->timerBase, this->timerAB, this->periodPWM);
  ROM_TimerMatchSet(this->timerBase, this->timerAB,(this->analog_res - this->duty) * this->periodPWM / this->analog_res);

  ROM_TimerEnable(this->timerBase, this->timerAB);

  this->initialized = true;
}

/*
 Sets the PWM frequency  in Hz
 
 */
void Motor::setFrequency(long whatFreq) {
#ifdef __TM4C1294NCPDT__
  this->periodPWM = F_CPU/this->frequency;
#else
  this->periodPWM = SysCtlClockGet() / this->frequency;
#endif
}

/*
    Two way PWM speed control. If speed is >0 then motor runs forward.
    Else if speed is < 0 then motor runs forward
 */
void Motor::speed(signed int motor_speed) {
  if (motor_speed == 0)
    motor_speed = 1;

  if (motor_speed > 0) {
    ROM_GPIOPinWrite(this->motor_dir_port, this->motor_dir_pin,	this->motor_dir_pin);
  } 
  else {
    ROM_GPIOPinWrite(this->motor_dir_port, this->motor_dir_pin, 0);
  }

  motor_speed = abs(motor_speed);

  if (motor_speed > this->analog_res)
    motor_speed = this->analog_res - 1;

  this->duty = motor_speed;

  ROM_TimerLoadSet(this->timerBase, this->timerAB, this->periodPWM);
  ROM_TimerMatchSet(this->timerBase, this->timerAB,
  (this->analog_res - this->duty) * this->periodPWM
    / this->analog_res);
}




