/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : stm32f1xx_it.c
  * @brief          : 中断服务例程
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_it.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "TaskMain.h"    // 应用程序头文件
#include "Board.h"       // 板级支持包头文件

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim1;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M3处理器中断和异常处理函数                              */
/******************************************************************************/
/**
  * @brief 这个函数处理不可屏蔽中断（NMI）
  */
void NMI_Handler(void)
{
    /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

    /* USER CODE END NonMaskableInt_IRQn 0 */
    /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
    while (1) {
    }
    /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief 这个函数处理硬故障中断（Hard Fault）
  */
void HardFault_Handler(void)
{
    /* USER CODE BEGIN HardFault_IRQn 0 */

    /* USER CODE END HardFault_IRQn 0 */
    while (1) {
        /* USER CODE BEGIN W1_HardFault_IRQn 0 */
        /* USER CODE END W1_HardFault_IRQn 0 */
    }
}

/**
  * @brief 这个函数处理存储器管理故障中断（Memory Management Fault）
  */
void MemManage_Handler(void)
{
    /* USER CODE BEGIN MemoryManagement_IRQn 0 */

    /* USER CODE END MemoryManagement_IRQn 0 */
    while (1) {
        /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
        /* USER CODE END W1_MemoryManagement_IRQn 0 */
    }
}

/**
  * @brief 这个函数处理预取故障和存储器访问故障中断（Bus Fault）
  */
void BusFault_Handler(void)
{
    /* USER CODE BEGIN BusFault_IRQn 0 */

    /* USER CODE END BusFault_IRQn 0 */
    while (1) {
        /* USER CODE BEGIN W1_BusFault_IRQn 0 */
        /* USER CODE END W1_BusFault_IRQn 0 */
    }
}

/**
  * @brief 这个函数处理未定义指令或非法状态中断（Usage Fault）
  */
void UsageFault_Handler(void)
{
    /* USER CODE BEGIN UsageFault_IRQn 0 */

    /* USER CODE END UsageFault_IRQn 0 */
    while (1) {
        /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
        /* USER CODE END W1_UsageFault_IRQn 0 */
    }
}

/**
  * @brief 这个函数处理系统服务调用中断（SVC）
  */
void SVC_Handler(void)
{
    /* USER CODE BEGIN SVCall_IRQn 0 */

    /* USER CODE END SVCall_IRQn 0 */
    /* USER CODE BEGIN SVCall_IRQn 1 */

    /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief 这个函数处理调试monitor中断（Debug Monitor）
  */
void DebugMon_Handler(void)
{
    /* USER CODE BEGIN DebugMonitor_IRQn 0 */

    /* USER CODE END DebugMonitor_IRQn 0 */
    /* USER CODE BEGIN DebugMonitor_IRQn 1 */

    /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief 这个函数处理系统服务请求中断（PendSV）
  */
void PendSV_Handler(void)
{
    /* USER CODE BEGIN PendSV_IRQn 0 */

    /* USER CODE END PendSV_IRQn 0 */
    /* USER CODE BEGIN PendSV_IRQn 1 */

    /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief 这个函数处理系统滴答定时器中断（SysTick）
  * @details 每1ms触发一次，用于：
  *          1. HAL库滴答计数递增
  *          2. 任务调度：递减各任务定时器，当定时器为0时设置任务运行标志
  */
void SysTick_Handler(void)
{
    /* USER CODE BEGIN SysTick_IRQn 0 */
    unsigned short i;

    /* USER CODE END SysTick_IRQn 0 */
    HAL_IncTick();  // HAL库滴答计数递增
    /* USER CODE BEGIN SysTick_IRQn 1 */
    // 每1ms递减所有任务的定时器片值
    for(i = 0; i < TASK_MAX; i++) {   // 扫描所有任务
        if(TaskComps[i].Timer) {      // 当前任务定时器未到
            TaskComps[i].Timer --;    // 定时器片值减一
            if(TaskComps[i].Timer == 0) {   // 定时器片值归零
                TaskComps[i].Timer = TaskComps[i].ItvTime;  // 重新加载定时器片值
                TaskComps[i].Run = 1;       // 设置任务运行标志
            }
        }
    }

    /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F1xx 外设中断处理函数                                               */
/* 在此处添加所使用外设的中断处理函数                                       */
/* 可用的外设中断处理函数名称请参考启动文件（startup_stm32f1xx.s）            */
/******************************************************************************/

/**
  * @brief 这个函数处理ADC1和ADC2全局中断
  */
void ADC1_2_IRQHandler(void)
{
    /* USER CODE BEGIN ADC1_2_IRQn 0 */

    /* USER CODE END ADC1_2_IRQn 0 */
    HAL_ADC_IRQHandler(&hadc1);
    /* USER CODE BEGIN ADC1_2_IRQn 1 */

    /* USER CODE END ADC1_2_IRQn 1 */
}

/**
  * @brief 这个函数处理TIM1更新中断
  * @details 用于PID循迹控制，每1ms触发一次
  *          在UsrTimer.c的HAL_TIM_PeriodElapsedCallback中实现具体控制逻辑
  */
void TIM1_UP_IRQHandler(void)
{
    /* USER CODE BEGIN TIM1_UP_IRQn 0 */

    /* USER CODE END TIM1_UP_IRQn 0 */
    HAL_TIM_IRQHandler(&htim1);
    /* USER CODE BEGIN TIM1_UP_IRQn 1 */

    /* USER CODE END TIM1_UP_IRQn 1 */
}

/**
  * @brief 这个函数处理USART1全局中断
  */
void USART1_IRQHandler(void)
{
    /* USER CODE BEGIN USART1_IRQn 0 */

    /* USER CODE END USART1_IRQn 0 */
    HAL_UART_IRQHandler(&huart1);
    /* USER CODE BEGIN USART1_IRQn 1 */

    /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief 这个函数处理USART3全局中断
  */
void USART3_IRQHandler(void)
{
    /* USER CODE BEGIN USART3_IRQn 0 */

    /* USER CODE END USART3_IRQn 0 */
    HAL_UART_IRQHandler(&huart3);
    /* USER CODE BEGIN USART3_IRQn 1 */

    /* USER CODE END USART3_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
