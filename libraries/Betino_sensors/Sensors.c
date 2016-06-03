/*
 * sensors.c
 *
 *  Created on: 21.01.2016 �.
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
#include "Sensors.h"


//-------------------------- Sensors and position -------------------------
static unsigned int sensors[SENSORS_NR] ;  //= { A1, A2, A4, A5, A6 };	//left-right



//=================  CALIBRATE ==========================
unsigned int sensor_values[SENSORS_NR];
unsigned int sensor_calmin[SENSORS_NR];
unsigned int sensor_calmax[SENSORS_NR];
unsigned int sensor_denom[SENSORS_NR];
unsigned char line_map;

//-------------- Sensors_bitmap ----------
const char sens_bitmap[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80}; 

//-------------------
void sensors_init(void) {
  int i, tmp_value;

  sensors[0] = A1; //= { A1, A2, A4, A5, A6 };
  sensors[1] = A2;
  sensors[2] = A4;
  sensors[3] = A5;
  sensors[4] = A6;

  //--- Preset the arrays ---
  for (i = 0; i < SENSORS_NR; i++)
    (sensor_calmin)[i] = 4095;		// Maximum ADC value

  for (i = 0; i < SENSORS_NR; i++) {
    (sensor_calmax)[i] = 0;			// Minimum ADC value
    tmp_value = analogRead(sensors[i]);
  }
}




//-------------------
void callibrate(void) {
  int sens;
  unsigned int tmp_value;

  //----- Find minimum and maximum values for all sensors -----
  for (sens = 0; sens < SENSORS_NR; sens++) {
    tmp_value = analogRead(sensors[sens])/2;
    if (tmp_value < sensor_calmin[sens])
      sensor_calmin[sens] = tmp_value;
    if (tmp_value > sensor_calmax[sens])
      sensor_calmax[sens] = tmp_value;
  }
}



//-------   Calculate calibration  denom --------
void calc_denom(void) {
  int sens;
  for (sens = 0; sens < SENSORS_NR; sens++) {
    sensor_denom[sens] = (sensor_calmax[sens] - sensor_calmin[sens]) / 10;

  }
}


//============================= Read sensors  and scale =====================
unsigned int read_position(void) {
  static unsigned int last_pos;
  unsigned char sens;
  unsigned int tmp_value;
  unsigned int sensors_sum;
  signed int pos;
  unsigned char on_line;    

  digitalWrite(EMITERS_ENABLE, HIGH);  // LEDS ON
  pos = 0;
  sensors_sum = 0;
  on_line = 0;
  line_map =0;
  
  delayMicroseconds(300);    // Wait for lighting
  //delay(1);

  //-------------- Read sensors ------------
  for (sens = 0; sens < SENSORS_NR; sens++) {
    tmp_value = analogRead(sensors[sens])/2;

    //--------- Validate ----------
    if (tmp_value < sensor_calmin[sens])
      tmp_value = sensor_calmin[sens];
    if (tmp_value > sensor_calmax[sens])
      tmp_value = sensor_calmax[sens];

    //-------- Calibrate ----------
    sensor_values[sens] = ((tmp_value - sensor_calmin[sens]) * 10)
      / sensor_denom[sens];

    //----------- Noise filtering ----------
    if (tmp_value < SENSOR_TRESHOLD) {
      sensor_values[sens] = 0;
    }

    // The estimate position is made using a weighted average of the sensor indices
    // multiplied by 100,  The formula is:
    // 
    //    100*value0 + 200*value1 + 300*value2 + ...
    //   --------------------------------------------
    //         value0  +  value1  +  value2 + ...

    pos += sensor_values[sens] * ((sens + 1) * 100);
    sensors_sum += sensor_values[sens];

    //--- line presens check ---
    if (sensor_values[sens] > LINE_TRESHOLD) {
      line_map += sens_bitmap[sens];
      on_line++;
    }
  }

  if (!on_line ) {
    // If it last read to the left of center, return 0.
    if (last_pos < 200) {
      last_pos = 100;
      // If it last read to the right of center, return the max.
    } 
    else if (last_pos > 400) {
      last_pos = (SENSORS_NR * 100);
    } 
    else {
      last_pos = 300;		// center pos
    }
  } 
  else {
    // calculate new position if only one sensor is online
      last_pos = pos / sensors_sum;  // else retrn last valid position
  }

  digitalWrite(EMITERS_ENABLE, LOW);  //LEDS OFF
  return last_pos;
}






