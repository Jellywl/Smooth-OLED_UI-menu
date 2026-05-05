# OLED_UI 移植与开发指南

本文档帮助你理解 OLED_UI 框架的代码结构，以便移植到其他芯片或屏幕。

---

## 一、整体架构

```
┌─────────────────────────────────────────────┐
│              应用层 (OLED_UI_MenuData.c)       │  ← 定义菜单结构、回调函数
├─────────────────────────────────────────────┤
│              UI 引擎 (OLED_UI.c)              │  ← 菜单逻辑、动画、输入处理
├─────────────────────────────────────────────┤
│         硬件抽象层 (OLED_UI_Driver.c/.h)      │  ← 按键、编码器、延时（移植需改）
├─────────────────────────────────────────────┤
│         OLED 驱动 (dri_oled.c/.h)             │  ← 屏幕通信、绘图函数（移植需改）
└─────────────────────────────────────────────┘
```

移植时只需要改最下面两层，上面两层可以直接复用。

---

## 二、需要移植的文件（2个）

### 2.1 OLED_UI_Driver — 硬件抽象层

这个文件是**移植的关键**，它把 UI 框架和具体硬件隔离开。你需要实现以下接口：

#### 按键读取（4个宏，定义在 .h 中）

```c
// 返回 0 = 按下，1 = 未按下（低电平有效）
#define Key_GetEnterStatus()    HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin)
#define Key_GetBackStatus()     HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin)
#define Key_GetUpStatus()       HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin)
#define Key_GetDownStatus()     HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin)
```

移植时改成你的芯片的 GPIO 读取方式。例如：
- STM32 HAL: `HAL_GPIO_ReadPin()`
- STM32 标准库: `GPIO_ReadInputDataBit()`
- ESP32: `gpio_get_level()`
- 51单片机: `P1_0`

#### 编码器函数（可选，当前未启用）

```c
void Encoder_Init(void);      // 初始化编码器，不用就留空
void Encoder_Enable(void);    // 使能，不用就留空
void Encoder_Disable(void);   // 失能，不用就留空
int16_t Encoder_Get(void);    // 返回增量值，不用就 return 0
```

#### 延时函数

```c
void Delay_ms(uint32_t xms);  // 毫秒延时
void Delay_s(uint32_t xs);    // 秒延时
```

移植时改成你的平台的延时函数。

#### 定时器初始化（可选）

```c
void Timer_Init(void);  // 如果定时器已在别处初始化，留空即可
void Key_Init(void);    // 如果 GPIO 已在别处初始化，留空即可
```

### 2.2 dri_oled — OLED 驱动层

这个文件负责和屏幕通信。你需要实现以下函数：

#### 初始化和刷新

```c
void OLED_Init(void);           // 初始化 OLED 屏幕
void OLED_Refresh(void);        // 将显存 GRAM_BUF 数据发送到屏幕
void OLED_Clear(uint8_t clr);   // 清屏
void OLED_Brightness(int16_t Brightness);  // 设置亮度 (0-255)
```

#### 显存缓冲区

```c
extern uint8_t OLED_GRAM_BUF[OLED_HEIGHT/8][OLED_WIDTH];
// 例如 128x64 屏幕: OLED_GRAM_BUF[8][128]
// 每个 bit 代表一个像素，按页排列
```

所有绘图函数都是在这个缓冲区上操作，最后通过 `OLED_Refresh()` 一次性发送到屏幕。

#### 文字显示

```c
void OLED_ShowChar(int16_t X, int16_t Y, char Char, uint8_t FontSize);
void OLED_ShowString(int16_t X, int16_t Y, char *String, uint8_t FontSize);
void OLED_ShowMixString(int16_t X, int16_t Y, char *String, uint8_t ChineseFontSize, uint8_t ASCIIFontSize);
void OLED_Printf(int16_t X, int16_t Y, uint8_t FontSize, char *format, ...);
void OLED_PrintfMix(int16_t X, int16_t Y, uint8_t ChineseFontSize, uint8_t ASCIIFontSize, const char *format, ...);
```

