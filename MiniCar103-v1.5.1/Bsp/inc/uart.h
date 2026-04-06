/**
  ******************************************************************************
  * @file           : uart.h
  * @brief          : UART业务封装头文件
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
#ifndef _UART_H_
#define _UART_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include "stdio.h"
#include "FIFO.h"

/* UART FIFO缓冲区大小配置 */
#define USART_REC_LEN 128   // 接收缓冲区大小（字节）
#define USART_TRS_LEN 128   // 发送缓冲区大小（字节）

/* UART串口FIFO句柄结构体定义 */
typedef struct {
    UART_HandleTypeDef *huart;        // HAL UART句柄指针
    fifo_t tx_fifo;                   // 发送FIFO
    fifo_t rx_fifo;                   // 接收FIFO
    uint8_t tx_buf[USART_TRS_LEN];    // 发送缓冲区
    uint8_t rx_buf[USART_REC_LEN];    // 接收缓冲区
    volatile bool tx_busy;            // 标志发送器是否忙碌（正在使用DMA或中断）
} uart_fifo_handle_t;

/* UART FIFO句柄外部声明 */
extern uart_fifo_handle_t g_uart1;    // UART1句柄
extern uart_fifo_handle_t g_uart3;    // UART3句柄

/* UART初始化函数声明 */
void InitMwUart1(void);  // UART1 HAL初始化
void InitMwUart3(void);  // UART3 HAL初始化

/* UART FIFO操作函数声明 */
int uart_fifo_put(uart_fifo_handle_t *handle, uint8_t ch);          // 单字节写入
int uart_fifo_puts(uart_fifo_handle_t *handle, const uint8_t *buf, size_t len);  // 多字节写入
int uart_fifo_get(uart_fifo_handle_t *handle, uint8_t *ch);         // 单字节读取
int uart_fifo_gets(uart_fifo_handle_t *handle, uint8_t *data, size_t len);        // 多字节读取

#ifdef __cplusplus
}
#endif

#endif /* _UART_H_ */
