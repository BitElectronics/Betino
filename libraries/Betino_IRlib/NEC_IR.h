/*
 * NEC infrared receiver library
 *
 *  Created on: 21.10.2015 �.
 *      Author: Simo
 */

#ifndef LIB_IR_MODULE_H_
#define LIB_IR_MODULE_H_

//*****************************************************************************
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

void ir_timer_init(void);


typedef struct {
	unsigned char address_low;
	unsigned char address_high;
	unsigned char command;
	unsigned char command_n;
} IRDATA;

//======================= Common dunctions =============================
void PortEIntHandler(void);
void Timer0AIntHandler(void);

/*
 * Functions
 */
void ir_init(void);
extern volatile IRDATA ir_data;
extern unsigned char ir_data_get;


//*****************************************************************************
// Mark the end of the C bindings section for C++ compilers.
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif /* LIBRARIES_IR_MODULE_H_ */


