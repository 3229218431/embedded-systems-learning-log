# CLAUDE_zh.md

本文档为 Claude Code (claude.ai/code) 在此代码库中工作时提供指导。

## 项目概述

MiniCar103 是一个基于 STM32F103C8T6 的循迹小车参考设计，用于嵌入式系统教学。它采用时间片轮询的软件架构而非 RTOS。硬件包括 8 路红外传感器、两路带 PWM 控制的直流电机、OLED 显示屏、按键、蜂鸣器和电池电压监测。

## 构建系统

项目使用 Keil MDK-ARM (uVision) 作为主要的集成开发环境和构建系统。

- 项目文件：`MDK-ARM/MiniCar103.uvprojx`
- 命令行构建（需要安装 Keil）：
  ```bash
  UV4.exe -b MDK-ARM/MiniCar103.uvprojx -o build_log.txt
  ```
- 在 Keil uVision GUI 中打开并构建：双击 `.uvprojx` 文件。
- 项目最初由 STM32CubeMX 生成（`MiniCar103.ioc`）。重新生成代码时，在 STM32CubeMX 中打开 `.ioc` 文件并选择“生成代码”。这将覆盖 `Core/` 和 `Drivers/` 目录中的文件。用户代码保存在 `USER CODE BEGIN` 和 `USER CODE END` 注释之间。

## 代码架构

### 分层结构

1. **核心层** (`Core/`)：STM32CubeMX 生成的 HAL 初始化、系统时钟、外设设置（`main.c`、`stm32f1xx_it.c`）和中断处理程序。用户修改在 `USER CODE` 节中。

2. **板级支持包 (BSP)** (`Bsp/`)：硬件抽象层。
   - `Board.h/.c`：板级初始化以及 LED、按键、传感器、蜂鸣器的宏定义。
   - `Motor.h/.c`：直流电机控制，使用 PWM (TIM3/TIM4) 和方向 GPIO。
   - `uart.h/.c`：UART1/UART3 驱动，采用 FIFO 缓冲实现中断驱动的通信。
   - `adc_bat.h/.c`：通过 ADC1 监测电池电压。
   - `oled/`：I2C OLED 显示屏驱动，支持分页扫描更新。

3. **应用层** (`App/`)：时间片轮询调度器和任务实现。
   - `TaskMain.h/.c`：任务组件定义和任务数组。每个任务包含运行标志、定时器、间隔和函数钩子。
   - `UsrTimer.h/.c`：循迹 PID 控制，由 TIM1 中断触发。包含 PID 参数、基础速度、循迹使能标志等全局变量。

### 时间片轮询调度器

- SysTick 中断（1 ms）递减 `TaskComps[]` 中每个任务的定时器。当定时器减到零时，任务的 `Run` 标志被置位，定时器重新加载间隔值。
- 主循环（`main.c`）扫描 `TaskComps[]`，对任何 `Run == 1` 的任务调用 `TaskHook()`，然后清除标志。
- 任务间隔以毫秒为单位指定（例如，按键扫描 4 ms，UART 检查 100 ms，LED 翻转 1000 ms）。

### PID 循迹控制

- 实时控制在 TIM1 的 `HAL_TIM_PeriodElapsedCallback` 中运行（配置为 1 kHz 更新速率）。
- 读取 8 路红外传感器，计算加权误差，执行 PID 运算。
- 通过 `Motor_SetSpeed()` 输出 PWM 占空比到左右电机。
- 控制可以通过 `g_track_enable` 标志启用/禁用（由 KEY0 切换）。

### 关键全局变量

- `TaskComps[TASK_MAX]` – 任务描述符数组（定义在 `TaskMain.c` 中）。
- `g_track_enable` – 启用/禁用循迹 PID。
- `g_speed_mode` / `baseSpeed` – 三个速度档位（低速/中速/高速），由 KEY1 选择。
- `Kp`, `Ki`, `Kd` – PID 调参参数（硬编码在 `UsrTimer.c` 中）。
- `motor_left`, `motor_right` – 电机控制结构体（定义在 `Motor.c` 中）。

## 开发流程

1. **硬件变更** – 在 STM32CubeMX 中更新 `MiniCar103.ioc`，重新生成代码。
2. **添加/修改任务** – 编辑 `TaskMain.c`：
   - 添加一个具有 `void TaskFunc(void)` 签名的新函数。
   - 在 `TaskComps[]` 中插入一项：`{0, 初始定时器, 间隔, TaskFunc}`。
3. **修改 PID 参数** – 编辑 `UsrTimer.c` 中的 `Kp`、`Ki`、`Kd`。
4. **添加硬件驱动** – 在 `Bsp/inc` 和 `Bsp/src` 下新建 `.h/.c` 文件，然后在 `Board.h` 中包含它们。
5. **构建和烧录** – 使用 Keil uVision 编译并通过 ST‑Link（或其他调试探头）编程。

## 重要说明

- 项目为教学而设计；为避免概念负担过重，没有使用 RTOS。
- 所有用户代码必须放在 CubeMX 生成文件的 `USER CODE BEGIN` 和 `USER CODE END` 注释之间，以便在重新生成时保留。
- OLED 更新函数 `OLED_Update_InPages()` 作为一个任务被调用（每 10 ms 一次），以分页扫描方式刷新显示。
- UART1 (PA9/PA10) 连接到 USB‑串口转换器用于调试 printf 输出（默认通过 `fputc` 重定向到 UART3）。
- 电池电压由 ADC1 通道 8 (PB0) 采样，并由 `AdcReadTask`（每 500 ms 一次）在 OLED 上显示。

## 文件命名约定

- CubeMX 生成的文件：`main.c`、`stm32f1xx_it.c`、`stm32f1xx_hal_msp.c`
- BSP 模块：`<模块名>.h` 位于 `Bsp/inc/`，`<模块名>.c` 位于 `Bsp/src/`
- 应用文件：`TaskMain.c/h`、`UsrTimer.c/h`
- OLED 驱动文件位于 `Bsp/inc/oled/` 和 `Bsp/src/oled/`

## 常见陷阱

- 不要在 `USER CODE` 节外修改 SysTick 处理程序；定时器递减循环必须保留。
- 添加新任务时，确保 `TaskMain.h` 中的 `TASK_MAX` 宏与数组大小匹配。
- PID 参数针对特定赛道和电机特性调校；请根据实际情况调整。
- FIFO UART 驱动使用中断接收；发送采用轮询方式。