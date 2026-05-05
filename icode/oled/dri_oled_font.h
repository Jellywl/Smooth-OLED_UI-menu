/**
 * @file dri_oled_font.h
 * @author 是凌子呀_ (isLingX@outlook.com)
 * @brief
 * @version 1.0
 * @date 2023-03-12
 *
 * 更新日志：
 * v1.0 2023年7月12日
 * 更新部分函数翻译，重构代码，修改文字取模方式，与正点原子相同
 * XFONT PCTOLCD2002 文字取模方式:阴码-逐[列]式-顺向(高位在前)
 * XFONT PCTOLCD2002 图形取模方式:阴码-逐[行]式-顺向(高位在前)
 *
 * 数组命名方式 fontLir_文字_字体尺寸 例如 fontLir_cn_1616
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __DRIVER_OLED_FONT_H__
#define __DRIVER_OLED_FONT_H__

#include <stdint.h>



/*中文字符字节宽度*/
#define OLED_CHN_CHAR_WIDTH			(2)		//UTF-8编码格式给3，GB2312编码格式给2

/*字模基本单元*/
typedef struct 
{
	char Index[OLED_CHN_CHAR_WIDTH + 1];	//汉字索引
	uint8_t Data[60];						//字模数据
} ChineseCell20x20_t;
typedef struct 
{
	char Index[OLED_CHN_CHAR_WIDTH + 1];	//汉字索引
	uint8_t Data[32];						//字模数据
} ChineseCell16x16_t;
typedef struct 
{
    char Index[OLED_CHN_CHAR_WIDTH + 1];   // 汉字索引
    uint8_t Data[24];                      // 字模数据，12*12的汉字需要24字节
} ChineseCell12x12_t;

typedef struct 
{
    char Index[OLED_CHN_CHAR_WIDTH + 1];   // 汉字索引
    uint8_t Data[8];                      // 字模数据，8*8的汉字需要24字节
} ChineseCell8x8_t;

/*ASCII字模数据声明*/
extern const uint8_t OLED_F10x20[][30];
extern const uint8_t OLED_F8x16[][16];
extern const uint8_t OLED_F7x12[][14];
extern const uint8_t OLED_F6x8[][6];

/*汉字字模数据声明*/
extern const ChineseCell20x20_t OLED_CF20x20[];
extern const ChineseCell16x16_t OLED_CF16x16[];
extern const ChineseCell12x12_t OLED_CF12x12[];  // 声明12x12字模数组
extern const ChineseCell8x8_t OLED_CF8x8[];  // 声明12x12字模数组
/*图像数据声明*/
extern const uint8_t Arrow[];
extern const uint8_t UnKnown[];
extern const uint8_t Image_setings[];
extern const uint8_t Image_window[];
extern const uint8_t Image_wechat[];
extern const uint8_t Image_qq[];
extern const uint8_t Image_alipay[];
extern const uint8_t Image_more[];
extern const uint8_t Image_calc[];
extern const uint8_t Image_night[];
extern const uint8_t Image_sleep[];


extern const uint8_t Image_settings_64[];
extern const uint8_t Image_calc_64[];
extern const uint8_t Image_wechat_64[];
extern const uint8_t Image_alipay_64[];
extern const uint8_t Image_night_64[];
extern const uint8_t Image_more_64[];
extern const uint8_t OLED_UI_LOGO[];
extern const uint8_t OLED_UI_LOGOTEXT[];
extern const uint8_t OLED_UI_LOGOTEXT64[];
extern const uint8_t OLED_UI_LOGOGithub[];
extern const uint8_t Image_alipay_QR_Code[];
extern const uint8_t OLED_UI_SettingsLogo[];
/*按照上面的格式，在这个位置加入新的图像数据声明*/
//...

#endif
