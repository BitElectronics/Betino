/* Infrared car with Bluetooth RC Controler for Android
 club Byala Robots , eng.Anton Anchev ,2015  
 */

#include "Energia.h"
#include "pin_definitions.h"
#include "NEC_IR.h"

#define SPEED            80
#define TURN_HIGH_SPEED  60
#define TURN_LOW_SPEED    30 

// ------------------------------- SETUP ---------------------------------
void setup()
{
  pinMode(LEFT_MOTOR_PWM,OUTPUT);                               // motor speed
  pinMode(RIGHT_MOTOR_PWM,OUTPUT);
  pinMode(LEFT_MOTOR_DIR,OUTPUT);                               // посока на въртене на моторите
  pinMode(RIGHT_MOTOR_DIR,OUTPUT);
  pinMode(BEEP, OUTPUT);

  ir_init();
  Serial.begin(9600);
  Serial.println("----  Infrared remote test ----");
  tone(BEEP, 2400, 400);
}
// ------------------------------- LOOP ----------------------------------
void loop()
{
  char inp_char;
  if (ir_data_get){
    ir_data_get = 0;
    Serial.print(ir_data.address_high, HEX);
    Serial.print(" ");
    Serial.print(ir_data.address_low, HEX);
    Serial.print(" ");
    Serial.println(ir_data.command, HEX);
    inp_char = ir_data.command; 
  }
  switch (inp_char){

  case 0x46:                                       // движение напред
    FORWARD();
    break;  

  case 0x15:                                      // движение назад
    BACKWARD();
    break;  

  case 0x44:
    TURNLEFT();                                         // завой наляво
    break;

  case 0x43:
    TURNRIGHT();                                        // завой надясно
    break;

  case 0x40:
    STOP();                                             // стоп
    break;
  }
  inp_char = 0;
}
// ------------------- Движение НАПРЕД ---------------------- 
void FORWARD() 
{ 
  Serial.println("FORWARD...");
  digitalWrite(LEFT_MOTOR_DIR,HIGH);
  digitalWrite(RIGHT_MOTOR_DIR,HIGH);
  analogWrite(RIGHT_MOTOR_PWM,SPEED);
  analogWrite(LEFT_MOTOR_PWM,SPEED);
  delay(100);
}
// ------------------ Движение НАЗАД ------------------------
void BACKWARD() 
{ 
  Serial.println("BACKWARD...");
  digitalWrite(LEFT_MOTOR_DIR,LOW);
  digitalWrite(RIGHT_MOTOR_DIR,LOW);
  analogWrite(RIGHT_MOTOR_PWM,SPEED);
  analogWrite(LEFT_MOTOR_PWM,SPEED);
  delay(100);
}
// ----------------- Завой НАЛЯВО ------------------------
void TURNLEFT() 
{
  Serial.println("TURNLEFT..."); 
  digitalWrite(LEFT_MOTOR_DIR,HIGH);
  digitalWrite(RIGHT_MOTOR_DIR,HIGH);
  analogWrite(RIGHT_MOTOR_PWM,TURN_HIGH_SPEED);
  analogWrite(LEFT_MOTOR_PWM,TURN_LOW_SPEED);
  delay(100);
}
// ----------------- Завой НАДЯСНО --------------------------
void TURNRIGHT() 
{
  Serial.println("TURNRIGHT..."); 
  digitalWrite(LEFT_MOTOR_DIR,HIGH);
  digitalWrite(RIGHT_MOTOR_DIR,HIGH);
  analogWrite(RIGHT_MOTOR_PWM,TURN_LOW_SPEED);
  analogWrite(LEFT_MOTOR_PWM,TURN_HIGH_SPEED);
  delay(100);
}
// ---------------- СТОП -----------------------------------
void STOP() 
{
  Serial.println("STOP..."); 
  digitalWrite(LEFT_MOTOR_DIR,HIGH);
  digitalWrite(RIGHT_MOTOR_DIR,HIGH);
  analogWrite(RIGHT_MOTOR_PWM,0);
  analogWrite(LEFT_MOTOR_PWM,0);
  delay(100);
} 