#### 带区域裁剪的文字显示（UI 框架主要用这些）

```c
void OLED_PrintfMixArea(int16_t RangeX, int16_t RangeY, int16_t RangeWidth, int16_t RangeHeight,
                        int16_t X, int16_t Y, uint8_t ChineseFontSize, uint8_t ASCIIFontSize, char *format, ...);
void OLED_ShowStringArea(int16_t RangeX, int16_t RangeY, int16_t RangeWidth, int16_t RangeHeight,
                         int16_t X, int16_t Y, char *String, uint8_t FontSize);
void OLED_ShowImageArea(int16_t X_Pic, int16_t Y_Pic, int16_t PictureWidth, int16_t PictureHeight,
                        int16_t X_Area, int16_t Y_Area, int16_t AreaWidth, int16_t AreaHeight, const uint8_t *Image);
```

这些函数只在指定矩形区域内绘制，超出部分被裁剪。UI 框架的菜单滚动、窗口弹出都依赖这个功能。

#### 绘图函数

```c
void OLED_DrawPoint(int16_t X, int16_t Y);
void OLED_DrawLine(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1);
void OLED_DrawRectangle(int16_t X, int16_t Y, int16_t Width, int16_t Height, uint8_t IsFilled);
void OLED_DrawRoundedRectangle(int16_t X, int16_t Y, int16_t Width, int16_t Height, int16_t Radius, uint8_t IsFilled);
void OLED_DrawCircle(int16_t X, int16_t Y, int16_t Radius, uint8_t IsFilled);
```

#### 其他

```c
void OLED_ReverseArea(int16_t X, int16_t Y, int16_t Width, int16_t Height);  // 区域反色
void OLED_SetColorMode(bool colormode);  // 设置颜色模式（深色/浅色）
```

---

## 三、不需要移植的文件（直接复用）

### 3.1 OLED_UI.c — UI 引擎

核心文件，包含菜单渲染、动画系统、输入处理。**不需要修改任何代码**。

#### 关键全局变量

```c
MenuPage*  CurrentMenuPage;     // 当前显示的菜单页
MenuWindow *CurrentWindow;      // 当前弹出的窗口（NULL=无窗口）
bool ColorMode;                 // DARKMODE(true)=深色, LIGHTMODE(false)=浅色
int16_t OLED_UI_Brightness;     // 屏幕亮度 0-255
bool OLED_UI_ShowFps;           // 是否显示帧率
```

#### 核心 API

```c
void OLED_UI_Init(MenuPage* Page);       // 初始化，传入首页指针
void OLED_UI_MainLoop(void);             // 主循环中调用
void OLED_UI_InterruptHandler(void);     // 定时器中断中调用，周期 20ms
void OLED_UI_Back(void);                 // 返回上一级菜单
void OLED_UI_CreateWindow(MenuWindow* window);  // 弹出窗口
```

#### 动画系统

框架用两种算法实现平滑移动，在 `MenuPage.General_MoveStyle` 中选择：

- `UNLINEAR` — 非线性缓动，`当前位置 += 0.02 * 速度 * 误差`，越近越慢
- `PID_CURVE` — PID 控制器，产生弹簧效果（有惯性和回弹）

速度由 `MenuPage.General_MovingSpeed` 控制，值越大越快。

### 3.2 OLED_UI_MenuData.c — 菜单数据

定义菜单的结构和行为，修改这个文件来添加/删除菜单项。

---

## 四、数据结构详解

### 4.1 MenuPage — 菜单页

每一页菜单对应一个 `MenuPage` 结构体：

