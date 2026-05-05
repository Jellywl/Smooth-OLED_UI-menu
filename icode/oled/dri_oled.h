/**
 *******************************************************************************
 * @file dri_oled.h
 * @author 是凌子呀 _ (isLingX@outlook.com)
 * @brief OLED驱动头文件
 * @version 1.2
 * @date 2023-03-13
 *******************************************************************************
 * 本程序适用于 SSD1315芯片的12864屏幕
 * 本程序适用于 模拟I2C、SPI驱动程序，
 * 可以切换不同的宏定义选择模拟I2C、硬件I2C、硬件I2D-DMA、模拟SPI、硬件SPI、硬件SPI-DMA
 * 建议使用I2C-DMA模式、速度手动更改为1.4MHz，这样既有不错的刷新率，还有不错的空闲时间用来处理其他事件
 *
 * 在默认   400KHz的情况下可以实现30fps刷新，每秒计算150万次 只刷新数字可以达到35fps 每秒计算160.8万次
 * 在       800KHz的情况下可以实现48fps刷新，每秒计算120万次 只刷新数字可以达到60fps 每秒计算146.6万次
 * 在       1.4MHz的情况下可以实现62fps刷新，每秒计算93万次 只刷新数字可以达到87fps 每秒计算124.3万次
 * 在只刷新数字的情况下，1.4MHz可以每秒刷新88次，计算120万次，但频率越高，抗干扰能力也就越弱。
 ********************************************************************************
 * I2C
 * 1.VCC 供电 3.3～5.5V DC
 * 2.GND 接地，电源负极
 * 3.SCL
 * 4.SDA
 *
 *SPI
 * 1.VCC 供电 3.3～5.5V DC
 * 2.GND 接地，电源负极
 * 3.D0 SCLK（Serial Clock）：时钟线，由主设备提供。它驱动数据的传输，每个数据位在时钟的上升或下降沿进行传输。
 * 4.D1 MOSI（Master Out Slave In）：主设备输出从设备输入线，也称为数据线。主设备通过该线将数据发送给从设备。
 * 5.RST RESET 复位引脚
 * 6.DC DC（Data/Command）脚用于区分发送的数据是命令还是实际的数据。
 * 7.CS CS（Chip Select）：片选线，用于选择从设备。当CS线为低电平时，表示选择对应的从设备，使其处于通信状态。当CS线为高电平时，表示不选择该从设备，使其处于非通信状态。
 ********************************************************************************
 * 更新日志
 * v1.0 2023.07.12
 * - 更新部分函数翻译
 * - 修改文字取模方式，与正点原子相同
 * - XFONT PCTOLCD2002 文字取模方式:阴码-逐[列]式-顺向(高位在前)
 * - XFONT PCTOLCD2002 图形取模方式:阴码-逐[行]式-顺向(高位在前)   //
 * v1.1 202307.18
 * - 增加SPI驱动
 * - 增加对于驱动模式的选择功能，可以根据选择相应的宏定义自动选择模式
 * v1.2 2023.07.28
 * - 注释掉测试代码，原需要使用tim3测试刷新率
 * v1.3 2023.08.3
 * - 增加数字显示代码，可以显示数字，无符号数字以及浮点型数字
 * v1.3.1 2023.08.29
 * - 增加 OLED_Get_Point(...); 函数，用来获取点是否存在
 * - 修复 OLED_Show_Char(...); OLED_Show_Char_CN(...); 函数非完整显示时的错误。
 * - OLED_Show_Char(...); 仍然存在显示的BUG，在下个版本后修复
 * v1.3.2 2026.05.04
 * PCTOLCD2002 文字取模方式:阴码-列行式-逆向(低位在前) 十六进制，C51格式 
 ********************************************************************************
 *
 * @copyright Copyright (c) 2023
 *
 ********************************************************************************
 */

#ifndef __DRIVER_OLED_H__
#define __DRIVER_OLED_H__

