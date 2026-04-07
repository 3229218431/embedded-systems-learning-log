/**
  ******************************************************************************
  * @file           : TaskMain.h
  * @brief          : 任务调度器头文件
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
#ifndef _TASKMAIN_H_
#define _TASKMAIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* 任务组件结构体定义 */
typedef struct _TASK_COMPONENTS
{
    unsigned char Run;        // 运行标志，0-未运行，1-运行就绪
    unsigned short Timer;     // 当前剩余定时器片值
    unsigned short ItvTime;   // 任务调度周期（毫秒）
    void (*TaskHook)(void);   // 任务函数指针
} TASK_COMPONENTS;

#define TASK_MAX 13  // 最大任务数量（增加蓝牙数据传输任务）

/* 蓝牙数据传输控制变量（在TaskMain.c中定义） */
extern volatile uint8_t g_bluetooth_data_enable;      // 蓝牙数据传输使能标志（1=启用，0=禁用）
extern volatile uint16_t g_bluetooth_data_interval;   // 蓝牙数据传输间隔（毫秒，默认50ms）

/* 任务列表外部声明 */
extern TASK_COMPONENTS TaskComps[];

/* 蓝牙数据传输任务函数声明 */
void BluetoothDataFunc(void);

#ifdef __cplusplus
}
#endif

#endif /* _TASKMAIN_H_ */
