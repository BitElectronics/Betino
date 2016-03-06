//--------------------------------------------------------------
//
//             Ultrasonic library
//		standard ANSI C
//            author : Simeon Simeonov  / Bit Electronics
//---------------------------------------------------------------

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************

#ifdef __cplusplus
extern "C"
{
#endif

//--- Променливи за конфигурация на пиновете ---
//int trig, echo;

void ultrasonic_init(int trig_pin, int echo_pin);
int us_meassure(void);



//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif


