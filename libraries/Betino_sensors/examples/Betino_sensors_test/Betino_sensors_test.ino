/*
 Author : Simeon Simeonov
 Robotics Club Byala 
 Name : Sensors test
 Ver: 1.0.0
 */

#include "Energia.h"

#include "pin_definitions.h"
#include "Sensors.h"

#include <stdio.h> // for function sprintf

const char sensors[5] = { 
  A1, A2, A4, A5, A6 };	//left-right

//-------------------------------------------------------------------
void setup()
{
  // put your setup code here, to run once:

  pinMode(EMITERS_ENABLE, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  pinMode(PUSH1, INPUT_PULLUP);
  pinMode(PUSH2, INPUT_PULLUP);

  digitalWrite(EMITERS_ENABLE, LOW);

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  tone(BEEP, 2400, 400);

  sensors_init();

  while (digitalRead(PUSH2) == 1)
    ;
    delay(1000);
}

//-------------------------------------------------------------------
void loop()
{
  int i;  
  int analog_value;
  int tmp_value;
  int error;
  
  // put your main code here, to run repeatedly:
  while (digitalRead(PUSH2) == 1) {
    for (i=0; i<5; i++) {
      analog_value = analogRead(sensors[i]); 
      Serial.print(analog_value);
      Serial.print("  ");

    }
    Serial.println(" ");
    delay(200);
  }   


  //------------------------ Start of callibration ------------------------
  // --- Move black line under the senosrs ---
  Serial.println(" Move black line under the senosrs ");
  delay(1000);
  
  sensors_init();

  digitalWrite(EMITERS_ENABLE, HIGH);  //OPTRON LEDS ON
  delay(1);

  for (i=0; i<2000; i++) {
    callibrate();
    delay(2);
  }

  calc_denom();
  digitalWrite(EMITERS_ENABLE, LOW);  //OPTRON LEDS Off
  //-------------------------------- End of callibration   -----------------------
  //-----  Display current position and error ---
  while (digitalRead(PUSH2) == 1) {
    tmp_value = read_position();
    error = tmp_value - 300;
    Serial.print(tmp_value);
    Serial.print("   ");
    Serial.print(error); 
    Serial.print("   ");
    Serial.println(line_map, HEX); 
    delay(150);
  }


}



