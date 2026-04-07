/**
  ******************************************************************************
  * @file           : adc_bat.c
  * @brief          : 电池电压采集模块
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
#include "adc_bat.h"
#include "oled_api.h"

/* ADC转换完成标志 */
uint8_t Adc_Cplt_Flag = 0;

/* ADC采样值变量 */
uint32_t adc_value;

/* ADC电压转换系数，根据分压电路计算得出
   公式：实际电压 = ADC采样值 * ADCRatio
   系数5.228759765625考虑了分压电阻和参考电压
*/
#define ADCRatio 5.228759765625

/**
  * @brief  ADC转换完成回调函数
  * @param  hadc: ADC句柄指针
  * @retval 无
  * @details 当ADC转换完成时，该回调函数被HAL库自动调用
  *          - 获取ADC转换结果
  *          - 设置转换完成标志
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if(hadc->Instance == ADC1) {
        /* 获取ADC转换结果 */
        adc_value = HAL_ADC_GetValue(hadc);

        /* 设置ADC转换完成标志 */
        Adc_Cplt_Flag = 1;
    }
}

/* ADC_HandleTypeDef在main.c中定义 */
extern ADC_HandleTypeDef hadc1;

/**
  * @brief  电池电压采集任务
  * @param  无
  * @retval 无
  * @details 每500ms执行一次，用于：
  *          - 读取电池电压并显示在OLED上
  *          - 重新启动ADC转换
  *          - 显示位置：OLED第48行，从第40列开始
  */
void AdcReadTask(void)
{
    uint16_t AdcVal;

    /* 检查ADC转换完成标志 */
    if(Adc_Cplt_Flag) {
        /* 计算电池电压值（mV） */
        AdcVal = (int)(ADCRatio * adc_value);

        /* 在OLED上显示电池电压 */
        OLED_ShowNum(40, 48, AdcVal, 4, OLED_8X16);
    }

    /* 重新启动ADC中断转换 */
    HAL_ADC_Start_IT(&hadc1);

    /* 清除ADC转换完成标志 */
    Adc_Cplt_Flag = 0;
}
