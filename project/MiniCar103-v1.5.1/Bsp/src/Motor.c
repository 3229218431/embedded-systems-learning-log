/**
  ******************************************************************************
  * @file           : Motor.c
  * @brief          : 电机驱动C文件
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
#include "Motor.h"

/* PWM定时器句柄，用于电机速度控制 */
extern TIM_HandleTypeDef htim3;  // TIM3用于左电机PWM控制
extern TIM_HandleTypeDef htim4;  // TIM4用于右电机PWM控制

/* 左电机控制结构体实例 */
MotorControl_t motor_left = {
    .pwm_timer = &htim3,              // 指向TIM3定时器句柄
    .pwm_channel1 = TIM_CHANNEL_2,    // TIM3_CH2 -> PB4
};

/* 右电机控制结构体实例 */
MotorControl_t motor_right = {
    .pwm_timer = &htim4,              // 指向TIM4定时器句柄
    .pwm_channel1 = TIM_CHANNEL_1,    // TIM4_CH1 -> PB6
};

/* 系统时钟配置函数声明 */
void SystemClock_Config(void);        // 系统时钟配置函数声明
//static void MX_GPIO_Init(void);       // GPIO初始化函数声明
//static void MX_TIM3_Init(void);       // TIM3初始化函数声明
void HAL_MspInit(void);               // HAL MSP初始化函数声明
void HAL_TIM_MspInit(TIM_HandleTypeDef* tim_handle);  // TIM MSP初始化函数声明

/* 电机控制函数声明 */
void Motor_Init(MotorControl_t *motor);       // 电机初始化函数声明
void Motor_Stop(MotorControl_t *motor);       // 电机停止函数声明
float Motor_GetSpeedRPM(MotorControl_t *motor); // 获取电机转速函数声明
void Encoder_Reset(MotorControl_t *motor);    // 重置编码器计数函数声明

/* 编码器相关函数声明 */
void Encoder_Init(void);              // 编码器初始化函数声明
void Encoder_IRQHandler(void);        // 编码器中断处理函数声明
int8_t Encoder_GetDirection(uint8_t prev_a, uint8_t prev_b, uint8_t curr_a, uint8_t curr_b); // 获取编码器方向函数声明

/**
  * @brief  初始化所有电机
  * @param  无
  * @retval 无
  * @details 电机初始化过程：
  *          1. 停止左右电机
  *          2. 配置PWM输出引脚为低电平
  *          3. 启动定时器PWM输出
  */
void InitMotor(void)                    // 电机初始化函数
{
    /* 初始化电机，首先停止电机 */
    Motor_Stop(&motor_left);                  // 停止左电机
    Motor_Stop(&motor_right);                  // 停止右电机

    /* 配置PWM输出并启动定时器 */
    ResetLeftIn2();   // PB4设置为低电平
    HAL_TIM_PWM_Start(motor_left.pwm_timer, motor_left.pwm_channel1);  // 启动TIM3_CH2 PWM输出

    ResetRightIn2();  // PB7设置为低电平
    HAL_TIM_PWM_Start(motor_right.pwm_timer, motor_right.pwm_channel1);  // 启动TIM4_CH1 PWM输出
}

/**
  * @brief  设置电机转速
  * @param  motor: 电机控制结构体指针
  * @param  speed_percent: 转速百分比 (-999 到 +999)，正数为正转，负数为反转
  * @retval 无
  * @details 转速设置：
  *          - 输入范围：-999 到 +999
  *          - 实际转速范围：0-99.9%
  *          -PWM占空比直接对应转速百分比
  * @note  此版本暂未支持反转功能
  */
void Motor_SetSpeed(MotorControl_t *motor, int16_t speed_percent)  // 设置电机转速实现
{
    /* 限制转速范围 */
    if(speed_percent < 0) return;
    if(speed_percent > 999) speed_percent = 999;  // 转速上限为99.9%

    /* 设置PWM占空比 */
    __HAL_TIM_SET_COMPARE(motor->pwm_timer,motor->pwm_channel1, speed_percent);
}

/**
  * @brief  停止电机
  * @param  motor: 电机控制结构体指针
  * @retval 无
  * @details 将电机PWM占空比设置为0，使电机停止转动
  */
void Motor_Stop(MotorControl_t *motor)  // 停止电机实现
{
    /* 将PWM占空比设置为0 */
    __HAL_TIM_SET_COMPARE(motor->pwm_timer, motor->pwm_channel1, 0); // 设置PB4 PWM占空比为0
}