```c
typedef struct MenuPage {
    // ===== 通用属性 =====
    MenuStyle General_MenuType;           // 菜单类型: MENU_TYPE_LIST(0) 或 MENU_TYPE_TILES(1)
    MenuMovingSpeed General_MovingSpeed;  // 移动速度，推荐值 4
    uint8_t General_CursorStyle;          // 光标样式（见下方常量表）
    uint8_t General_MoveStyle;            // 移动算法: UNLINEAR(0) 或 PID_CURVE(1)
    OLED_Font General_FontSize;           // 字号: OLED_UI_FONT_8/12/16/20
    struct MenuPage* General_ParentMenuPage;  // 父菜单指针（返回用），顶层为 NULL
    struct MenuItem* General_MenuItems;       // 菜单项数组指针
    int16_t General_LineSpace;            // 行间距（像素）
    void (*General_ShowAuxiliaryFunction)(void);  // 辅助绘制函数，每帧调用，不需要则 NULL

    // ===== 列表模式特有属性 =====
    OLED_Area List_MenuArea;     // 列表显示区域 {x, y, width, height}
    bool List_IfDrawFrame;       // 是否绘制边框
    bool List_IfDrawLinePerfix;  // 是否显示行前缀符号（>, ~, *, -）
    int16_t List_StartPointX;    // 列表文字起始 X 偏移
    int16_t List_StartPointY;    // 列表文字起始 Y 偏移

    // ===== 磁贴模式特有属性 =====
    int16_t Tiles_ScreenWidth;   // 屏幕宽度
    int16_t Tiles_ScreenHeight;  // 屏幕高度
    int16_t Tiles_TileWidth;     // 图标宽度
    int16_t Tiles_TileHeight;    // 图标高度
} MenuPage;
```

#### 光标样式常量

| 常量                     | 值  | 说明         |
| ------------------------ | --- | ------------ |
| `REVERSE_RECTANGLE`      | 0   | 矩形反色     |
| `REVERSE_ROUNDRECTANGLE` | 1   | 圆角矩形反色 |
| `HOLLOW_RECTANGLE`       | 2   | 空心矩形     |
| `HOLLOW_ROUNDRECTANGLE`  | 3   | 空心圆角矩形 |
| `REVERSE_BLOCK`          | 4   | 下划线       |
| `NOT_SHOW`               | 5   | 不显示光标   |

#### 菜单类型常量

| 常量              | 值  | 说明          |
| ----------------- | --- | ------------- |
| `MENU_TYPE_LIST`  | 0   | 列表菜单      |
| `MENU_TYPE_TILES` | 1   | 磁贴/图标菜单 |

### 4.2 MenuItem — 菜单项

每个菜单项是 `MenuItem` 数组中的一个元素：

```c
typedef struct MenuItem {
    // ===== 通用属性 =====
    char* General_item_text;              // 显示的文字，最后一个元素必须为 NULL
    void (*General_callback)(void);       // 点击后的回调函数，无则 NULL
    MenuPage* General_SubMenuPage;        // 子菜单指针，无则 NULL

    // ===== 列表模式特有 =====
    bool* List_BoolRadioBox;     // 绑定的布尔变量（显示开关），无则 NULL
    int16_t* List_IntBox;        // 绑定的整数变量（显示数值），无则 NULL
    float* List_FloatBox;        // 绑定的浮点变量（显示数值），无则 NULL

    // ===== 磁贴模式特有 =====
    const uint8_t* Tiles_Icon;           // 静态图标数据，无则 NULL
    const uint8_t (*Tiles_GifIcon)[128]; // GIF 图标数据（32帧），无则 NULL
} MenuItem;
```

### 4.3 MenuWindow — 弹出窗口

```c
typedef struct MenuWindow {
    // ===== 通用属性 =====
    int16_t General_Width;        // 窗口宽度
    int16_t General_Height;       // 窗口高度
    float General_ContinueTime;   // 窗口持续时间（秒）
    uint8_t General_WindowType;   // 窗口类型: WINDOW_RECTANGLE(0) 或 WINDOW_ROUNDRECTANGLE(1)

    // ===== 文本属性（可选）=====
    char* Text_String;            // 窗口标题文字
    OLED_Font Text_FontSize;      // 字号
    int16_t Text_FontSideDistance; // 文字距窗口边缘的距离
    int16_t Text_FontTopDistance;  // 文字距窗口顶部的距离

    // ===== 数据调节属性（可选）=====
    float* Prob_Data_Float;       // 绑定的浮点数据指针
    int16_t* Prob_Data_Int;       // 绑定的整数数据指针
    float Prob_DataStep;          // 每次调节的步长
    float Prob_MinData;           // 最小值
    float Prob_MaxData;           // 最大值
    int16_t Prob_BottomDistance;  // 进度条距底部的距离
    int16_t Prob_LineHeight;      // 进度条高度
    int16_t Prob_SideDistance;    // 进度条距边缘的距离
} MenuWindow;
```

