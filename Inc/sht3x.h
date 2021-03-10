/*
 * sht3x.h
 *
 *  Created on: 24-Jan-2019
 *      Author: cdac
 */

#ifndef SHT3X_H_
#define SHT3X_H_



#include"stm32l0xx_hal.h"

void sht3x_update(void);
float get_temperature(void);
float get_humidity(void);

#define DevAddress 0x45<<1
#define Timeout 100

#endif /* SHT3X_H_ */
