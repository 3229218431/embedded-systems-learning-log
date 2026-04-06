/**
  ******************************************************************************
  * @file           : adc_bat.h
  * @brief          : 电池电压采集头文件
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
#ifndef _ADC_BAT_H_
#define _ADC_BAT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/**
  * @brief  电池电压采集任务
  * @param  无
  * @retval 无
  * @details 每500ms执行一次，用于读取电池电压并显示在OLED上
  */
void AdcReadTask(void);

#ifdef __cplusplus
}
#endif

#endif /* _ADC_BAT_H_ */