### 4.4 动画结构体

```c
// 可变区域（光标、菜单边框、窗口用）
typedef struct OLED_ChangeArea {
    OLED_Area CurrentArea;   // 当前位置
    OLED_Area TargetArea;    // 目标位置
    OLED_Area Error;         // 误差值
    OLED_Area LastError;     // 上次误差
    OLED_Area Integral;      // 积分值
    OLED_Area Derivative;    // 微分值
} OLED_ChangeArea;

// 可变坐标（页面起始点用）
typedef struct OLED_ChangePoint {
    OLED_Point CurrentPoint;
    OLED_Point TargetPoint;
    OLED_Point Error, LastError, Integral, Derivative;
} OLED_ChangePoint;

// 可变距离（行间距、滚动条高度用）
typedef struct OLED_ChangeDistance {
    float CurrentDistance, TargetDistance;
    float Error, LastError, Integral, Derivative;
} OLED_ChangeDistance;
```

---

## 五、运行流程

### 5.1 初始化流程

```
main()
  → OLED_UI_Init(&MainMenuPage)
      → OLED_Init()              // 初始化屏幕
      → Timer_Init()             // 初始化定时器（留空或自定义）
      → Key_Init()               // 初始化按键 GPIO
      → Encoder_Init()           // 初始化编码器（可选）
      → CurrentMenuPage = Page   // 设置首页
      → CurrentMenuPageInit()    // 初始化首页动画参数
```

### 5.2 主循环流程（每帧调用）

```
OLED_UI_MainLoop()
  → 读取按键状态 (OLED_KeyAndEncoderRecord)
  → 处理按键事件
      → 上/下键: MenuItemsMoveUp/Down() 移动光标
      → Enter键: EnterEventMenuItem() 进入子菜单或执行回调
      → Back键:  BackEventMenuItem() 返回父菜单
  → 移动所有动画元素 (MoveMenuElements)
      → ChangePoint()  // 页面起始点动画
      → ChangeArea()   // 光标、菜单框动画
      → ChangeDistance() // 行间距、滚动条动画
  → 绘制当前菜单 (PrintMenuElements)
      → 列表模式: 绘制文字、前缀、光标、滚动条
      → 磁贴模式: 绘制图标、箭头、文字
  → 绘制窗口 (OLED_DrawWindow)
  → 显示 FPS (OLED_UI_ShowFPS)
  → OLED_Refresh()  // 刷新到屏幕
```

### 5.3 定时器中断流程（每 20ms 调用一次）

```
OLED_UI_InterruptHandler()
  → GetFPS()  // 计算帧率
```

这个函数很轻量，主要工作在 `OLED_UI_MainLoop()` 中完成。

---

## 六、如何添加新菜单

### 6.1 添加列表菜单

在 `OLED_UI_MenuData.c` 中：

