/**
  ******************************************************************************
  * @file           : Board.h
  * @brief          : 板级支持包HAL头文件
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 浙江工业大学 2025-2035
  * All rights reserved.
  *
  * 本文件仅供学习使用，未经许可不得擅改，违者必究;
  * 浙江工业大学 信息科学与工程学院 人工智能专业
  * 源码地址：https://gitee.com/NEagle
  * 修改时间：2025/12/06
  * 版本： V1.0
  * 版权所有，禁止滥用
  * V1.0修改说明：
  *
  ******************************************************************************
  */
#ifndef _BOARD_H_
#define _BOARD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* 外设驱动头文件包含 */
#include "uart.h"       // UART驱动
#include "i2c_oled.h"   // I2C OLED驱动
#include "oled_api.h"   // OLED API接口
#include "adc_bat.h"    // ADC电池采集
#include "Motor.h"      // 电机驱动

/* 核心板LED控制宏定义 */
#define CoreLedON()     HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET)
#define CoreLedOFF()    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET)
#define CoreLedToggle() HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13)

/* 扩展板LED控制宏定义 */
#define BrdLedON()      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET)
#define BrdLedOFF()     HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET)
#define BrdLedToggle()  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_11)

/* 蜂鸣器控制宏定义 */
#define BuzzON()        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET)
#define BuzzOFF()       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET)
#define BuzzToggle()    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_3)

/* 按键输入宏定义（低电平为按下） */
#define GetKey0Val()    (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_14) ? 0 : 1)
#define GetKey1Val()    (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) ? 0 : 1)

/* 红外传感器输入宏定义（低电平为检测到黑线） */
#define GetSen0Val()    (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) ? 0 : 1)  // 最左侧传感器
#define GetSen1Val()    (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) ? 0 : 1)
#define GetSen2Val()    (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) ? 0 : 1)
#define GetSen3Val()    (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) ? 0 : 1)
#define GetSen4Val()    (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) ? 0 : 1)
#define GetSen5Val()    (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) ? 0 : 1)
#define GetSen6Val()    (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) ? 0 : 1)
#define GetSen7Val()    (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) ? 0 : 1)  // 最右侧传感器

/**
  * @brief  板级初始化函数
  * @param  无
  * @retval 无
  * @details 初始化所有外设模块：
  *          - 串口通信
  *          - OLED显示
  *          - 电机驱动
  */
void BoardInit(void);

#ifdef __cplusplus
}
#endif

#endif /* _BOARD_H_ */