// 检测是否是C++编译器
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include "dri_OLED_font.h"
#include "stdbool.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

    /**
     * @brief 驱动方式选择
     */
    /*!!!通讯模式只能选择其中一种,并且已经进行相关通讯方式的初始化!!!*/
    /*!!!通讯模式只能选择其中一种,并且已经进行相关通讯方式的初始化!!!*/
    /*!!!通讯模式只能选择其中一种,并且已经进行相关通讯方式的初始化!!!*/
    // #define OLED_I2C_DRIVER        /*使用I2C通信-硬件  -- 需要配置I2C相关功能，这里用的是I2C1，建议FastMode模式 1.4M*/
    // #define OLED_I2C_DRIVER_DMA /*使用I2C通信-硬件DMA -- 需要配置I2C及DMA相关功能，这里用的是I2C1,建议FastMode模式 1.4M */
    // #define OLED_I2C_DRIVER_ANALOG /*使用I2C通信-模拟 -- 建议配置SCL和SDA为推挽输出 高速模式*/
    // #define OLED_SPI_DRIVER /*使用SPI通信-硬件 -- 需要配置SPI相关功能，这里用的是SPI1*/
#define OLED_SPI_DRIVER_DMA /*使用SPI通信-硬件DMA -- 需要配置SPI和DMA相关功能，这里用的是SPI1*/
// #define OLED_SPI_DRIVER_ANALOG /*使用SPI通信-模拟 -- 建议配置RST、C、DC为推挽输出 高速模式*/

/**
 * @brief 屏幕信息
 */
#define OLED_WIDTH 128 /*屏幕X-宽度*/
#define OLED_HEIGHT 64 /*屏幕Y-高度*/
#define OLED_ADD 0x78  /*oled设备地址*/

/*半角字符参数取值*/
/*此参数值不仅用于判断，而且用于计算横向字符偏移，默认值为字体像素宽度*/
#define OLED_10X20_HALF (10)
#define OLED_8X16_HALF (8)
#define OLED_7X12_HALF (7)
#define OLED_6X8_HALF (6)

/*全角大小参数取值*/
#define OLED_8X8_FULL (8)
#define OLED_12X12_FULL (12)
#define OLED_16X16_FULL (16)
#define OLED_20X20_FULL (20)

/*字体间隔取值*/
#define OLED_CHAR_SPACING (0)
#define OLED_LINE_SPACING (2)

/*IsFilled参数数值*/
#define OLED_UNFILLED (0)
#define OLED_FILLED (1)

/**关于字符串最大长度的宏，用于格式化输出字符串*/
#define MAX_STRING_LENGTH 128

/**
 * @brief 其他不要动的
 */
#define DIS_INV 0   // 字符反色显示
#define DIS_NOR 1   // 字符填充显示
#define OLED_CMD 0  // 写命令
#define OLED_DATA 1 // 写数据

//================================================================
// I2C相关   										    		 =
//================================================================
/**
 * @brief 模拟I2C引脚配置
 */
#ifdef OLED_I2C_DRIVER_ANALOG

#define OLED_SCL_GPIO_Port GPIOB /*SCL - PB6*/
#define OLED_SCL_Pin GPIO_PIN_10 /*SCL - PB6*/
#define OLED_SDA_GPIO_Port GPIOB /*SDA -  PB7*/
#define OLED_SDA_Pin GPIO_PIN_11 /*SDA -  PB7*/

#define OLED_SCL_Clr() HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_RESET) /*SCL Low Power*/
#define OLED_SCL_Set() HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_SET)   /*SCL High Power*/
#define OLED_SDA_Clr() HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_RESET) /*SDA Low Power*/
#define OLED_SDA_Set() HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_SET)   /*SDA High Power*/

#endif /*OLED_I2C_DRIVER_ANALOG*/

/*屏幕刷新状态反馈-DMA模式生效*/
#if defined(OLED_I2C_DRIVER_DMA) || defined(OLED_SPI_DRIVER_DMA)

    extern uint8_t OLED_Update_Flag; // oled刷新标志 0刷新完成，允许再次刷新，1等待刷新完成，不允许再次刷新-使用DMA时使用

#endif /*OLED_I2C_DRIVER_DMA || OLED_SPI_DRIVER_DMA*/

