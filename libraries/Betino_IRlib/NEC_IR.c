/*
 * NEC infrared receiver library
 *
 *  Created on: 21.10.2015 �.
 *      Author: Simo
 */
 
#include "Energia.h"
#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_timer.h"

#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"

#include "pin_definitions.h"
#include "NEC_IR.h"


/*=====================================================================*/
/* HW-dependent stuff. Change these macros if your hardware differs. */
/*
 * Timer0 A and EINT0 (PE_0)
 */

#define IR_FALLING_INT()	ROM_GPIOIntTypeSet(IR_PORT, IR_PIN, GPIO_FALLING_EDGE);
#define IR_RISING_INT()		ROM_GPIOIntTypeSet(IR_PORT, IR_PIN, GPIO_RISING_EDGE);

#define IR_COUNTER_CLOCK	(ROM_SysCtlClockGet() / 2UL)

#define IR_GET_EDGE()		(ROM_GPIOIntTypeGet(IR_PORT, IR_PIN) == GPIO_RISING_EDGE)
#define IR_EDGE_LOW			!IR_GET_EDGE
#define IR_EDGE_HIGH		IR_GET_EDGE)

/* ===================================================================== */

unsigned long g_ulPeriod;
unsigned long ulCountsPerMicrosecond;
unsigned long gulTicks;
unsigned char ir_data_get;

//------------------------------------ Timer init ---------------------------------
void ir_timer_init(void) {

  // The Timer0 peripheral must be enabled for use.
  //
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

  //
  // The Timer0 peripheral must be enabled for use.
  // When  configured for a pair of half-width timers, each timer is separately configured.
  //
  ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC | TIMER_CFG_B_ONE_SHOT);

  // Calculate the number of timer counts/microsecond
  ulCountsPerMicrosecond = ROM_SysCtlClockGet() / 10000;

  // 0.10ms = timeout delay
  ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ulCountsPerMicrosecond);

  ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

  TimerIntRegister(TIMER0_BASE, TIMER_A, Timer0AIntHandler );

  //
  // Configure the Timer0 interrupt for timer timeout.
  //
  ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

  //
  // Enable the Timer0 interrupt on the processor (NVIC).
  //
  ROM_IntEnable(INT_TIMER0A);

  gulTicks = 0;

  //
  // Enable Timer0A.
  //
  ROM_TimerEnable(TIMER0_BASE, TIMER_A);

}

//-------------------------- IR pint init ----------------------------------
void ir_pin_init(void) {

  ROM_GPIOPinTypeGPIOInput(IR_PORT, IR_PIN);
  ROM_GPIOPadConfigSet( IR_PORT, IR_PIN, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
  //
  //  Register  the port-level   interrupt  handler.  This  handler  is  the
  //  first  level interrupt   handler  for all  the pin  interrupts.
  //
  GPIOIntRegister(IR_PORT, PortEIntHandler);		// PORT B

  //
  //  Make pin rising edge triggered  interrupt.
  //
  ROM_GPIOIntTypeSet(IR_PORT, IR_PIN, GPIO_FALLING_EDGE);	// GPIO_HIGH_LEVEL  GPIO_RISING_EDGE    GPIO_FALLING_EDGE

    GPIOIntClear(IR_PORT, IR_PIN);
  //
  //  Enable the pin interrupts.
  //
  GPIOIntEnable(IR_PORT, IR_PIN);

}

// -------------------------- END HW-stuff --------------------------- //

/*
 * Protocol definitions
 */
#define ADDR_LEN	16
#define CMD_LEN		16

#define IR_CNT_GET		gulTicks
#define IR_CNT_SET(ticks)	gulTicks = ticks

/*
 * Interrupt stuff
 */
#define	IR_STARTBIT1	0
#define	IR_STARTBIT2	1
#define	IR_ADDRL		2
#define	IR_ADDRH		3
#define	IR_CMD			4
#define	IR_CMDN			5

/*
 * Variables
 */
volatile static int8_t status;
volatile static uint8_t bits;
volatile IRDATA ir_data;
volatile unsigned long *ir_command;

//-------------------------------------------------------------------------------
void ir_init(void) {

  /*
	 * Variables
   	 */
  status = IR_STARTBIT1;
  ir_data_get = 0;
  bits = 0;
  ir_command = 0;

  ir_timer_init();
  ir_pin_init();
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
void PortEIntHandler(void) {
  unsigned long period;
  static unsigned char edge = 0;

  GPIOIntClear(IR_PORT, IR_PIN);

  period = IR_CNT_GET;
  IR_CNT_SET(0);

  switch (status) {
  case IR_STARTBIT1:
    // Workaround to interrupt on any edge
    if (!edge) {
      IR_RISING_INT();
      edge = 1;
    } 
    else {
      IR_FALLING_INT();
      if ((period >= 85) && (period < 95) && !ir_data_get)
        status = IR_STARTBIT2;
      edge = 0;
    }
    break;

  case IR_STARTBIT2:
    if ((period > 40) && (period < 50)) {
      status = IR_CMD;
      //IR_FALLING_INT();
      edge = 0;
      ir_command = (unsigned long*) &ir_data;
      *ir_command = 0;
      //ir_data_get = 0;
      bits = 0;
    } 
    else {
      status = IR_STARTBIT1;
      return;
    }
    break;

  case IR_CMD:
    if ((bits < 32) && (period < 30)) {
      ir_command = (unsigned long*) &ir_data;
      if ((period > 10) && (period < 15)) {
        *ir_command >>= 1;
      } 
      else if (period > 16) {
        *ir_command >>= 1;
        *ir_command |= 0x80000000;
      }
      bits++;
    }
    if (bits >= 32) {               // AddrH + addrL + cmd + cmdN = 32 bit
      ir_data_get = 1;
      status = IR_STARTBIT1;
      return;
    }

    if (period > 30) {
      status = IR_STARTBIT1;
      return;
    }

    break;

  }
}

//==================  Timer 0 A ===================================
void Timer0AIntHandler(void) {
  //
  // Clear the timer interrupt flag.
  //
  ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

  gulTicks++;

  if (gulTicks > 110) {     //   Timeout
    status = IR_STARTBIT1;
    // Reset to Falling Edge
    ROM_GPIOIntTypeSet(IR_PORT, IR_PIN, GPIO_FALLING_EDGE);
  }
}



