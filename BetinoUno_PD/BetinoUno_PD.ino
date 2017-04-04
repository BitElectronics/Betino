/*
  Author : Simeon Simeonov
  Name : Bolid  Arduino project
  Ver: 1.0.0
*/
#include <Arduino.h>
#include "pitches.h"
#include <stdio.h> // for function sprintf

//--------------------- PIN definitions -----------

#define RIGHT_EDGE  A0
#define RIGHT_POS   A1
#define CENTER_POS  A2
#define LEFT_POS    A3
#define LEFT_EDGE   A4

#define LEFT_PWM    11
#define RIGHT_PWM   10
#define LEFT_DIR    9
#define RIGHT_DIR   8


//--------------- LEDS -----------
#define GREEN_LED  4      // LED1
#define RED_LED    7      // LED2

//-------------- Optrons ---------
#define OPT_ENABLE    6


//-------------- Beep ----------
#define BEEP        3

//--------------  Buttons -----------
#define BUTT1     2


//------------ IR ------------
#define IR_PIN    5

//-------------------------- Sensors and position -------------------------
#define SENSORS_NR  5
const unsigned int sensors[SENSORS_NR] = { A0, A1, A2, A3, A4 };  //left-right

//---------------- Състояние ---------
#define FORWARD    1
#define LEFT       2
#define RIGHT      3
#define BACK       4


///////////////////////////////////////////////////////////////////////////
//----------------------------------- PID ---------------------------------
///////////////////////////////////////////////////////////////////////////
#define PWM_FREQ      4000

//----------------  40mm Wheels ---------------
#define LINE_TRESHOLD   40    // Ниво под което се смята че няма линия под сензора
#define SENSOR_TRESHOLD 5    // Филтриране на шумове от сензорите

//------------------  1000 rpm -------------
#define KP             0.55
#define KD             6
#define MAX_SPEED      100
#define ACQUIRE_SPEED  36

//---------------
#define SLOW_SPEED     MAX_SPEED - MAX_SPEED/4  // Стабилизирне след завръщане на линията
#define BREAK_LEVEL    MAX_SPEED * 0.30
#define BREAK_SPEED     -8

//  Скорости на моторите при изпуснат завой - търсене на линията в ляво или дясно
// Two speed for turn - for making right turn radius

#define TURN_SPEED_HIGH     MAX_SPEED + MAX_SPEED/10
#define TURN_SPEED_LOW      -8

// Лява и дясна позиция, след които се изпуска линията
#define TURN_ERROR_LEFT    100    //  100 minimum
#define TURN_ERROR_RIGHT   500    // 500 maximum


/////////////////////////////////////////////////////////////////////////
//------- Global Variables -----
signed long position;
int dir;
unsigned int sensors_sum;
signed int error;
signed int lastError;
signed int left_pwm;
signed int right_pwm;
signed int motorSpeed;
int stojnost;

//=================  CALIBRATE ==========================
unsigned int sensor_values[SENSORS_NR];
unsigned int sensor_calmin[SENSORS_NR];
unsigned int sensor_calmax[SENSORS_NR];
unsigned int sensor_denom[SENSORS_NR];

//--------- Common ------------
char tmp_str[64];

//--------------------------------
int melody[] = { NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4 };
// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = { 4, 8, 8, 4, 4, 4, 4, 4 };


//////////////////////////////////////////////////////////////////////////////
//
//                          SETUP
//////////////////////////////////////////////////////////////////////////////

//     Първоначална инициализация на всички входове, изходи и променливи
//
 
void setup() {

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  pinMode(LEFT_DIR, OUTPUT);
  pinMode(RIGHT_DIR, OUTPUT);

  // Управление на оптроните
  pinMode( OPT_ENABLE , OUTPUT);

  // --- Изходи  ---
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BEEP, OUTPUT);

  // --- Входове ---
  pinMode(BUTT1, INPUT_PULLUP);
  pinMode(IR_PIN, INPUT_PULLUP);

  //---- PWM инициализация за управление на моторите
  pinMode(LEFT_PWM, OUTPUT);
  pinMode(RIGHT_PWM, OUTPUT);

  tone(BEEP, 2400, 400);
  delay(2000);
  noTone(BEEP);

  analogWrite(LEFT_PWM, 0);
  analogWrite(RIGHT_PWM, 0);

  // Изчакване натискането на бутон
  while (digitalRead (BUTT1) == HIGH) ;

  callibrate();
  dir = FORWARD;
  lastError = 0;
  prevError = 0;

  // ---  Изпълняване на мелодията ---
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

  // Изчакване натискането на бутон или команда от старт модул
  while ((digitalRead (BUTT1) == HIGH)) ;

}