//================================================================
// SPI相关   										    		 =
//================================================================
/**
 * @brief 模拟SPI引脚配置
 */
#ifdef OLED_SPI_DRIVER_ANALOG

#define OLED_SCL_GPIO_Port GPIOA /*CLK - PA5*/
#define OLED_SCL_Pin GPIO_PIN_5  /*CLK - PA5*/
#define OLED_SDA_GPIO_Port GPIOA /*SDA - PA7*/
#define OLED_SDA_Pin GPIO_PIN_7  /*SDA - PA7*/

#define OLED_SCL_Clr() HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_RESET) /*CLK Low Power*/
#define OLED_SCL_Set() HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_SET)   /*CLK High Power*/
#define OLED_SDA_Clr() HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_RESET)  /*SDA Low Power*/
#define OLED_SDA_Set() HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_SET)    /*SDA High Power*/

#endif /*OLED_SPI_DRIVER_ANALOG*/

/*SPI 其他的引脚定于*/
#if defined(OLED_SPI_DRIVER) || defined(OLED_SPI_DRIVER_DMA) || defined(OLED_SPI_DRIVER_ANALOG)

#define OLED_DC_GPIO_Port GPIOB  /*DC - PA1*/
#define OLED_DC_Pin GPIO_PIN_12  /*DC - PA1*/
#define OLED_RST_GPIO_Port GPIOA /*RST - PA2*/
#define OLED_RST_Pin GPIO_PIN_8  /*RST - PA2*/
#define OLED_CS_GPIO_Port GPIOA  /*CS - PA4*/
#define OLED_CS_Pin GPIO_PIN_9   /*CS - PA4*/

#define OLED_CS_Clr() HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET)    /*CS Low Power*/
#define OLED_CS_Set() HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET)      /*CS High Power*/
#define OLED_DC_Clr() HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET)    /*DC Low Power*/
#define OLED_DC_Set() HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET)      /*DC High Power*/
#define OLED_RES_Clr() HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET) /*RST Low Power*/
#define OLED_RES_Set() HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET)   /*RST High Power*/

#endif

