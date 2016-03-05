/*
 Author : Simeon Simeonov / bitElectronics ltd.
 Robotics Club Byala 
 Name : Robot  Energia project
 Ver: 1.0.0
 */

#include "Energia.h"

#include "pin_definitions.h"
#include "pitches.h"
#include "Motors.h"
#include "Sensors.h"

#include <stdio.h> // for function sprintf


//--------- Directions ( states )--------
#define FORWARD    1
#define LEFT       2
#define RIGHT      3
#define BACK       4

///////////////////////////////////////////////////////////////////////////
//----------------------------------- PID ---------------------------------
///////////////////////////////////////////////////////////////////////////
#define PWM_FREQ		50000
#define KP             0.6        // Пропорционален коефициент - определя разликата на моторите при завой
#define KD             10         // Диференциялен коефициент. Ускорява реакцията на робота при завой
#define MAX_SPEED      100        // Скоростта в права посока
#define ACQUIRE_SPEED  45         // Скорост за калибриране - роботът се движи наляво и надясно с малка скорост
#define SLOW_SPEED     MAX_SPEED - MAX_SPEED/4  // За стабилизация след завой

// Скоростите на двата мотора при завой - вътрешния мотор е смалката скорот, а външния с голяма, подходяща да завие без да поднесе
// Two speed for turn - for making right turn radius
#define TURN_SPEED_HIGH     MAX_SPEED + MAX_SPEED/4    
#define TURN_SPEED_LOW      1

// Граници при които се смята, че роботът вече изпуска линията
// Left or right position treshold
#define TURN_ERROR_LEFT		300 - 190    
#define TURN_ERROR_RIGHT	300 + 190

/////////////////////////////////////////////////////////////////////////

//-----------------------------  Motors -----------------------------------
Motor left_motor(50000, LEFT_MOTOR_PWM, LEFT_MOTOR_DIR);
Motor right_motor(50000,RIGHT_MOTOR_PWM, RIGHT_MOTOR_DIR);
//---------

//------- Global Variables -----
signed long position;
int dir;

signed int error;
signed int lastError;
signed int last_position;
signed int left_pwm;
signed int right_pwm;
signed int motorSpeed;

//--------- Common ------------
char tmp_str[64];

//--------------------------------
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4 };
// note durations: 4 = quarter note, 8 = eighth note, etc.: 
int noteDurations[] = { 
  4, 8, 8, 4, 4, 4, 4, 4 };

//////////////////////////////////////////////////////////////////////////////

// the setup routine runs once when you press reset:
void setup() {
  int i, tmp_value, center_pos;

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

  // Чака натискането на бутон за започване на калибрирането
  while (digitalRead(PUSH2) == 1)
    ;
  digitalWrite(EMITERS_ENABLE, HIGH);
  delay(1);

  //------------------------ Start of callibration ------------------------
  sensors_init();

  digitalWrite(EMITERS_ENABLE, HIGH);  // LEDS ON
  delay(1);

  //--- Turn right ---
  left_motor.speed(ACQUIRE_SPEED);		// forward
  right_motor.speed(-ACQUIRE_SPEED);		// backward

  for (i=0; i<600; i++) {
    callibrate();
    delay(1);
  }

  // --- turn left  ---
  left_motor.speed(-ACQUIRE_SPEED);
  right_motor.speed(ACQUIRE_SPEED);
  for (i=0; i<1200; i++) {
    callibrate();
    delay(1);
  }

  calc_denom();
  //-------------------------------- End of callibration   -----------------------

  //----- Go back to the line -----
  left_motor.speed(ACQUIRE_SPEED);
  right_motor.speed(-ACQUIRE_SPEED);
  do {
    tmp_value = read_position();
    delay(5);
  } 
  while (sensor_values[2] < 60);    // Center sensor value

  digitalWrite(EMITERS_ENABLE, LOW);  //LEDS OFF
  // --- Stop the motors ---
  left_motor.speed(0);
  right_motor.speed(0);

  dir = FORWARD;
  lastError = 0;
  
  
  // За тестове - показват се намерените минимални и максимални стойности на сензорите
 // Намерени по време на калибрацията. На базата на тези стойности пресмятат калибриращи коефициенти
// които калибрират показанията на всеки сензор в обхвата - 0 - 100% 
 //-- Display minimun and maximum meassured values --
  for (i=0; i<5; i++) {
    Serial.print(sensor_calmin[i]);
    Serial.print(" <<< ");
    Serial.println(sensor_calmax[i]);
  }
  
  //-------------------------------------------------------------------

  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 8; thisNote++) {
    // to calculate the note duration, take one second
    // divided by the note type. //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(BEEP, melody[thisNote], noteDuration);
    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(BEEP);
  }


  while (digitalRead(PUSH2) == 1)
    ;
}


