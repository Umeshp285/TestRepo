
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l0xx_hal.h"
#include<stdbool.h>
#include<string.h>
#include<stdlib.h>
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
extern void initialise_monitor_handles(void);


bool temp=true;
uint8_t str1[128];
uint8_t str2[128];

#define PMTK_SET_NMEA_UPDATE_100_MILLIHERTZ			"$PMTK220,10000*2F\r\n" // Once every 10 seconds, 100 millihertz.
#define PMTK_SET_NMEA_UPDATE_200_MILLIHERTZ			"$PMTK220,5000*1B\r\n"  // Once every 5 seconds, 200 millihertz.
#define PMTK_SET_NMEA_UPDATE_1HZ  					"$PMTK220,1000*1F\r\n"	// Once every 1 second, 1 Hz
#define PMTK_SET_NMEA_UPDATE_5HZ  					"$PMTK220,200*2C\r\n"		// 5 Times every second, 5 Hz
#define PMTK_SET_NMEA_UPDATE_10HZ 					"$PMTK220,100*2F\r\n"		// 10 Times every second, 10 Hz

#define PMTK_API_SET_FIX_CTL_100_MILLIHERTZ			"$PMTK300,10000,0,0,0,0*2C\r\n" // Once every 10 seconds, 100 millihertz.
#define PMTK_API_SET_FIX_CTL_200_MILLIHERTZ			"$PMTK300,5000,0,0,0,0*18\r\n"  // Once every 5 seconds, 200 millihertz.
#define PMTK_API_SET_FIX_CTL_1HZ					"$PMTK300,1000,0,0,0,0*1C\r\n"
#define PMTK_API_SET_FIX_CTL_5HZ					"$PMTK300,200,0,0,0,0*2F\r\n"


#define PMTK_SET_BAUD_57600							"$PMTK251,57600*2C\r\n"
#define PMTK_SET_BAUD_9600							"$PMTK251,9600*17\r\n"


#define PMTK_SET_NMEA_OUTPUT_RMCONLY 				"$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n"  // turn on only the second sentence (GPRMC)

#define PMTK_SET_NMEA_OUTPUT_RMCGGA					"$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n"  // turn on GPRMC and GGA

#define PMTK_SET_NMEA_OUTPUT_ALLDATA				"$PMTK314,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n"  // turn on ALL THE DATA

#define PMTK_SET_NMEA_OUTPUT_OFF					"$PMTK314,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n"  // turn off output

#define PMTK_TEST 									"$PMTK000*32\r\n"
#define PMTK_HOT_START 								"$PMTK101*32\r\n"
#define PMTK_SET_BAUDRATE						    "$PMTK251,9600*27\r\n"

#define PMTK_AIC_ENABLE 							"$PMTK286,1*23\r\n"  //AIC Enable
#define PMTK_EASY_ENABLE							"$PMTK869,1,1*35\r\n"

