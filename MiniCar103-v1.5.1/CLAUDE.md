# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

MiniCar103 is an STM32F103C8T6-based line-following car reference design for teaching embedded systems. It uses a time-slice polling software architecture rather than an RTOS. The hardware includes 8 infrared sensors, two DC motors with PWM control, OLED display, buttons, buzzer, and battery voltage monitoring.

## Build System

The project uses Keil MDK-ARM (uVision) as the primary IDE and build system.

- Project file: `MDK-ARM/MiniCar103.uvprojx`
- To build from the command line (requires Keil installed):
  ```bash
  UV4.exe -b MDK-ARM/MiniCar103.uvprojx -o build_log.txt
  ```
- To open and build in the Keil uVision GUI, double-click the `.uvprojx` file.
- The project was originally generated with STM32CubeMX (`MiniCar103.ioc`). Regenerate code by opening the `.ioc` file in STM32CubeMX and selecting "Generate Code". This will overwrite files in `Core/` and `Drivers/`. User code is preserved between `USER CODE BEGIN` and `USER CODE END` comments.

## Code Architecture

### Layered Structure

1. **Core** (`Core/`): STM32CubeMX-generated HAL initialization, system clock, peripheral setup (`main.c`, `stm32f1xx_it.c`), and interrupt handlers. User modifications are in `USER CODE` sections.

2. **Board Support Package (BSP)** (`Bsp/`): Hardware abstraction layer.
   - `Board.h/.c`: Board-level initialization and macros for LEDs, buttons, sensors, buzzer.
   - `Motor.h/.c`: DC motor control with PWM (TIM3/TIM4) and direction GPIO.
   - `uart.h/.c`: UART1/UART3 drivers with FIFO buffering for interrupt-driven communication.
   - `adc_bat.h/.c`: Battery voltage monitoring via ADC1.
   - `oled/`: I2C OLED display driver with page-scan update.

3. **Application** (`App/`): Time-slice polling scheduler and task implementations.
   - `TaskMain.h/.c`: Task component definition and task array. Each task has a run flag, timer, interval, and function hook.
   - `UsrTimer.h/.c`: PID control for line following, triggered by TIM1 interrupt. Contains global variables for PID parameters, base speed, and tracking enable flag.

### Time‑Slice Polling Scheduler

- SysTick interrupt (1 ms) decrements timers for each task in `TaskComps[]`. When a timer reaches zero, the task's `Run` flag is set and the timer is reloaded with its interval.
- Main loop (`main.c`) scans `TaskComps[]` and calls `TaskHook()` for any task with `Run == 1`, then clears the flag.
- Task intervals are specified in milliseconds (e.g., 4 ms for key scanning, 100 ms for UART checks, 1000 ms for LED toggling).

### PID Line‑Following Control

- Real‑time control runs in the `HAL_TIM_PeriodElapsedCallback` for TIM1 (configured for 1 kHz update rate).
- Reads 8 infrared sensors, computes weighted error, and performs PID calculation.
- Outputs PWM duty cycles to left and right motors via `Motor_SetSpeed()`.
- Control can be enabled/disabled with the `g_track_enable` flag (toggled by KEY0).

### Key Global Variables

- `TaskComps[TASK_MAX]` – array of task descriptors (defined in `TaskMain.c`).
- `g_track_enable` – enables/disables line‑following PID.
- `g_speed_mode` / `baseSpeed` – three speed levels (low/mid/high) selectable with KEY1.
- `Kp`, `Ki`, `Kd` – PID tuning parameters (hard‑coded in `UsrTimer.c`).
- `motor_left`, `motor_right` – motor control structures (defined in `Motor.c`).

## Development Workflow

1. **Hardware changes** – Update `MiniCar103.ioc` in STM32CubeMX, regenerate code.
2. **Add/Modify tasks** – Edit `TaskMain.c`:
   - Add a new function with `void TaskFunc(void)` signature.
   - Insert an entry in `TaskComps[]` with `{0, initial_timer, interval, TaskFunc}`.
3. **Modify PID parameters** – Edit `Kp`, `Ki`, `Kd` in `UsrTimer.c`.
4. **Add hardware drivers** – Create new `.h/.c` files under `Bsp/inc` and `Bsp/src`, then include them in `Board.h`.
5. **Build and flash** – Use Keil uVision to compile and program via ST‑Link (or other debug probe).

## Important Notes

- The project is designed for teaching; it avoids an RTOS to keep the conceptual load lower.
- All user code must be placed between `USER CODE BEGIN` and `USER CODE END` comments in CubeMX‑generated files to survive regeneration.
- The OLED update function `OLED_Update_InPages()` is called as a task (every 10 ms) to refresh the display in a page‑scan manner.
- UART1 (PA9/PA10) is connected to a USB‑to‑serial converter for debugging printf output (redirected via `fputc` to UART3 by default).
- The battery voltage is sampled by ADC1 channel 8 (PB0) and displayed on the OLED by the `AdcReadTask` (every 500 ms).

## File Naming Conventions

- CubeMX‑generated files: `main.c`, `stm32f1xx_it.c`, `stm32f1xx_hal_msp.c`
- BSP modules: `<module>.h` in `Bsp/inc/`, `<module>.c` in `Bsp/src/`
- Application files: `TaskMain.c/h`, `UsrTimer.c/h`
- OLED driver files are under `Bsp/inc/oled/` and `Bsp/src/oled/`

## Common Pitfalls

- Do not modify the SysTick handler outside the `USER CODE` section; the timer‑decrement loop must remain.
- When adding new tasks, ensure the `TASK_MAX` macro in `TaskMain.h` matches the array size.
- PID parameters are tuned for a specific track and motor characteristics; adjust them empirically.
- The FIFO UART drivers use interrupts for reception; transmission is polling‑based.