// 字符显示
/**关于是否使用精简版vsprintf函数代替标准vsprintf函数的宏。如果开启此选项，代码体积将减少约5kb，有利于小型嵌入式系统**/
#define USE_SIMPLE_VSPRINTF (true)

    // 工具函数，实现了一个简单的vsprintf函数，用于格式化输出字符串
    int OLED_Simple_vsprintf(char *buf, const char *format, va_list args);

    //================================================================
    // 屏幕相关功能函数                                               =
    //================================================================

    // 基础显示设定
    void OLED_Color_Turn(uint8_t i);                                            // 正反显示切换
    void OLED_Display_Turn(uint8_t i);                                          // 显示方向切换
    void OLED_DisPlay_On(void);                                                 // 开启OLED显示
    void OLED_DisPlay_Off(void);                                                // 关闭OLED显示
    void OLED_Refresh(void);                                                    // 更新显存到OLED
    void OLED_Clear(uint8_t clr);                                               // 清屏函数
    void OLED_ClearArea(int16_t X, int16_t Y, int16_t Width, int16_t Height);   // 局部清屏函数
    void OLED_Reverse(void);                                                    // 屏幕反色
    void OLED_ReverseArea(int16_t X, int16_t Y, int16_t Width, int16_t Height); // 屏幕局部反色
    void OLED_Brightness(int16_t Brightness);                                   // OLED 设置亮度函数

    void OLED_SetColorMode(bool colormode); // 设置颜色模式
    // 显存数组是否变化
    bool OLED_IfChangedScreen(void);

    void OLED_WriteDataArr(uint8_t *Data, uint8_t Count);

    // 绘制位图
    void OLED_ShowImage(int16_t X, int16_t Y, uint16_t Width, uint16_t Height, const uint8_t *Image);
    // 显示ASCII字符与数字
    void OLED_ShowChar(int16_t X, int16_t Y, char Char, uint8_t FontSize);
    void OLED_ShowNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
    void OLED_ShowSignedNum(int16_t X, int16_t Y, int32_t Number, uint8_t Length, uint8_t FontSize);
    void OLED_ShowHexNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
    void OLED_ShowBinNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
    void OLED_ShowFloatNum(int16_t X, int16_t Y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize);
    // 显示中英文字符串
    void OLED_ShowString(int16_t X, int16_t Y, char *String, uint8_t FontSize);
    void OLED_ShowMixString(int16_t X, int16_t Y, char *String, uint8_t ChineseFontSize, uint8_t ASCIIFontSize);
    void OLED_ShowChinese(int16_t X, int16_t Y, char *Chinese, uint8_t FontSize);
    void OLED_Printf(int16_t X, int16_t Y, uint8_t FontSize, char *format, ...);
    void OLED_PrintfMix(int16_t X, int16_t Y, uint8_t ChineseFontSize, uint8_t ASCIIFontSize, const char *format, ...);

    // Area系列显示函数，可以想象为将屏幕使用蒙版遮挡，并在上面挖出一个X2，Y2，AreaWidth，AreaHeight的透明区域，图片仅仅会在这个区域内显示
    void OLED_ShowImageArea(int16_t X_Pic, int16_t Y_Pic, int16_t PictureWidth, int16_t PictureHeight, int16_t X_Area, int16_t Y_Area, int16_t AreaWidth, int16_t AreaHeight, const uint8_t *Image);
    void OLED_ShowCharArea(int16_t RangeX, int16_t RangeY, int16_t RangeWidth, int16_t RangeHeight, int16_t X, int16_t Y, char Char, uint8_t FontSize);
    void OLED_ShowStringArea(int16_t RangeX, int16_t RangeY, int16_t RangeWidth, int16_t RangeHeight, int16_t X, int16_t Y, char *String, uint8_t FontSize);
    void OLED_ShowChineseArea(int16_t RangeX, int16_t RangeY, int16_t RangeWidth, int16_t RangeHeight, int16_t X, int16_t Y, char *Chinese, uint8_t FontSize);
    void OLED_PrintfArea(int16_t RangeX, int16_t RangeY, int16_t RangeWidth, int16_t RangeHeight, int16_t X, int16_t Y, uint8_t FontSize, char *format, ...);
    void OLED_ShowMixStringArea(int16_t RangeX, int16_t RangeY, int16_t RangeWidth, int16_t RangeHeight, int16_t X, int16_t Y, char *String, uint8_t ChineseFontSize, uint8_t ASCIIFontSize);
    void OLED_PrintfMixArea(int16_t RangeX, int16_t RangeY, int16_t RangeWidth, int16_t RangeHeight, int16_t X, int16_t Y, uint8_t ChineseFontSize, uint8_t ASCIIFontSize, char *format, ...);

    // 绘制函数，绘制基础的ui
    void OLED_DrawPoint(int16_t X, int16_t Y);
    uint8_t OLED_GetPoint(uint8_t X, uint8_t Y);
    void OLED_DrawLine(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1);
    void OLED_DrawRectangle(int16_t X, int16_t Y, int16_t Width, int16_t Height, uint8_t IsFilled);
    void OLED_DrawTriangle(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint8_t IsFilled);
    void OLED_DrawCircle(int16_t X, int16_t Y, int16_t Radius, uint8_t IsFilled);
    void OLED_DrawEllipse(int16_t X, int16_t Y, int16_t A, int16_t B, uint8_t IsFilled);
    void OLED_DrawArc(int16_t X, int16_t Y, int16_t Radius, int16_t StartAngle, int16_t EndAngle, uint8_t IsFilled);
    void OLED_DrawRoundedRectangle(int16_t X, int16_t Y, int16_t Width, int16_t Height, int16_t Radius, uint8_t IsFilled);

    void DrawCube3D(int16_t centerX, int16_t centerY, float size, uint8_t perspective);

    void OLED_Init(void); // OLED初始化
    // void OLED_i2c_TestCode(void);                                                                                     // 实例代码

#endif /* __DRIVER_OLED_H__ */
