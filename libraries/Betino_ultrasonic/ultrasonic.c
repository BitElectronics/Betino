//--------------------------------------------------------------
//
//                   Ultrasonic library
//		      standard ANSI C
//            author : Simeon Simeonov  / Bit Electronics
//---------------------------------------------------------------
#include "Energia.h"

//--- Променливи за конфигурация на пиновете ---
static int trig, echo;

//-------------------------------------------------------------------------
//   Инициализираща функция - конфигурира пиновете свързани към
//   Ultrasonic  сензора :  trig - изход,  echo - вход
//
//-------------------------------------------------------------------------
void ultrasonic_init(int trig_pin, int echo_pin) {
  	trig = trig_pin;
	echo = echo_pin;
	pinMode( trig, OUTPUT);
	pinMode(echo, INPUT);
}


// ------------------------------------------------------------------------
//      result is in cm
//
//-------------------------------------------------------------------------
int us_meassure(void) {
  int dist;
  int duration;
  
  // Изходът trig  се вдига във високо ниво HIGH за 2 и повече микросекунди
  digitalWrite(trig, HIGH);
  delayMicroseconds(5);
  digitalWrite(trig, LOW);

  // Измерва се продулжителността на импулса с високо ниво на пин echo.
  duration = pulseIn(echo, HIGH, 32768);

  // Преобразуване на измерения импулс в разстояние в сантиметри
  dist = duration / 29 / 2;
  return dist; 
}


