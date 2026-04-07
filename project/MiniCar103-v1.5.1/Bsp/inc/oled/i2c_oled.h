/**
  ******************************************************************************
  * @file           : i2c_oled.h
  * @brief          : I2C OLED驱动头文件
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 浙江工业大学 2025-2035
  * All rights reserved.
  *
  * 本文件仅供学习使用，未经许可不得擅改，违者必究;
  * 浙江工业大学 信息科学与工程学院 人工智能专业
  * 源码地址：https://gitee.com/NEagle
  * 修改时间：2025/12/10
  * 版本： V1.0
  * 版权所有，禁止滥用
  * V1.0修改说明：
  *
  ******************************************************************************
  */

#ifndef _I2C_OLED_H_
#define _I2C_OLED_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* OLED I2C命令写入函数声明 */
void OLED_WriteCommand(uint8_t Command);

/* OLED I2C数据写入函数声明 */
void OLED_WriteData(uint8_t *Data, uint8_t Count);

/* OLED GPIO初始化函数声明 */
void OLED_GPIO_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* _I2C_OLED_H_ */
