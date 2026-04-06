/**
  ******************************************************************************
  * @file           : Motor.h
  * @brief          : 电机驱动头文件
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
#ifndef _MOTOR_H_
#define _MOTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* PWM输出端口定义 */
#define MOTOR_PWM_PORT    GPIOB       // PWM输出端口为GPIOB

/* 编码器输入端口定义 */
#define ENCODER_PORT      GPIOB       // 编码器输入端口为GPIOB

/* 左电机方向控制宏定义 */
#define SetLeftIn2()      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET)
#define ResetLeftIn2()    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET)

/* 右电机方向控制宏定义 */
#define SetRightIn2()     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET)
#define ResetRightIn2()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET)

/* PWM周期配置（可选） */
//#define PWM_PERIOD        1000  // PWM周期值，用于设置PWM频率

/* 电机控制结构体定义 */
typedef struct {
    TIM_HandleTypeDef *pwm_timer;     // PWM定时器句柄，用于设置PWM频率和占空比
    uint32_t pwm_channel1;            // PWM通道1 (PB4 - TIM3_CH2)
} MotorControl_t;                     // 电机控制结构体

/* 电机控制实例声明 */
extern MotorControl_t motor_left;     // 左电机控制结构体
extern MotorControl_t motor_right;    // 右电机控制结构体

/* 电机控制函数声明 */
void InitMotor(void);                                 // 电机初始化函数
void Motor_SetSpeed(MotorControl_t *motor, int16_t speed_percent);  // 设置电机转速函数

#ifdef __cplusplus
}
#endif

#endif /* _MOTOR_H_ */
