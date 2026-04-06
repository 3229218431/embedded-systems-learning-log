/**
  ******************************************************************************
  * @file           : UsrTimer.c
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

#include "UsrTimer.h"
#include "stdio.h"
#include "Board.h"
#include <stdlib.h>   // 用于abs函数

/* PID控制器参数 */
float Kp = 15.0f;   // 比例系数，越大响应越快但可能振荡
float Ki = 0.0f;    // 积分系数，用于消除稳态误差
float Kd = 5.0f;    // 微分系数，用于抑制振荡，提高稳定性

/* 基础速度和PID状态变量 */
int baseSpeed = 600;  // 基础速度，建议 >400（对应约40% PWM占空比）
int lastError = 0;    // 上一次的误差值，用于微分计算
int integral = 0;     // 误差积分值，用于积分控制

/* 调试标志 - 通过OLED和串口输出PID调试信息 */
#define DEBUG_PID 0  // 0=关闭调试输出,1=开启调试输出

/* 循迹使能标志 - 控制是否启用PID循迹控制 */
volatile uint8_t g_track_enable = 1;  // 1=启用循迹控制,0=禁用

/* 速度模式：0=低速,1=中速,2=高速 */
uint8_t g_speed_mode = 1;  // 默认中速

/* 蓝牙数据传输使用的电机速度变量 */
volatile int leftSpeed = 0;   // 左电机速度（PWM占空比0-999）
volatile int rightSpeed = 0;  // 右电机速度（PWM占空比0-999）

/* 提前量补偿算法变量 */
int16_t advance_compensation = 0;  // 提前量补偿值（加到PID输出上）
int16_t predicted_error = 0;       // 预测误差值
uint16_t delay_distance = 200;     // 机械延迟距离（单位：0.1mm，2cm = 200）
uint16_t car_speed_estimate = 30;  // 小车速度估计（单位：0.1mm/ms，约3.0mm/ms）

/* 传感器权重定义（固定） */
const int8_t sensor_weights[SENSOR_WEIGHTS_N] = {-2, -1, -1, 0, 0, 1, 1, 2};

/* 内部变量 */
static int16_t last_speed_percent = 0;  // 上次速度值（用于速度变化估计）
static int16_t speed_change_rate = 0;   // 速度变化率

/**
  * @brief  提前量预测控制算法初始化
  * @param  无
  * @retval 无
  * @details 初始化延迟补偿参数
  *          默认延迟距离：200 (表示20.0mm = 2.0cm)
  *          默认速度估计：根据速度模式设置
  * @note   调整延迟距离：根据实际机械安装位置设置
  *         传感器在主动轮前方2cm，则设置 delay_distance = 200
  */
void AdvanceCompensation_Init(void)
{
    delay_distance = 200;      // 2.0cm = 20.0mm = 200 (0.1mm单位)

    /* 根据速度模式设置估计速度 */
    switch(g_speed_mode) {
        case 0:  // 低速 40%
            car_speed_estimate = 20;  // 2.0mm/ms
            break;
        case 1:  // 中速 60%
            car_speed_estimate = 30;  // 3.0mm/ms（默认）
            break;
        case 2:  // 高速 80%
            car_speed_estimate = 40;  // 4.0mm/ms
            break;
    }

    last_speed_percent = baseSpeed;
    speed_change_rate = 0;
    advance_compensation = 0;
    predicted_error = 0;
}

/**
  * @brief  计算提前量补偿值
  * @param  current_error: 当前误差值
  * @param  speed_percent: 当前速度百分比（0-999）
  * @retval 提前量补偿值（加到PID输出上）
  * @details 公式说明：
  *          1. 延迟时间估计：delay_time = delay_distance / (speed * speed_factor)
  *             speed_factor = car_speed_estimate / 30 (基准速度因子)
  *          2. 预测误差：predicted_error = current_error * (1 + delay_factor)
  *             delay_factor = delay_time * Kp_comp
  *          3. 补偿值：advance_compensation = predicted_error - current_error
  */
