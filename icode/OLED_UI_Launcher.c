/*
 *  OLED_UI_Launcher.c
 *  Version: 0.2.1
 *  Created on: Jun 26, 2025
 *  Author: 5akura1
 */
#include "OLED_UI_Launcher.h"

/**
 * @brief OLED用户界面初始化函数
 * @note 此函数用于初始化OLED显示界面
 */
void OLED_UI_init(void)
{
	// init
	// 开启外围器件
	// 注意：开启外围器件可能导致电压波动，不要在此操作之后立即读写Flash
	//	HAL_GPIO_WritePin(PWRON_GPIO_Port, PWRON_Pin, 0);
//		BTN_init();
	OLED_UI_Init(&MainMenuPage);
}

void OLED_UI_start(void)
{
	while (1)
	{
		//		BtnTask();

		OLED_UI_MainLoop();
	}
}

//// 定时器1中断回调函数
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//	if (htim == &htim1)
//	{
//		// 中断函数
//		OLED_UI_InterruptHandler();
//	}
//}
