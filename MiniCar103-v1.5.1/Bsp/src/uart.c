/**
  ******************************************************************************
  * @file           : uart.c
  * @brief          : UART业务封装文件
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
#include "uart.h"

/* CubeMX生成的UART句柄 */
extern UART_HandleTypeDef huart1;  // UART1底层硬件句柄
extern UART_HandleTypeDef huart3;  // UART3底层硬件句柄

/* 接收中断字节变量 */
uint8_t Rx1Byte;  // UART1接收中断字节
uint8_t Rx3Byte;  // UART3接收中断字节

/* UART1的FIFO句柄结构体 */
uart_fifo_handle_t g_uart1 = {
    .huart = &huart1,
    .tx_busy = false
};

/* UART3的FIFO句柄结构体 */
uart_fifo_handle_t g_uart3 = {
    .huart = &huart3,
    .tx_busy = false
};

/* 支持stdio.h头文件对标准输入输出函数的支持 */
#pragma import(__use_no_semihosting)

/* 标准库需要支持的结构体 */
struct __FILE
{
    int handle;
};

FILE __stdout;

/* 避免使用半主机模式 */
void _sys_exit(int x)
{
    x = x;
}

/**
  * @brief  重定向 fputc 函数，支持 printf 通过 UART 输出
  * @param  ch: 要输出的字符
  * @param  f: 文件句柄
  * @retval 返回输出的字符
  * @details 该函数将 printf 的输出重定向到 UART3
  *          使用 FIFO 缓冲区提高输出效率
  */
int fputc(int ch, FILE *f)
{
    uart_fifo_put(&g_uart3, (uint8_t)ch);
    return ch;
}

/**
  * @brief  初始化 UART1
  * @param  无
  * @retval 无
  * @details 配置 UART1 的 FIFO 缓冲区并启动接收中断
  */
void InitMwUart1(void)
{
    fifo_init(&g_uart1.rx_fifo, g_uart1.rx_buf, USART_REC_LEN);
    fifo_init(&g_uart1.tx_fifo, g_uart1.tx_buf, USART_TRS_LEN);
    HAL_UART_Receive_IT(g_uart1.huart, &Rx1Byte, 1);
}

/**
  * @brief  初始化 UART3
  * @param  无
  * @retval 无
  * @details 配置 UART3 的 FIFO 缓冲区并启动接收中断
  */
void InitMwUart3(void)
{
    fifo_init(&g_uart3.rx_fifo, g_uart3.rx_buf, USART_REC_LEN);
    fifo_init(&g_uart3.tx_fifo, g_uart3.tx_buf, USART_TRS_LEN);
    HAL_UART_Receive_IT(g_uart3.huart, &Rx3Byte, 1);
}

/**
  * @brief  将单个字符放入 UART FIFO 发送缓冲区
  * @param  handle: UART FIFO 句柄指针
  * @param  ch: 要发送的字符
  * @retval 0 表示成功，-1 表示 FIFO 已满
  * @details 如果发送器忙碌，则将字符存入 FIFO；
  *          否则直接启动发送中断
  */
int uart_fifo_put(uart_fifo_handle_t *handle, uint8_t ch) {
    if (fifo_is_full(&handle->tx_fifo)) {
        return -1;  // FIFO 已满
    }
    fifo_write(&handle->tx_fifo, ch);

    /* 如果当前没有正在发送的数据，则启动发送 */
    if (!handle->tx_busy) {
        handle->tx_busy = true;
        uint8_t data;
        if (fifo_read(&handle->tx_fifo, &data) == 0) {
            HAL_UART_Transmit_IT(handle->huart, &data, 1);
        } else {
            handle->tx_busy = false;  // 应该不会发生
        }
    }
    return 0;
}

/**
  * @brief  将多个字符放入 UART FIFO 发送缓冲区
  * @param  handle: UART FIFO 句柄指针
  * @param  buf: 要发送的数据缓冲区指针
  * @param  len: 要发送的数据长度
  * @retval 实际写入 FIFO 的数据长度
  * @details 如果 FIFO 空间不足，则只写入可容纳的部分
  */