//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
// the loop routine runs over and over again forever:
void loop() {

  //static int tmp_value;

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);

  //---- Read current position --
  position = read_position();
  error = position - 300;
  last_position = position;

  if (line_map == 0x01F) {
    tone(BEEP, 2500, 300);  
  }
  
  
  //sprintf(tmp_str,"position  %d  error=%d", position, error);
  //Serial.println(tmp_str);

  //  Според състоянието на робота в момента - права линия или завой, определяме реакцията.
  switch (dir) {
  case FORWARD:      // Движение направо
  default:
  
    // Ако отклонението е много голямо преминаваме в състояние - завой наляво
    if ((position < TURN_ERROR_LEFT)) {
      digitalWrite (LED1, HIGH);
      right_motor.speed( TURN_SPEED_HIGH);
      left_motor.speed(TURN_SPEED_LOW);
      dir = LEFT;
      //delay(1);
      //Serial.println("LEFT <<<<<<<<<<<<<<<<<<");
      break;
    }

    // Ако отклонението е много голямо преминаваме в състояние - завой надясно
    if (position > TURN_ERROR_RIGHT) {
      digitalWrite(LED3, HIGH);
      right_motor.speed(TURN_SPEED_LOW);
      left_motor.speed(TURN_SPEED_HIGH);
      dir = RIGHT;
      //delay(1);
      //Serial.println(">>>>>>>>>>>>>>>>>> RIGHT");
      break;
    }

    digitalWrite(LED2, HIGH);    // Движение направо
    //--------- PD ------------
    motorSpeed = KP * error + KD * (error - lastError);
    lastError = error;

    left_pwm = MAX_SPEED + motorSpeed;
    right_pwm = MAX_SPEED - motorSpeed;

    //------------ Set motor speed -------
    left_motor.speed(left_pwm);
    right_motor.speed(right_pwm);
    break;

  case LEFT:
    digitalWrite(LED1, HIGH);
    if (position > TURN_ERROR_LEFT+20) {           // Завръщане обратно на линията .Maximum is 100%
      // Спирачен въртящ момент за предотвраряване на заклащането след завой
      left_motor.speed(TURN_SPEED_HIGH);
      right_motor.speed(SLOW_SPEED);

      dir = FORWARD;
      lastError = 0;
      last_position = 0;
      delay(6);            // Време на спирачния момент
      //Serial.println( "LEFT >>>>  FORWARD");
    }
    break;
  case RIGHT:
    digitalWrite(LED2, HIGH);
    if (position < TURN_ERROR_RIGHT-20) {         // Завръщане обратно на линията .Maximum is 100%
     // Спирачен въртящ момент за предотвраряване на заклащането след завой
      left_motor.speed(SLOW_SPEED);
      right_motor.speed(TURN_SPEED_HIGH);

      dir = FORWARD;
      lastError = 0;
      last_position = 0;
      delay(6);
      //Serial.println( "FORWARD  <<<<<<  RIGHT");
    }
    break;

  }

  //sprintf(tmp_str," error %d - speed %d",error, motorSpeed);
  //sprintf(tmp_str,"DIR %d /  l %d - R %d",dir, left_pwm, right_pwm);
  //sprintf(tmp_str,"Edges : %d   %d", left_edge, right_edge);
  //Serial.println(tmp_str);

  // wait for 3 milliseconds 
  delay(3);
}











