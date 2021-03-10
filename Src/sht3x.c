/*
 * sht3x.c
 *
 *  Created on: 24-Jan-2019
 *      Author: cdac
 */


#include"sht3x.h"
#include"main.h"

extern I2C_HandleTypeDef hi2c1;
static uint8_t rxbuff[6];

void sht3x_update(void)
{
	uint8_t data[2];

	data[0] = 0x24;
	data[1] = 0x00;

	HAL_I2C_Master_Transmit(&hi2c1,DevAddress,data,2,Timeout);
	HAL_I2C_Master_Receive(&hi2c1 , DevAddress , rxbuff,6,Timeout);

}

float get_temperature(void)
{
	float temperature;

	uint16_t rawT;
	rawT = rxbuff[0];
	rawT = rawT << (uint16_t) 8 ;
	rawT = rawT + (uint16_t) rxbuff[1];
	temperature = 175.0 * (float) rawT / 65535.0;
	temperature = temperature - 45;

	return temperature;

}


float get_humidity(void)
{
	float humidity;

	uint16_t rawH;
	rawH = rxbuff[3];
	rawH = rawH << (uint16_t) 8;
	rawH = rawH + (uint16_t) rxbuff[4];
	humidity = 100.0 * (float) rawH / 65535.0;

	return humidity;
}
