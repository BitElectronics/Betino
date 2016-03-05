/*
 Author : Simeon Simeonov
 Robotics Club Byala 
 Name : Motors test
 Ver: 1.0.0
 */
#include "Energia.h"

#include "pin_definitions.h"
#include "Motors.h"

//-----------------------------  Motors -----------------------------------
Motor left_motor(50000, LEFT_MOTOR_PWM, LEFT_MOTOR_DIR);
Motor right_motor(50000,RIGHT_MOTOR_PWM, RIGHT_MOTOR_DIR);


#include <stdio.h> // for function sprintf

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

  while (digitalRead(PUSH2) == 1)
    ;
    
  //--- Forward ---
  left_motor.speed(60);
  right_motor.speed(60);
  Serial.println("FORWARD");
  delay(600);  // simple debouncing
}


void loop()
{
  // put your main code here, to run repeatedly:
  
  while (digitalRead(PUSH2) == 1)
    ;
  //--- Turn right ---
  left_motor.speed(60);		// forward
  right_motor.speed(-60);		// backward
  Serial.println("Turn RIGHT");
  delay(600);
  
  while (digitalRead(PUSH2) == 1)
    ;
  //--- Turn left ---
  left_motor.speed(-60);		// backward
  right_motor.speed(60);		// forward 
  Serial.println("Turn LEFT");
  delay(600);
  
  while (digitalRead(PUSH2) == 1)
    ;
  //--- Backward ---
  left_motor.speed(-60);		// backward
  right_motor.speed(-60);		// backward 
  Serial.println("BACKWARD");
  delay(600);

}