//============================================================================
//
// the loop routine runs over and over again forever:
//
//============================================================================
void loop() {
  int adc_value;
  long quadr;
  int  derivate;

  //---- Прочитане на текущата позиция и пресмятане на грешката ----
  position = read_position();
  error = 300 - position;

  switch (dir) {
    case FORWARD:    //Движение направо
    default:

      if (position < TURN_ERROR_LEFT) {  //Ако отклонението е много голямо преминаване в състояние "Завой наляво"
        // --- Turn ---
        right_motor_speed (TURN_SPEED_HIGH);
        left_motor_speed (TURN_SPEED_LOW);
        //Serial.println(">>>>>>>>>>>>>>>>>>>>>>>");
        dir = LEFT;
        break;
      }

      if (position > TURN_ERROR_RIGHT) {  //Ако отклонението е много голямо преминаване в състояние "Завой надясно"
        // --- Turn ---
        right_motor_speed (TURN_SPEED_LOW);
        left_motor_speed (TURN_SPEED_HIGH);
        //Serial.println("<<<<<<<<<<<<<<<<<<<<<");
        dir = RIGHT;
        break;
      }


      //-------------PD закон за управление---------------
      derivate =  (error - lastError) * KD;
      motorSpeed = KP * error + derivate;
      lastError = error;

      left_pwm = MAX_SPEED + motorSpeed;
      right_pwm = MAX_SPEED - motorSpeed;


      // ако MAX_SPEED > 140 трябва да се спира единия мотор.
      if (left_pwm < BREAK_LEVEL)
        left_pwm = BREAK_SPEED;
      if (right_pwm < BREAK_LEVEL )
        right_pwm = BREAK_SPEED;


      //---- Задаване на пресметнатата скорост на моторите ----
      left_motor_speed(left_pwm);
      right_motor_speed(right_pwm);
      break;

    case LEFT:     // Завой наляво при голямо отклонение - търсене на линия
      if (position > TURN_ERROR_LEFT + 50) {    //Maximum is 100%
        left_motor_speed (MAX_SPEED);       // Завръщане обратно на линията
        right_motor_speed (SLOW_SPEED);     // Спирачен момент в обратна посока (за предотвратяване на заклащането)

        dir = FORWARD;
        lastError = 0;
        prevError = 0;
        last_position = 0;
        delay (10);
      }
      break;


    case RIGHT:     // Завой надясно при голямо отклонение - търсене на линия
      if (position < TURN_ERROR_RIGHT - 50) {    //Maximum is 100%
        left_motor_speed (SLOW_SPEED);       // Завръщане обратно на линията
        right_motor_speed (MAX_SPEED);     // Спирачен момент в обратна посока (за предотвратяване на заклащането)

        dir = FORWARD;
        lastError = 0;
        prevError = 0;
        last_position = 0;
        delay (10);
      }
      break;

  }

  while (digitalRead(IR_PIN) == LOW);    // Спиране ако има команда от Старт модул

  delay(2);
}



//=======================================================================================

/* -----------------------------------------------------------------------------------
    Two way PWM speed control. If speed is >0 then motor runs forward.
    Else if speed is < 0 then motor runs backward
  ------------------------------------------------------------------------------------ */
void left_motor_speed(signed int motor_speed) {
  if (motor_speed == 0)
    motor_speed = 1;

  if (motor_speed > 0) {
    digitalWrite(LEFT_DIR, LOW);
  }
  else {
    digitalWrite(LEFT_DIR, HIGH);
  }
  motor_speed = abs(motor_speed);
  if (motor_speed > 255)
    motor_speed = 255;

  analogWrite(LEFT_PWM, motor_speed);
}

//---------------------------------------
void right_motor_speed(signed int motor_speed) {
  if (motor_speed == 0)
    motor_speed = 1;

  if (motor_speed > 0) {
    digitalWrite(RIGHT_DIR, LOW);
  }
  else {
    digitalWrite(RIGHT_DIR, HIGH);
  }
  motor_speed = abs(motor_speed);
  if (motor_speed > 255)
    motor_speed = 255;

  analogWrite(RIGHT_PWM, motor_speed);
}

