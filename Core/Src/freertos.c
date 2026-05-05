/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "dri_oled.h"
#include "ui.h"
#include "test.h"
#include "OLED_UI.h"
#include "OLED_UI_MenuData.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// FPS Í³ï¿½Æ±ï¿½ï¿½ï¿½
extern uint32_t fps_cnt;   
extern float OLED_fps;
extern char fps_str[16];
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

extern uint32_t number;
extern char buf[32];
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */





/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for Task */
osThreadId_t TaskHandle;
const osThreadAttr_t Task_attributes = {
  .name = "Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
//------------------------------------------------------------------------------------------------------------------------------------------
void MyTask(void *argument)
{
	unsigned char buf2[32];
	while(1)
	{
		
//		if(OLED_IfChangedScreen()){
//				fps_cnt++;
//			}
//			
//		
////		OLED_Clear(0);
//		HAL_Delay(10);

//		if(number++ >=245)
//	{
//		number=5;
//	}
//	OLED_Brightness(number);
//	sprintf((char*)buf2, "%lu", number);  //ï¿½ï¿½uint32_t
//		
//		OLED_Printf(0, 28,OLED_7X12_HALF, "Task2:");
//		OLED_Printf(37, 28,OLED_7X12_HALF, (char*)buf2);
//		
//		OLED_ShowChinese(0,0,"Ò»¶þÈý",OLED_8X8_FULL);
//		
//		sprintf(fps_str,"FPS:%.0f",OLED_fps);  //
//		OLED_Printf(80, 0,OLED_6X8_HALF, fps_str);
////		OLED_ClearArea(0, 28,12,12);
////		OLED_Reverse();
//			OLED_Refresh();
	}
	
}

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of Task */
  TaskHandle = osThreadNew(StartDefaultTask, NULL, &Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
	xTaskCreate(MyTask,"MyTask",128,NULL,osPriorityNormal,NULL);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the Task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
	
  OLED_Init();
	OLED_UI_Init(&MainMenuPage);
	
	uint16_t cont=0;
	
	
  for(;;)
  {
		
		OLED_UI_MainLoop();
		
//	HAL_Delay(1000);
//		
//	cont++;
//	if(cont ==9999)
//	{
//		cont =0;
//		
//	}
//		sprintf(buf, "%lu", cont);  // 

//		OLED_Printf(0, 16,OLED_6X8_HALF, "Task1:");
//		OLED_Printf(37, 16,6, buf);
//		
//		
////		OLED_Refresh();
////    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