void NMEA_Parse(char* nmea)
{
	 int32_t degree;
	 long minutes;
	 char degreebuff[10];
	 float timef;
	 uint32_t time;
	 uint8_t hour , minute , seconds ;
	 uint8_t day , year , month ;

	 float latitude , longitude ;
	 int32_t latitude_fixed , longitude_fixed ;
	 float latitude_Degrees , longitude_Degrees ;
	 float speed , angle ;


	 if(strstr(nmea , "$GPRMC"))
	 {
		 char* p = nmea ; // RMC sentence found

		 //Time calculation
		 p = strchr(p , ',')+1;
		 timef = atof(p);
		 time  = timef ;
		 hour = time/10000;
		 minute = (time % 10000)/100 ;
		 seconds = (time % 100);

		printf("Time = %d:%d:%d\r\n",hour,minute,seconds);

	  /*  Data validation
		 A : Data valid
		 V : Data not valid */

		 p = strchr(p , ',')+1;
		 if(p[0]=='A')
		 {
			 printf("Data valid\r\n");
		 }
		 else
		 {
			 printf("Data not valid..\r\n");
		 }

		 // Latitude


		    // parse out latitude
		    p = strchr(p, ',')+1;

		    if (',' != *p)
		    {
		      strncpy(degreebuff, p, 2);
		      p += 2;
		      degreebuff[2] = '\0';

		      long degree = atol(degreebuff) * 10000000;
		      strncpy(degreebuff, p, 2); // minutes
		      p += 3; // skip decimal point

		      strncpy(degreebuff + 2, p, 4);
		      degreebuff[6] = '\0';

		      long minutes = 50 * atol(degreebuff) / 3;
		      latitude_fixed = degree + minutes;
		      latitude = degree / 100000 + minutes * 0.000006F;

		      latitude_Degrees = (latitude-100*(int)(latitude/100))/60.0;
		      latitude_Degrees += (int)(latitude/100);
		      printf(" Latitude = %f\r\n",latitude_Degrees);
		    }

		    // Latitude Direction South or North
		    p = strchr(p , ',')+1;
		 	if(',' != *p)
		 	{
		 		if(p[0]=='N')
		 			printf("North..\r\n");
		 		else if(p[0]=='S')
		 			printf("South..\r\n");
		  	}

		 	// Longitude
		 	p = strchr(p , ',')+1;
		 	if(',' != *p)
		 	{
		 		strncpy(degreebuff , p ,3);
		 		p += 3;

		 		degreebuff[3] = '\0';
		 		degree = atol(degreebuff) * 10000000;

		 		strncpy(degreebuff, p, 2); // minutes
		 		p += 3; // skip decimal point
		 		strncpy(degreebuff + 2, p, 4);
		 		degreebuff[6] = '\0';
		 		minutes = 50 * atol(degreebuff) / 3;
		 		longitude_fixed = degree + minutes;
		 		longitude = degree / 100000 + minutes * 0.000006F;
		 		longitude_Degrees = (longitude-100* (int)(longitude/100))/60.0;
		 		longitude_Degrees += (int)(longitude/100);
		 		printf(" Longitude = %f\r\n",longitude_Degrees);
		 	}

		 	//Longitude Direction East or West
		 	p = strchr(p , ',')+1;
		 	if(',' != *p)
		 	{
		 		if(p[0]=='W')
		 			printf("West..\r\n");
		 		else if(p[0]=='E')
		 			printf("East...\r\n");
		 	}

		 	 //Speed
		 	p = strchr(p, ',')+1;
		 	if (',' != *p)
		 	{
		 	     speed = atof(p);
		 	     speed = (speed*1.852);  //1 knots = 1.852 kmph
		 	   //  printf("speed = %f\r\n" , speed);
		 	}

		 	 //Angle
		 	 p = strchr(p, ',')+1;
		 	 if (',' != *p)
		 	 {
		 	    angle = atof(p);
		 	   // printf("Angle = %f\r\n",angle);
		 	 }

		 	 // Day , Month , Year
		 	 p = strchr(p, ',')+1;
		 	 if (',' != *p)
		 	 {
		 		 uint32_t fulldate = atof(p);
		 		 day = fulldate / 10000;
		 		 month = (fulldate % 10000) / 100;
		 		 year = (fulldate % 100);

		 		 printf("Date = %d/%d/%d\r\n" , day,month,year);

		 	 }
	 }

}





void GPS_SendCommand(char* str)
{
	HAL_Delay(100);
	HAL_UART_Transmit(&huart1,(uint8_t *)str,strlen(str),1000);
}

void GPS_Commands(void)
{

	  GPS_SendCommand(PMTK_HOT_START);
	  HAL_Delay(1000);
	  GPS_SendCommand(PMTK_EASY_ENABLE);
	  GPS_SendCommand(PMTK_AIC_ENABLE);
	  GPS_SendCommand(PMTK_SET_BAUDRATE);
	  GPS_SendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
	  GPS_SendCommand(PMTK_API_SET_FIX_CTL_1HZ);
	  HAL_Delay(1000);

	  GPS_SendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);

}

int main(void)
{
  initialise_monitor_handles();
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();

  HAL_Delay(1000);

  GPS_Commands();



  while (1)
  {
	  HAL_UART_Receive_IT(&huart1,(uint8_t *)str1,128);
  }

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	uint8_t str3[128];

		printf("%s\r\n",str1);
		strcpy(str3 , str1);
HAL_UART_Transmit(&huart2,str3,128,100);
		if(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13) && temp)
		{
			NMEA_Parse(str1);
			HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_5);
			temp = false ;
		}
		else if(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13) == GPIO_PIN_RESET)
		{
			temp = true;
		}

		//HAL_UART_Transmit(&huart2,(uint8_t*)str1,80,100);

		//strcpy(str2,str1);

		HAL_UART_Receive_IT(&huart1,(uint8_t*)str1,66);
}


void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_5;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USART2 init function */
static void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