```c
// 第1步：定义菜单项数组
MenuItem MyMenuItems[] = {
    {.General_item_text = "Item1", .General_callback = NULL, .General_SubMenuPage = NULL},
    {.General_item_text = "Item2", .General_callback = MyCallback, .General_SubMenuPage = NULL},
    {.General_item_text = "[返回]", .General_callback = OLED_UI_Back, .General_SubMenuPage = NULL},
    {.General_item_text = NULL},  // 结束标记
};

// 第2步：定义菜单页
MenuPage MyMenuPage = {
    .General_MenuType = MENU_TYPE_LIST,
    .General_CursorStyle = REVERSE_ROUNDRECTANGLE,
    .General_FontSize = OLED_UI_FONT_12,
    .General_ParentMenuPage = &MainMenuPage,  // 父菜单
    .General_LineSpace = 4,
    .General_MoveStyle = UNLINEAR,
    .General_MovingSpeed = 4,
    .General_ShowAuxiliaryFunction = NULL,
    .General_MenuItems = MyMenuItems,

    .List_MenuArea = {0, 0, 128, 64},
    .List_IfDrawFrame = false,
    .List_IfDrawLinePerfix = true,
    .List_StartPointX = 4,
    .List_StartPointY = 2,
};

// 第3步：在父菜单的 MenuItem 中引用
// 在 MainMenuItems[] 中加一行：
{.General_item_text = "MyMenu", .General_callback = NULL, .General_SubMenuPage = &MyMenuPage, .Tiles_Icon = Image_xxx},
```

### 6.2 添加带开关的菜单项

```c
extern bool MySwitch;  // 声明一个布尔变量

MenuItem SettingsMenuItems[] = {
    // 绑定布尔变量，自动显示开/关
    {.General_item_text = "My Feature", .General_callback = NULL, .General_SubMenuPage = NULL, .List_BoolRadioBox = &MySwitch},
    {.General_item_text = NULL},
};
```

### 6.3 添加带数值显示的菜单项

```c
extern int16_t MyValue;

MenuItem SettingsMenuItems[] = {
    // 绑定整数变量，右侧显示数值，点击弹出调节窗口
    {.General_item_text = "My Value", .General_callback = MyValueWindow, .General_SubMenuPage = NULL, .List_IntBox = &MyValue},
    {.General_item_text = NULL},
};

// 配套的窗口定义
MenuWindow MyValueWindowData = {
    .General_Width = 80,
    .General_Height = 28,
    .Text_String = "调节数值",
    .Text_FontSize = OLED_UI_FONT_12,
    .Text_FontSideDistance = 4,
    .Text_FontTopDistance = 3,
    .General_WindowType = WINDOW_ROUNDRECTANGLE,
    .General_ContinueTime = 4.0,
    .Prob_Data_Int = &MyValue,
    .Prob_DataStep = 1,
    .Prob_MinData = 0,
    .Prob_MaxData = 100,
    .Prob_BottomDistance = 3,
    .Prob_LineHeight = 8,
    .Prob_SideDistance = 4,
};

void MyValueWindow(void) {
    OLED_UI_CreateWindow(&MyValueWindowData);
}
```

---

## 七、移植到其他芯片的步骤

### 第1步：让 OLED 屏幕能正常显示

确保你的 OLED 驱动能实现：
1. `OLED_Init()` — 初始化屏幕
2. `OLED_Refresh()` — 将显存数据发送到屏幕
3. `OLED_Clear()` — 清屏
4. 至少一种文字显示函数（推荐 `OLED_PrintfMixArea`）

如果用的不是 SSD1315/SSD1306，需要重写 `dri_oled.c` 中的通信部分。

### 第2步：实现硬件抽象层

修改 `OLED_UI_Driver.c` 和 `OLED_UI_Driver.h`：

1. 改 4 个按键读取宏为你的 GPIO 读取方式
2. 改 `Delay_ms()` 为你的平台延时函数
3. `Encoder_Init()`、`Encoder_Get()` 不用就留空
4. `Timer_Init()`、`Key_Init()` 不用就留空

### 第3步：配置定时器

需要一个 20ms 周期的定时器中断，在中断中调用 `OLED_UI_InterruptHandler()`。

以 STM32 HAL 为例：
```c
// 在 main.c 的定时器中断回调中
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    static uint16_t cnt = 0;
    if (htim->Instance == TIM1) {
        cnt++;
        if (cnt >= 20) {  // 1kHz 中断，20次 = 20ms
            cnt = 0;
            OLED_UI_InterruptHandler();
        }
    }
}
```

### 第4步：在 main() 中调用

