#ifndef __MAIN__H__
#define __MAIN__H__

#include "adc_port.h"

#define PIN_SEM_SENSOR_1_SIGNAL 13
#define PIN_SEM_SENSOR_2_SIGNAL 27
// #define PIN_SEM_R_SIGNAL 33
// #define PIN_SEM_G_SIGNAL 25

#define MASK_BYTE 0x07FF //Define la cantidad de bits que se van a utiizar para limitar el número random en rango, hay que mantenerlo igual al ADC para aprovechar el rango que se toma como ms
#define BIT_MASK_R 0
#define BIT_MASK_G 1

#define BIT_MASK_SENSOR_1 3
#define BIT_MASK_SENSOR_2 4


#endif /*__MAIN__H__*/