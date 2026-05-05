# Smooth-OLED_UI-menu

基于 STM32F103C8T6 (Blue Pill) 的 OLED 菜单 UI 移植项目，通过 SPI+DMA 驱动 128x64 SSD1315 OLED 显示屏。

## 项目介绍

本项目将 [OLED_UI](https://github.com/bdth-7777777/OLED_UI) 菜单框架移植到 STM32F103C8T6 平台。OLED_UI 是一个轻量级、丝滑流畅的嵌入式 OLED 菜单库，具备以下特性：

- **流畅的菜单动画**: 平滑滚动与过渡效果，操作响应自然
- **PID 调节控件**: 内置 PID 参数整定界面，适合控制类项目
- **多输入支持**: 支持按键与旋转编码器两种操作方式
- **低资源占用**: 专为资源受限的 MCU 设计，128x64 单色屏优化
- **易于扩展**: 菜单项与回调函数分离，新增页面只需填写数据结构

当前移植工作在 STM32F103C8T6 + SSD1315 OLED 上验证通过，使用 SPI+DMA 方式刷新屏幕，最大程度释放 CPU。

## 致谢

本项目移植自 [OLED_UI](https://github.com/bdth-7777777/OLED_UI) v0.2.1，感谢原作者 [bdth-7777777](https://github.com/bdth-7777777) 提供的优秀 OLED 菜单框架。

## 硬件

| 外设           | 引脚           |
| -------------- | -------------- |
| OLED SPI2 SCK  | PB13           |
| OLED SPI2 MOSI | PB15           |
| OLED RST       | PA8            |
| OLED CS        | PA9            |
| OLED DC        | PB12           |
| 按键 KEY0-3    | PA2-PA5        |
| 编码器 A/B     | PA0/PA1 (TIM2) |

## 软件

- **MCU**: STM32F103C8T6
- **HAL 库**: STM32CubeMX 6.17.0 生成
- **RTOS**: FreeRTOS V10.3.1 (已集成，默认注释)
- **编译器**: Keil MDK-ARM V5 (ARMCC V5.06)

## 构建与烧录

### Keil MDK

打开 `MDK-ARM/test.uvprojx`，F7 编译，F8 烧录。

### VS Code (EIDE)

安装 EIDE 扩展后使用预设的构建任务：build / flash / rebuild / clean。

## 目录结构

```
Core/Src/               HAL 初始化代码 (CubeMX 生成)
icode/oled/             OLED 底层驱动 (SSD1315 SPI+DMA)
icode/UI/               OLED UI 菜单框架
icode/                 应用胶水层
MDK-ARM/               Keil 工程文件
```

## 架构分层

```
Application (main.c / OLED_UI_Launcher.c)
    │  HAL_Init → 外设初始化 → OLED_Init → OLED_UI_Init
    │  while(1) { OLED_UI_MainLoop(); }
    │  TIM1 ISR (每20ms) → OLED_UI_InterruptHandler()
    ▼
UI Framework (OLED_UI.c)
    │  页面渲染、菜单导航、PID 动画、窗口管理
    ▼
UI Data (OLED_UI_MenuData.c)
    │  菜单页面结构体、回调函数、辅助渲染函数
    ▼
Porting Glue (OLED_UI_Driver.c)  ← 移植关键
    │  Key_GetXxx() → HAL_GPIO_ReadPin()
    │  Delay_ms()   → HAL_Delay()
    │  Encoder_Get()→ 编码器读取（当前为桩函数）
    ▼
OLED Driver (dri_oled.c)
    │  OLED_GRAM_BUF[8][128] 帧缓冲
    │  画点/线/矩形/圆/文字  →  写入缓冲区
    │  OLED_Refresh() → HAL_SPI_Transmit_DMA() 推送全屏
    ▼
STM32 HAL (CubeMX 生成)
    │  SPI2 + DMA1 / GPIO / TIM1
    ▼
Hardware (SSD1315 128x64 OLED / 按键 / 编码器)
```

## 移植教程

### 1. 移植— [OLED_UI_Driver.c](../icode/oled/OLED_UI_Driver.c)

这是整个移植最关键的文件，也是 **唯一需要对接 MCU 硬件的文件**。UI 框架对硬件的所有依赖被抽象为三个接口：

**按键读取** — 四个宏直接映射到 GPIO：

```c
#define Key_GetEnterStatus()  HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin)  // PA4
#define Key_GetBackStatus()   HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin)  // PA5
#define Key_GetUpStatus()     HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin)  // PA3
#define Key_GetDownStatus()   HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin)  // PA2
```

按键消抖由框架内部在 `OLED_UI_InterruptHandler()` 中通过 20ms 定时采样完成，无需外部处理。

**延时函数** — 封装 HAL 毫秒延时：

```c
void Delay_ms(uint32_t ms) {
    HAL_Delay(ms);
}
```

**编码器读取** — 当前为桩函数（返回 0），编码器接口已预留（PA0/PA1, TIM2），可接入旋转编码器实现滚轮操作。

```c
int16_t Encoder_Get(void) {
    return 0;  // 桩：接入编码器后返回增量值
}
```

> 移植到其他 MCU 时，只需修改此文件中三个接口的实现，其余代码无需改动。

### 2. OLED 底层驱动适配

`dri_oled.c` 通过宏开关支持多种通信模式，本项目使用 **SPI+DMA**，由 `#define OLED_SPI_DRIVER_DMA` 控制。与 HAL 的四个接触点：

| 操作          | HAL API                                                   |
| ------------- | --------------------------------------------------------- |
| 命令/数据发送 | `HAL_SPI_Transmit_DMA(&hspi2, ...)`                       |
| DC 引脚控制   | `OLED_DC_SET()` / `OLED_DC_CLR()` → `HAL_GPIO_WritePin()` |
| CS 引脚控制   | `OLED_CS_CLR()` / `OLED_CS_SET()`                         |
| RST 引脚控制  | `OLED_RST_CLR()` / `OLED_RST_SET()`                       |
| DMA 完成回调  | `HAL_SPI_TxCpltCallback()` → 自动拉高 CS 结束传输         |

若更换 SPI 外设或 MCU，只需修改 [dri_oled.h](../icode/oled/dri_oled.h) 中的引脚宏定义和 `hspi2` 引用。

### 3. 初始化顺序（必须严格遵循）

```
HAL_Init()                → 系统时钟配置 (72MHz, HSE 8MHz × 9 PLL)
MX_GPIO_Init()            → GPIO 初始化（OLED 控制引脚、按键）
MX_DMA_Init()             → DMA 初始化（必须在 SPI 之前）
MX_SPI2_Init()            → SPI2 外设初始化
MX_TIM1_Init()            → 定时器初始化 (1kHz 时基)
HAL_TIM_Base_Start_IT()   → 启动 TIM1 中断（20ms 心跳）
OLED_Init()               → SSD1315 初始化命令序列
OLED_UI_Init(&MainMenuPage) → UI 框架初始化
→ 进入 while(1) { OLED_UI_MainLoop(); }
```

### 4. 主循环与中断协作

系统采用**裸机超循环 + 定时中断**的协作模式：

- **主循环** (`OLED_UI_MainLoop`)：读取输入 → 处理导航 → 运行动画 → 渲染页面 → `OLED_Refresh()` DMA 刷屏 → 返回。每次调用完成一帧。
- **中断** (`TIM1`, 每 20ms)：调用 `OLED_UI_InterruptHandler()`，完成按键消抖、编码器累积、长按检测、FPS 计数。主循环下一次执行时消费这些状态。

### 5. 添加自定义菜单

在 [OLED_UI_MenuData.c](../icode/UI/OLED_UI_MenuData.c) 中：

1. 定义菜单页面结构体 `MenuPage MyPage`
2. 填充菜单项数组 `MenuItem MyItems[]`，指定文本、回调函数、子页面
3. 实现回调函数处理用户操作
4. 在主页面中引用新页面即可

```c
// 示例：添加一个带回调的菜单项
MenuItem MyItems[] = {
    {"选项1", NULL,          &SubPage,    SYSTEM_ICON_ENTER},
    {"选项2", MyCallback,    NULL,        SYSTEM_ICON_TOGGLE},
    {NULL},  // 结束标记
};
```

## 移植细节

### 按键映射

| 按键 | 引脚 | UI 功能 | 触发方式             |
| ---- | ---- | ------- | -------------------- |
| KEY0 | PA2  | 下      | 低电平有效，上拉输入 |
| KEY1 | PA3  | 上      | 同上                 |
| KEY2 | PA4  | 确认    | 同上                 |
| KEY3 | PA5  | 返回    | 同上                 |

### 屏幕刷新机制

- 帧缓冲区 `OLED_GRAM_BUF[8][128]`（8 页 × 128 列，对应 SSD1315 的页寻址模式）
- 所有绘制函数先写入帧缓冲，调用 `OLED_Refresh()` 时通过 **DMA 链式传输**一次性将 8 页数据推送到 OLED
- DMA 传输期间 CPU 可继续处理其他逻辑，实现异步刷新

### SPI 配置

- SPI2 主机模式，36 MHz（最大速率）
- CPOL=0, CPHA=0, MSB First
- DMA1 Channel 5 用于 SPI2_TX，高优先级

### FreeRTOS 状态

FreeRTOS 内核文件已加入工程，`Core/Src/freertos.c` 中预留了 OLED 任务代码（当前注释），`main.c` 中的 `osKernelInitialize()` / `MX_FREERTOS_Init()` / `osKernelStart()` 调用也处于注释状态。启用后需注意：
- SysTick 由 FreeRTOS 接管，TIM4 为 HAL 提供时基
- `OLED_UI_MainLoop()` 需从任务中调用
- TIM1 中断回调需使用队列/信号量传递按键事件给 UI 任务

## 后期规划

- [ ] 启用 FreeRTOS，将 OLED 刷新与菜单逻辑分离为独立任务
- [ ] 接入 I2C 传感器，通过菜单实时显示/配置传感器数据
- [ ] 添加更多 UI 控件（曲线图、仪表盘、数值输入框）
- [ ] 支持多级菜单页面切换与参数持久化存储
- [ ] 移植到更多 STM32 系列（F4/H7）及其他 MCU 平台