int16_t Calculate_Advance_Compensation(int16_t current_error, int16_t speed_percent)
{
    int16_t compensation = 0;

    #if DELAY_COMPENSATION_ENABLE
    if (car_speed_estimate > 0 && delay_distance > 0) {
        /* 速度变化率估计（用于动态调整预测） */
        int16_t speed_delta = speed_percent - last_speed_percent;
        speed_change_rate = speed_delta;
        last_speed_percent = speed_percent;

        /* 计算速度因子（以基准速度30为参考） */
        uint16_t speed_factor = (car_speed_estimate > 0) ? car_speed_estimate : 30;

        /* 计算延迟时间对应的位移（单位：0.1mm） */
        uint16_t delay_time_equivalent = delay_distance;

        /* 计算延迟导致的误差放大系数
         * 基本思路：误差会随时间累积，提前量与误差和速度相关
         * delay_comp_factor ≈ delay_distance / (speed * 10)
         * 例如：delay=200, speed=30, delay_comp_factor ≈ 200/(30*10) = 0.67
         */
        int32_t delay_comp_factor = (int32_t)delay_time_equivalent * 100 / (speed_factor * 10);
        if (delay_comp_factor > 200) delay_comp_factor = 200;  // 限制最大补偿
        if (delay_comp_factor < 0) delay_comp_factor = 0;

        /* 根据误差方向决定提前量大小
         * 误差绝对值越大，提前量应该越大
         */
        int32_t predicted = current_error * 100;
        predicted += (predicted > 0 ? 1 : -1) * delay_comp_factor * (current_error > 0 ? current_error : -current_error) / 100;

        /* 简化的预测公式：提前量 = 误差 * 延迟系数 */
        int32_t advance_factor = delay_comp_factor;
        if (advance_factor > 100) advance_factor = 100;

        predicted = current_error + (current_error * advance_factor / 100);
        compensation = (int16_t)(predicted - current_error);

        /* 速度变化补偿（加速时提前更多，减速时减少提前量） */
        if (speed_change_rate > 50) {
            /* 加速时增加提前量 */
            compensation += compensation * speed_change_rate / 200;
        } else if (speed_change_rate < -50) {
            /* 减速时减少提前量 */
            compensation += compensation * speed_change_rate / 200;
        }

        /* 限制补偿范围 */
        if (compensation > 300) compensation = 300;
        if (compensation < -300) compensation = -300;

        /* 当前误差较小时，减少提前量（避免过度修正） */
        if (abs(current_error) < 2) {
            compensation = compensation * abs(current_error) / 2;
        }
    }
    #endif

    predicted_error = current_error + compensation;
    advance_compensation = compensation;

    return compensation;
}

/**
  * @brief  应用提前量补偿的PID控制
  * @param  sensor: 传感器数组指针
  * @param  weights: 传感器权重数组指针
  * @retval 无
  * @details 在UsrTimer.c的HAL_TIM_PeriodElapsedCallback中调用
  */
void PID_Control_With_Advance(int sensor[8], const int8_t weights[8])
{
    int error = 0;
    int activeCount = 0;

    /* 计算加权误差 */
    for (int i = 0; i < SENSOR_WEIGHTS_N; i++) {
        if (sensor[i]) {  // 如果传感器检测到黑线
            error += weights[i];
            activeCount++;
        }
    }

    /* 如果没有检测到线，停止电机 */
    if (activeCount == 0) {
        Motor_SetSpeed(&motor_left, 0);
        Motor_SetSpeed(&motor_right, 0);

        /* 调试输出 */
#if DEBUG_PID
        OLED_ShowNum(0, 0, 0, 3, OLED_8X16);
        OLED_ShowNum(0, 16, 0, 3, OLED_8X16);
        OLED_ShowNum(0, 32, 0, 3, OLED_8X16);
        printf("No line detected, stopping.\r\n");
#endif

        /* 重置PID积分项，避免积分饱和 */
        integral = 0;
        lastError = 0;
        advance_compensation = 0;
        predicted_error = 0;
        return;
    }

    /* PID计算 */
    integral += error;
    /* 积分限幅防止饱和 */
    if (integral > 1000) integral = 1000;
    if (integral < -1000) integral = -1000;

    int derivative = error - lastError;
    lastError = error;

    /* 计算PID输出 */
    int correction = (int)(Kp * error + Ki * integral + Kd * derivative);

    /* 计算并应用提前量补偿 */
    int comp = Calculate_Advance_Compensation(error, baseSpeed);
    correction += comp;

    /* 计算左右电机速度 */
    int leftSpeedLocal  = baseSpeed - correction;
    int rightSpeedLocal = baseSpeed + correction;

    /* 速度限幅（0-999对应0-99.9% PWM） */
    if (leftSpeedLocal < 0) leftSpeedLocal = 0;
    if (leftSpeedLocal > 999) leftSpeedLocal = 999;
    if (rightSpeedLocal < 0) rightSpeedLocal = 0;
    if (rightSpeedLocal > 999) rightSpeedLocal = 999;

    /* 更新全局速度变量（用于蓝牙数据传输） */
    leftSpeed = leftSpeedLocal;
    rightSpeed = rightSpeedLocal;

    /* 设置电机速度 */
    Motor_SetSpeed(&motor_left, leftSpeedLocal);
    Motor_SetSpeed(&motor_right, rightSpeedLocal);

    /* 调试输出 */
#if DEBUG_PID
    OLED_ShowNum(0, 0, error, 3, OLED_8X16);
    OLED_ShowNum(0, 16, leftSpeed, 3, OLED_8X16);
    OLED_ShowNum(0, 32, rightSpeed, 3, OLED_8X16);
    // OLED_ShowNum(0, 48, advance_compensation, 3, OLED_8X16);  // 显示提前量
    printf("Err:%d L:%d R:%d Comp:%d\r\n", error, leftSpeed, rightSpeed, advance_compensation);
#endif
}

