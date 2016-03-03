/*
Copyright (c) Simeon Simeonov
 Author : Simeon Simeonov
 Name : QTR sensors library
 Ver: 1.0.0

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _LIB_SENSORS_MODULE_H_
#define _LIB_SENSORS_MODULE_H_

//*****************************************************************************
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

#define SENSORS_NR	5
#define LINE_TRESHOLD	30	// calibrated values ar fom 0 - 100 %
#define SENSOR_TRESHOLD	10

//----------------------------------
extern unsigned int sensor_values[SENSORS_NR];
extern unsigned int sensor_calmin[SENSORS_NR];
extern unsigned int sensor_calmax[SENSORS_NR];
extern unsigned int sensor_denom[SENSORS_NR];
extern unsigned char line_map;
//----------------------------------
void sensors_init(void);
void callibrate(void);
void calc_denom(void);
unsigned int read_position(void);


//*****************************************************************************
// Mark the end of the C bindings section for C++ compilers.
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif /* _LIB_SENSORS_MODULE_H_ */