int uart_fifo_puts(uart_fifo_handle_t *handle, const uint8_t *buf, size_t len) {
    size_t wr_len;
    if (fifo_is_full(&handle->tx_fifo)) {
        return -1;  // FIFO 已满
    }
    wr_len = fifo_write_multi(&handle->tx_fifo, buf, len);
    if(wr_len <= 0)
        return 0;  // FIFO 未初始化

    /* 如果当前没有正在发送的数据，则启动发送 */
    if (!handle->tx_busy) {
        handle->tx_busy = true;
        uint8_t data;
        if (fifo_read(&handle->tx_fifo, &data) == 0) {
            HAL_UART_Transmit_IT(handle->huart, &data, 1);
        } else {
            handle->tx_busy = false;  // 应该不会发生
        }
    }
    return wr_len;
}

/**
  * @brief  从 UART FIFO 接收缓冲区读取单个字符
  * @param  handle: UART FIFO 句柄指针
  * @param  ch: 读取的字符存储地址
  * @retval 0 表示成功，-1 表示 FIFO 为空
  */
int uart_fifo_get(uart_fifo_handle_t *handle, uint8_t *ch) {
    return fifo_read(&handle->rx_fifo, ch);  // 返回 0 表示成功，-1 表示失败
}

/**
  * @brief  从 UART FIFO 接收缓冲区读取多个字符
  * @param  handle: UART FIFO 句柄指针
  * @param  data: 读取的数据存储缓冲区
  * @param  len: 要读取的数据长度
  * @retval 0 表示成功，-1 表示失败
  */
int uart_fifo_gets(uart_fifo_handle_t *handle, uint8_t *data, size_t len) {
    return fifo_read_multi(&handle->rx_fifo, data, len);  // 返回 0 表示成功，-1 表示失败
}

/**
  * @brief  UART 发送完成中断回调函数
  * @param  huart: UART 句柄指针
  * @retval 无
  * @details 当 FIFO 中还有数据时，继续发送下一批数据；
  *          如果 FIFO 为空，则 clears tx_busy 标志
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == g_uart1.huart) {
        uint8_t data;
        if (fifo_read(&g_uart1.tx_fifo, &data) == 0) {
            /* FIFO 中还有数据，继续发送 */
            HAL_UART_Transmit_IT(huart, &data, 1);
        } else {
            /* FIFO 为空，设置发送完成标志 */
            g_uart1.tx_busy = false;
        }
    }
    if (huart == g_uart3.huart) {
        uint8_t data;
        if (fifo_read(&g_uart3.tx_fifo, &data) == 0) {
            /* FIFO 中还有数据，继续发送 */
            HAL_UART_Transmit_IT(huart, &data, 1);
        } else {
            /* FIFO 为空，设置发送完成标志 */
            g_uart3.tx_busy = false;
        }
    }
}

/**
  * @brief  UART 接收完成中断回调函数
  * @param  huart: UART 句柄指针
  * @retval 无
  * @details 将接收到的字节存入 FIFO 缓冲区，
  *          然后重新启动接收中断等待下一个字节
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == g_uart1.huart) {
        /* 将接收到的字节存入 FIFO */
        uint8_t rx_byte = Rx1Byte;  // HAL 内部 buffer
        fifo_write(&g_uart1.rx_fifo, rx_byte);          // 写入 FIFO 接收缓冲区

        /* 启动下一次接收中断 */
        HAL_UART_Receive_IT(huart, &Rx1Byte, 1);
    }
    if (huart == g_uart3.huart) {
        /* 将接收到的字节存入 FIFO */
        uint8_t rx_byte = Rx3Byte;  // HAL 内部 buffer
        fifo_write(&g_uart3.rx_fifo, rx_byte);          // 写入 FIFO 接收缓冲区

        /* 启动下一次接收中断 */
        HAL_UART_Receive_IT(huart, &Rx3Byte, 1);
    }
}