/**
  * @brief  获取预测误差值（用于调试）
  * @param  无
  * @retval 预测误差值
  */
int16_t Get_Predicted_Error(void)
{
    return predicted_error;
}

/**
  * @brief  设置延迟距离参数
  * @param  distance_mm: 延迟距离（单位：mm）
  * @retval 无
  * @details 例如：传感器距离主动轮2cm，则调用 Set_Delay_Distance(20)
  */
void Set_Delay_Distance(uint16_t distance_mm)
{
    delay_distance = distance_mm * 10;  // 转换为0.1mm单位
}

/**
  * @brief  设置预估速度参数
  * @param  speed: 预估速度（单位：0.1mm/ms）
  * @retval 无
  * @details 通常根据baseSpeed估计，例如baseSpeed=600时，
  *          估计速度约为3.0mm/ms，调用 Set_Speed_Estimate(30)
  */
void Set_Speed_Estimate(uint16_t speed)
{
    car_speed_estimate = speed;
}

/**
  * @brief  定时器周期溢出回调函数（1ms触发）
  * @param  htim: 定时器句柄指针
  * @retval 无
  * @details 此函数在TIM1周期溢出时被HAL库调用
  *          配置为1ms周期，用于PID控制循环
  *
  * PID控制流程：
  * 1. 检查循迹使能标志
  * 2. 读取8个红外传感器值
  * 3. 计算加权误差值
  * 4. 执行PID计算（含提前量补偿）
  * 5. 限幅处理
  * 6. 设置左右电机速度
  *
  * 传感器权重分配（中心在传感器3和4之间）：
  * [-2, -1, -1, 0, 0, 1, 1, 2]
  * 传感器0（最左）权重为-2，传感器7（最右）权重为+2
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)  // 检查是否为TIM1定时器
    {
        /* 检查循迹使能标志 */
        if (!g_track_enable) {
            Motor_SetSpeed(&motor_left, 0);      // 停止左电机
            Motor_SetSpeed(&motor_right, 0);     // 停止右电机
            /* 重置PID状态，避免积分饱和 */
            integral = 0;
            lastError = 0;
            advance_compensation = 0;
            predicted_error = 0;
            return;
        }

        /* 读取8个红外传感器值 */
        int sensor[8];
        sensor[0] = GetSen0Val();  // 最左侧传感器
        sensor[1] = GetSen1Val();
        sensor[2] = GetSen2Val();
        sensor[3] = GetSen3Val();
        sensor[4] = GetSen4Val();
        sensor[5] = GetSen5Val();
        sensor[6] = GetSen6Val();
        sensor[7] = GetSen7Val();  // 最右侧传感器

        /* 使用提前量补偿的PID控制 */
        PID_Control_With_Advance(sensor, sensor_weights);
    }
}