//===============================  CALIBRATE ======================================
//-------------------
void callibrate(void) {
  int i, sens;
  int center_pos;
  unsigned int tmp_value;

  //--- Preset the arrays ---
  for (i = 0; i < SENSORS_NR; i++)
    (sensor_calmin)[i] = 1023;    // Maximum ADC value

  for (i = 0; i < SENSORS_NR; i++) {
    (sensor_calmax)[i] = 0;     // Minimum ADC value
    tmp_value = analogRead(sensors[i]);
  }

  //--- Turn right ---
  left_motor_speed(ACQUIRE_SPEED);    // forward
  right_motor_speed(-ACQUIRE_SPEED);    // backward
  digitalWrite( OPT_ENABLE , HIGH);
  delayMicroseconds(300);

  for (i = 0; i < 550; i++) {

    //----- Find minimum and maximum values for all sensors -----
    for (sens = 0; sens < SENSORS_NR; sens++) {
      tmp_value = analogRead(sensors[sens]) / 2;
      if (tmp_value < sensor_calmin[sens])
        sensor_calmin[sens] = tmp_value;
      if (tmp_value > sensor_calmax[sens])
        sensor_calmax[sens] = tmp_value;
    }

    if (i == 200) {   // --- turn left  ---
      left_motor_speed(-ACQUIRE_SPEED);
      right_motor_speed(ACQUIRE_SPEED);
    }
    delay(5);
  }

  digitalWrite( OPT_ENABLE , HIGH);
  Serial.println("Callibration values :");
  //-------   Calculate calibration  denom --------
  for (sens = 0; sens < SENSORS_NR; sens++) {
    sensor_denom[sens] = (sensor_calmax[sens] - sensor_calmin[sens]) / 10;

    sprintf(tmp_str, " min %d - max %d", sensor_calmin[sens], sensor_calmax[sens]);
    Serial.println(tmp_str);
  }

  //---------- Go back to the line ----------
  left_motor_speed(ACQUIRE_SPEED);
  right_motor_speed(-ACQUIRE_SPEED);
  do {
    tmp_value = analogRead(sensors[2]) / 2;
    center_pos = ((tmp_value - sensor_calmin[2]) * 10) / sensor_denom[2];// Center sonsor position in the array = 2
    delay(10);
  }
  while (center_pos < 80);

  // --- Stоп the motors ---
  left_motor_speed(0);
  right_motor_speed(0);

  digitalWrite( OPT_ENABLE , LOW);
}


//============================= Read sensors  and scale =====================
unsigned int read_position(void) {
  unsigned char on_line;
  static unsigned int last_pos;
  unsigned char sens;
  unsigned int tmp_value;
  signed long pos;

  pos = 0;
  sensors_sum = 0;
  on_line = 0;
  digitalWrite( OPT_ENABLE , HIGH);
  delayMicroseconds(350);    // Wait for lighting

  //-------------- Read sensors ------------
  for (sens = 0; sens < SENSORS_NR; sens++) {
    tmp_value = analogRead(sensors[sens]) / 2;

    //--------- Validate ----------
    if (tmp_value < sensor_calmin[sens])
      tmp_value = sensor_calmin[sens];
    if (tmp_value > sensor_calmax[sens])
      tmp_value = sensor_calmax[sens];

    //-------- Calibrate ----------
    sensor_values[sens] = ((tmp_value - sensor_calmin[sens]) * 10)
                          / sensor_denom[sens];

    //----------- Noise filtering ----------
    if (sensor_values[sens]  < SENSOR_TRESHOLD)
      sensor_values[sens] = 0;

    // The estimate position is made using a weighted average of the sensor values
    // multiplied by 100,  The formula is:
    //
    //    100*value0 + 200*value1 + 300*value2 + ...
    //   --------------------------------------------
    //         value0  +  value1  +  value2 + ...

    pos += sensor_values[sens] * ((sens + 1) * 100);
    sensors_sum += sensor_values[sens];

    //--- line presens check and count ---
    if (sensor_values[sens] > LINE_TRESHOLD) {
      on_line += 1;
    }
  }
  digitalWrite( OPT_ENABLE, LOW);

  if (!on_line) {
    // If it last read to the left of center, return 0.
    //if (last_pos < (SENSORS_NR - 1) * 100 / 2) {
    if (last_pos < 200) {
      last_pos = 90;
      // If it last read to the right of center, return the max.
    }
    else if (last_pos > 400) {
      last_pos = (SENSORS_NR * 100) + 10;
    } else {
      last_pos = 300;   // center pos
    }
  }
  else {
    if (on_line > 2) {
      tmp_value = sensor_values[0] + sensor_values[1];
      if (  tmp_value > 90 )
        last_pos = 100;
      tmp_value = sensor_values[SENSORS_NR - 2] + sensor_values[SENSORS_NR - 1];
      if  ( tmp_value > 90 )
        last_pos = (SENSORS_NR * 100);

    } else
      last_pos = pos / sensors_sum;
  }

  return last_pos;
}

