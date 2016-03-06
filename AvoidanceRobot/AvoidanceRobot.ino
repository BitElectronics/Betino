//=========================================================================
//
//                     Ultrasonic avoidance Robot
//                  Избягване на препятстия със завой надясно.
//            author : Vyara Simeonova  / Bit Electronics
//
//
//=========================================================================

#include "pin_definitions.h"
#include "ultrasonic.h"

int dist;
int left_motor, right_motor;

void setup()
{
  // put your setup code here, to run once:
  ultrasonic_init(TRIG, ECHO);
  Serial.begin(9600);
  pinMode (LEFT_MOTOR_DIR, OUTPUT);
  pinMode (RIGHT_MOTOR_DIR, OUTPUT);
  pinMode (LEFT_MOTOR_PWM, OUTPUT);
  pinMode (RIGHT_MOTOR_PWM, OUTPUT);
  pinMode (PUSH2, INPUT_PULLUP);
  pinMode (PUSH1, INPUT_PULLUP);


while (digitalRead (PUSH2)== HIGH);

}

void loop()
{
  // put your main code here, to run repeatedly:

  dist = us_meassure();
  //Serial.print(dist);
  //Serial.print("cm");
  //Serial.println();

  digitalWrite (LEFT_MOTOR_DIR, HIGH);
  digitalWrite (RIGHT_MOTOR_DIR, HIGH);

  left_motor= 60;
  right_motor= 60;

  if (dist<=35){
    //  Завой надясно за 185 милисекунди, ако разстоянието се увеличило
    // подължава направо. Ако разстоянието е по-малко от 35см прави още един завой за 180мс
    left_motor= left_motor*2;
    right_motor=0;
    analogWrite (LEFT_MOTOR_PWM, left_motor);
    analogWrite (RIGHT_MOTOR_PWM, right_motor);
    delay (185);
  }                             

  analogWrite (LEFT_MOTOR_PWM, left_motor);
  analogWrite (RIGHT_MOTOR_PWM, right_motor);

  delay(30);

}




