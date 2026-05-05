# CLAUDE.md

本文件为 Claude Code (claude.ai/code) 在本仓库中工作时提供指导。

## 项目概述

STM32F103C8T6 (Blue Pill) 嵌入式项目，通过 SPI+DMA 驱动 128x64 SSD1315 OLED 显示屏，使用 OLED_UI 菜单框架 (v0.2.1, 来自 github.com/bdth-7777777/OLED_UI)。由 STM32CubeMX 6.17.0 生成。当前为裸机运行模式；FreeRTOS V10.3.1 已包含但在 main.c 中被注释掉。

## 构建系统

**主构建工具: Keil MDK-ARM V5** (ARMCC V5.06)
- 工程文件: `MDK-ARM/test.uvprojx`
- 编译宏定义: `USE_HAL_DRIVER,STM32F103xB`
- Keil uVision 中打开，F7 编译，F8 烧录

**备选: EIDE (VS Code 扩展)**
- 配置文件: `MDK-ARM/.eide/eide.yml`
- VS Code 任务: build, flash, build-and-flash, rebuild, clean（通过 EIDE 扩展）
- 烧录器: J-Link，8000 kHz

无 Makefile 或 CMakeLists.txt，所有构建均通过 IDE 完成。

## 代码格式化

Clang-format 配置位于 `MDK-ARM/.clang-format`：基于 Microsoft 风格，4 空格缩进，Linux 花括号风格，无列宽限制，C99 标准。在 `MDK-ARM/` 目录下运行 `clang-format`。

## 架构

### 代码分层

```
Core/Src/              STM32CubeMX 生成的 HAL 初始化 (gpio, spi, dma, tim, i2c)
                       不要手动编辑 USER CODE 块之外的代码
icode/oled/            OLED 硬件驱动层 (SSD1315, SPI+DMA 模式)
  dri_oled.c           底层驱动：绘图原语、文字、显示控制
  dri_oled_font.c      字体位图数据（中文 + ASCII，多种字号）
  OLED_UI_Driver.c     UI 硬件抽象：按键读取、编码器桩函数
icode/UI/              OLED UI 框架
  OLED_UI.c            核心引擎：菜单、PID 动画、窗口
  OLED_UI_MenuData.c   菜单结构定义和回调函数
icode/                 应用层胶水代码
  OLED_UI_Launcher.c   OLED UI 高层初始化/启动
Core/Src/main.c        入口点、时钟配置、TIM1 回调
```

### 关键运行流程

1. `main()` (Core/Src/main.c)：HAL 初始化 → 外设初始化 → TIM1 启动 (1kHz) → OLED/UI 初始化 → 循环调用 `OLED_UI_MainLoop()`
2. TIM1 中断频率 1 kHz；每 20 个计数 (20ms) 调用 `OLED_UI_InterruptHandler()`，驱动动画和输入轮询
3. OLED 通过 SPI2 + DMA1 通道 5 刷新（非阻塞）
4. 导航：4 个 GPIO 按键 (KEY0-KEY3, PA2-PA5)。编码器 (TIM2, PA0/PA1) 已配置但 `Encoder_Get()` 返回 0（桩函数）

### 硬件引脚映射

| 功能               | 引脚                          |
| ------------------ | ----------------------------- |
| OLED SPI2 SCK/MOSI | PB13, PB15                    |
| OLED RST/CS/DC     | PA8, PA9, PB12                |
| 按键 KEY0-KEY3     | PA2-PA5                       |
| 编码器 A/B         | PA0, PA1 (TIM2, 当前为桩函数) |
| I2C1 SCL/SDA       | PB6, PB7 (OLED 未使用)        |

### FreeRTOS 状态

FreeRTOS 内核文件已编译链接，但启动代码在 main.c 中被注释掉（约 127-131 行）。任务定义在 `Core/Src/freertos.c` 中，包含被注释的 OLED 代码。堆配置为 3072 字节 (heap_4)。TIM4 用作 HAL 时基（替代 SysTick），SysTick 保留给 FreeRTOS 使用。

## 重要开发模式

- CubeMX 生成的文件使用 `/* USER CODE BEGIN/END */` 注释块。只编辑这些块内的代码；块外的代码会在 CubeMX 重新生成时被覆盖。
- OLED_UI 框架拥有主循环控制权。添加自定义 UI：在 `icode/UI/OLED_UI_MenuData.c` 中定义菜单项，在 `icode/UI/OLED_UI.c` 中编写回调。
- OLED 驱动 (`dri_oled.c`) 支持 6 种通信模式。当前激活模式为 SPI+DMA，由 `#define OLED_SPI_DRIVER_DMA` 控制。
- `icode/test.c` 和 `icode/ui.c` 是供用户实验的占位模块。
