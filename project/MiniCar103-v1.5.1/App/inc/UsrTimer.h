/**
  ******************************************************************************
  * @file           : UsrTimer.h
  * @brief          : 用户定时器和PID循迹控制（含提前量补偿算法）
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 浙江工业大学 2025-2035
  * All rights reserved.
  *
  * 本文件仅供学习使用，未经许可不得擅改，违者必究;
  * 浙江工业大学 信息科学与工程学院 人工智能专业
  * 源码地址：https://gitee.com/NEagle
  * 修改时间：2026-04-01
  * 版本： V1.5.2
  * 版权所有，禁止滥用
  * V1.5.2修改说明：
  * - 新增提前量预测补偿算法
  * - 增加延迟补偿参数配置
  ******************************************************************************
  */

#ifndef _UsrTimer_H_
#define _UsrTimer_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include "Board.h"

/* PID参数和状态变量外部声明（在UsrTimer.c中定义） */
extern float Kp;                    // 比例系数
extern float Ki;                    // 积分系数
extern float Kd;                    // 微分系数
extern int baseSpeed;               // 基础速度（0-999）
extern int lastError;               // 上一次误差值
extern int integral;                // 误差积分值
extern volatile int leftSpeed;      // 左电机速度（PWM占空比0-999，蓝牙数据传输）
extern volatile int rightSpeed;     // 右电机速度（PWM占空比0-999，蓝牙数据传输）

/* 循迹使能标志 - 控制是否启用PID循迹控制 */
extern volatile uint8_t g_track_enable;     // 1=启用循迹控制,0=禁用

/* 速度模式：0=低速,1=中速,2=高速 */
extern uint8_t g_speed_mode;        // 默认中速

/* 提前量补偿算法参数 */
extern int16_t advance_compensation; // 提前量补偿值（-1000 ~ +1000）
extern int16_t predicted_error;      // 预测误差值
extern uint16_t delay_distance;      // 机械延迟距离（单位：0.1mm，2cm = 200）
extern uint16_t car_speed_estimate;  // 小车速度估计（单位：0.1mm/ms）

/* 延迟补偿开关 */
#define DELAY_COMPENSATION_ENABLE 1   // 1=启用延迟补偿,0=禁用

/* 传感器权重定义（固定） */
#define SENSOR_WEIGHTS_N 8
extern const int8_t sensor_weights[SENSOR_WEIGHTS_N];

/**
  * @brief  提前量预测控制算法初始化
  * @param  无
  * @retval 无
  * @details 初始化延迟补偿参数
  *          默认延迟距离：200 (表示20.0mm = 2.0cm)
  *          默认速度估计：根据速度模式设置
  */
void AdvanceCompensation_Init(void);

/**
  * @brief  计算提前量补偿值
  * @param  current_error: 当前误差值
  * @param  speed_percent: 当前速度百分比（0-999）
  * @retval 提前量补偿值（加到PID输出上）
  * @details 公式：
  *          delay_time = delay_distance / (speed_percent * speed_factor)
  *          predicted_error = current_error * (1 + delay_time * Kp_comp)
  *          advance_compensation = predicted_error - current_error
  */
int16_t Calculate_Advance_Compensation(int16_t current_error, int16_t speed_percent);

/**
  * @brief  应用提前量补偿的PID计算
  * @param  sensor: 传感器数组指针
  * @param  weights: 传感器权重数组指针
  * @retval 无
  * @details 在UsrTimer.c的HAL_TIM_PeriodElapsedCallback中调用
  */
void PID_Control_With_Advance(int sensor[8], const int8_t weights[8]);

/**
  * @brief  获取预测误差值（用于调试）
  * @param  无
  * @retval 预测误差值
  */
int16_t Get_Predicted_Error(void);

/**
  * @brief  设置延迟距离参数
  * @param  distance_mm: 延迟距离（单位：mm）
  * @retval 无
  * @details 例如：传感器距离主动轮2cm，则调用 Set_Delay_Distance(20)
  */
void Set_Delay_Distance(uint16_t distance_mm);

/**
  * @brief  设置预估速度参数
  * @param  speed: 预估速度（单位：0.1mm/ms）
  * @retval 无
  * @details 通常根据baseSpeed估计，例如baseSpeed=600时，
  *          估计速度约为3.0mm/ms，调用 Set_Speed_Estimate(30)
  */
void Set_Speed_Estimate(uint16_t speed);

#ifdef __cplusplus
}
#endif

#endif /* _UsrTimer_H_ */
