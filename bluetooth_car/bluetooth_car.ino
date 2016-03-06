/* Bluetooth car with Bluetooth RC Controler for Android
 club Byala Robots , eng.Anton Anchev ,2015   
  Работи с приложение на Андроид за смартфон или таблет:
https://play.google.com/store/apps/details?id=braulio.calle.bluetoothRCcontroller

*/

#include "pin_definitions.h"
char moving = 0;

// ========================== SETUP =============================
void setup()
{
  Serial.begin(9600);                                           // серийна комуникация
  Serial4.begin(9600);                                          // Bluetooth комуникация
  pinMode(LEFT_MOTOR_PWM,OUTPUT);                               // motor speed
  pinMode(RIGHT_MOTOR_PWM,OUTPUT);
  pinMode(LEFT_MOTOR_DIR,OUTPUT);                               // посока на въртене на моторите
  pinMode(RIGHT_MOTOR_DIR,OUTPUT);
  pinMode(BEEP, OUTPUT);
}

// -------------------------- LOOP ---------------------------
void loop()
{
  char inp_char;
  // Има ли получен символ от сериен канал 4 ( Bluetooth модула) 
  if (Serial4.available()>0) {
	
    inp_char = Serial4.read();
    Serial.write(inp_char);                                    // Ехо на получения символ на серийния монитор
  // Реакция на робота според получения символ
    switch (inp_char)
    {    
    case  'F':                                                 // движение напред
      digitalWrite(LEFT_MOTOR_DIR,HIGH);
      digitalWrite(RIGHT_MOTOR_DIR,HIGH);
      analogWrite(RIGHT_MOTOR_PWM,110);
      analogWrite(LEFT_MOTOR_PWM,110);
      delay(100);
      break; 
    case  'G':                                                 // движение напред и наляво / Диагонал
      digitalWrite(LEFT_MOTOR_DIR,HIGH);
      digitalWrite(RIGHT_MOTOR_DIR,HIGH);
      analogWrite(RIGHT_MOTOR_PWM,110);
      analogWrite(LEFT_MOTOR_PWM,75);
      delay(100);
      break;
    case  'I':                                                 // движение напред и надясно / Диагонал
      digitalWrite(LEFT_MOTOR_DIR,HIGH);
      digitalWrite(RIGHT_MOTOR_DIR,HIGH);
      analogWrite(RIGHT_MOTOR_PWM,75);
      analogWrite(LEFT_MOTOR_PWM,110);
      delay(100);
      break;    
    case  'B':                                                 // движение назад
      digitalWrite(LEFT_MOTOR_DIR,LOW);
      digitalWrite(RIGHT_MOTOR_DIR,LOW);
      analogWrite(RIGHT_MOTOR_PWM,110);
      analogWrite(LEFT_MOTOR_PWM,110);
      delay(100);    
      break;
    case  'H':                                                 // движение назад и наляво / Диагонал
      digitalWrite(LEFT_MOTOR_DIR,LOW);
      digitalWrite(RIGHT_MOTOR_DIR,LOW);
      analogWrite(RIGHT_MOTOR_PWM,110);
      analogWrite(LEFT_MOTOR_PWM,75);
      delay(100);    
      break;
    case  'J':                                                 // движение назад и надясно / Диагонал
      digitalWrite(LEFT_MOTOR_DIR,LOW);
      digitalWrite(RIGHT_MOTOR_DIR,LOW);
      analogWrite(RIGHT_MOTOR_PWM,75);
      analogWrite(LEFT_MOTOR_PWM,110);
      delay(100);    
      break;
    case  'R':                                                 // завой надясно
      digitalWrite(LEFT_MOTOR_DIR,HIGH);
      digitalWrite(RIGHT_MOTOR_DIR,HIGH);
      analogWrite(RIGHT_MOTOR_PWM,50);
      analogWrite(LEFT_MOTOR_PWM,110);
      delay(100);    
      break;
    case  'L':                                                 // завой наляво
      digitalWrite(LEFT_MOTOR_DIR,HIGH);
      digitalWrite(RIGHT_MOTOR_DIR,HIGH);
      analogWrite(RIGHT_MOTOR_PWM,110);
      analogWrite(LEFT_MOTOR_PWM,50);
      delay(100);    
      break;
    case  'S':                                                 // спиране
      digitalWrite(LEFT_MOTOR_DIR,LOW);
      digitalWrite(RIGHT_MOTOR_DIR,LOW);
      analogWrite(RIGHT_MOTOR_PWM,0);
      analogWrite(LEFT_MOTOR_PWM,0);
      delay(100);    
      break;
    }  // END Switch
  }  // END IF
}  //END LOOP

