```c
int main(void) {
    // 你的硬件初始化...

    OLED_UI_Init(&MainMenuPage);  // 初始化 UI

    while (1) {
        OLED_UI_MainLoop();  // 主循环
    }
}
```

### 第5步：配置屏幕参数

在 `dri_oled.h` 中修改：

```c
#define OLED_WIDTH  128   // 改为你的屏幕宽度
#define OLED_HEIGHT 64    // 改为你的屏幕高度
```

在 `OLED_UI_MenuData.c` 的 `MenuPage` 中同步修改：
```c
.Tiles_ScreenWidth = 128,   // 与 OLED_WIDTH 一致
.Tiles_ScreenHeight = 64,   // 与 OLED_HEIGHT 一致
```

---

## 八、移植到其他屏幕的注意事项

### SSD1306（I2C）
- 与 SSD1315 兼容，直接使用现有驱动
- 选择 `#define OLED_I2C_DRIVER` 或 `OLED_I2C_DRIVER_DMA`
- 修改 `OLED_ADD` 为 `0x78`（SA0=0）或 `0x7A`（SA0=1）

### SSD1306（SPI）
- 选择 `#define OLED_SPI_DRIVER` 或 `OLED_SPI_DRIVER_DMA`
- 引脚接法：D0=SCK, D1=MOSI, RST, DC, CS

### ST7789 / ILI9341 等彩色屏幕
- 需要完全重写 `dri_oled.c`
- 显存格式不同（SSD1315 是单色按页排列，彩色屏通常是 RGB565）
- 文字渲染需要重新实现
- 工作量较大，建议找一个兼容的 GUI 框架

### 不同尺寸的 SSD1306/SSD1315
- 128x32: 修改 `OLED_HEIGHT` 为 32
- 128x128: 修改 `OLED_HEIGHT` 为 128
- 64x48: 同时修改 `OLED_WIDTH` 和 `OLED_HEIGHT`
- 显存数组 `OLED_GRAM_BUF` 会自动根据宏调整大小

---

## 九、常用调试技巧

### 查看帧率
```c
OLED_UI_ShowFps = true;  // 在左上角显示帧率
```

### 切换深色/浅色模式
```c
ColorMode = LIGHTMODE;  // 浅色模式（白底黑字）
ColorMode = DARKMODE;   // 深色模式（黑底白字）
```

### 调整动画速度
在 `MenuPage` 中修改 `General_MovingSpeed`：
- 值越大越快
- 值为 0 时无动画，直接跳到目标位置

### 调整弹簧效果
在 `OLED_UI.c` 的 `ChangeFloatNum()` 中修改 PID 参数：
```c
float Kp = 0.02f * CurrentMenuPage->General_MovingSpeed;  // 比例系数
float Ki = 0.005f * CurrentMenuPage->General_MovingSpeed;  // 积分系数
float Kd = 0.002f;  // 微分系数
```

### 减少 Flash 占用
- 在 `dri_oled_font.c` 中删除不需要的字体数据
- 不需要中文显示时，可以只保留 ASCII 字体
- 不需要的菜单页面可以删除对应的 `MenuItem` 和 `MenuPage`










OLED_UI_MenuData.c — 菜单数据文件
这个文件其实非常简单，它只做一件事：定义菜单的"样子"和"行为"。核心就是两种结构体的反复使用。
核心结构体 1：MenuItem（菜单项）
每个 MenuItem 就是菜单里的一行，用指定初始化器（.字段 = 值）来配置：
Copy code to clipboard
// 以"设置"菜单项为例：
{.General_item_text = "Settings",              // 显示的文字
 .General_callback = NULL,                     // 点击后调用的函数
 .General_SubMenuPage = &SettingsMenuPage,     // 进入的子菜单
 .Tiles_Icon = Image_setings}                  // 磁贴模式下的图标
每个菜单项数组的最后一个元素必须是 {.General_item_text = NULL}，表示数组结束（类似字符串的 \0）。
核心结构体 2：MenuPage（菜单页）
每个 MenuPage 定义了一整页菜单的外观和行为，分两部分：
通用属性（所有菜单类型都有）：
? General_MenuType — MENU_TYPE_TILES（磁贴/图标）或 MENU_TYPE_LIST（列表）
? General_FontSize — 字号（8/12/16/20）
? General_ParentMenuPage — 父菜单指针（用于返回上一级）
? General_MenuItems — 指向 MenuItem 数组
? General_MoveStyle — UNLINEAR（缓动）或 PID_CURVE（弹簧效果）
? General_ShowAuxiliaryFunction — 每帧额外绘制的函数（如显示 LOGO）
类型特有属性（根据 MenuType 选择）：
? 磁贴模式：Tiles_ScreenWidth/Height、Tiles_TileWidth/Height
? 列表模式：List_MenuArea（显示区域）、List_IfDrawFrame（是否画边框）
菜单层级关系
Copy code to clipboard
MainMenuPage (磁贴)
├── SettingsMenuPage (列表)
│   ├── AboutThisDeviceMenuPage (列表)
│   └── AboutOLED_UIMenuPage (列表)
└── MoreMenuPage (列表)
    ├── Font8MenuPage / Font12 / Font16 / Font20 (列表)
    ├── LongMenuPage / SpringMenuPage / ... (列表)
    └── SmallAreaMenuPage (列表)
通过 General_SubMenuPage 和 General_ParentMenuPage 两个指针连接成树形结构。
?
OLED_UI.c — UI 引擎文件
这个文件是框架的核心，有 ~62KB，但你只需要理解几个关键机制：
1. 全局状态变量
Copy code to clipboard
MenuPage*  CurrentMenuPage;    // 当前显示的菜单页
MenuWindow *CurrentWindow;     // 当前弹出的窗口（NULL=无窗口）
MutexFlag KeyEnterFlag;        // Enter键互斥标志
MutexFlag FadeOutFlag;         // 淡出动画标志
bool ColorMode;                // 深色/浅色模式
2. 动画系统 — ChangeFloatNum()
这是整个框架最核心的函数，实现了两种平滑移动算法：
? UNLINEAR（非线性缓动）：当前位置 += 0.02 * 速度 * 误差，类似"越近越慢"的效果
? PID_CURVE（PID 控制器）：用比例+积分+微分三个参数控制移动，产生"弹簧"效果
所有 UI 元素的移动（光标、菜单滚动、窗口弹出）都通过这个函数驱动。ChangePoint()、ChangeArea()、ChangeDistance() 都是它的包装。
3. 主循环 — OLED_UI_MainLoop()
每 20ms 被 TIM1 中断调用一次，流程大致是：
1. 读取按键状态
2. 处理按键事件（上下移动光标、进入子菜单、返回）
3. 更新所有动画（通过 ChangeXxx() 系列函数）
4. 绘制当前菜单（调用 dri_oled 的绘图函数）
5. 刷新显示
4. 菜单初始化 — CurrentMenuPageInit()
进入新菜单页时调用，设置初始位置和动画参数：
? 列表模式：从屏幕右侧滑入
? 磁贴模式：从初始位置移动到屏幕中心
?
建议的理解方式
第一步：先看 OLED_UI_MenuData.c 里 MainMenuItems[] 和 MainMenuPage 这两个定义，搞清菜单数据是怎么配置的。
第二步：看 OLED_UI.c 里 OLED_UI_Init() 函数（约第 450 行），理解初始化流程。
第三步：看 ChangeFloatNum() 函数（约第 254 行），理解动画是怎么动起来的。
第四步：看 CurrentMenuPageInit() 函数（约第 363 行），理解进入新菜单时发生了什么。
不需要看的：OLED_UI_FadeOut_Masking()（淡出特效）、ReverseCoordinate()（光标绘制细节）、FPS 相关函数 — 这些都是装饰性的，不影响理解核心逻辑。
如果你想动手验证，可以在 OLED_UI_MenuData.c 里加一个新的菜单项，比如在 MainMenuItems[] 里加一行，然后编译烧录看效果，这是最快的理解方式。