/**
  ******************************************************************************
  * @file           : FIFO.c
  * @brief          : 环形缓冲区FIFO实现文件
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
#include "FIFO.h"

/**
  * @brief  初始化FIFO缓冲区
  * @param  fifo: FIFO结构体指针
  * @param  buffer: 数据缓冲区指针
  * @param  size: 缓冲区大小（字节数）
  * @retval 0 表示成功，-1 表示失败
  * @details FIFO（First In First Out）是一种先进先出的数据结构，
  *          使用循环数组实现。初始化时设置：
  *          - buffer：数据存储缓冲区
  *          - size：缓冲区容量
  *          - head：读指针，初始为0
  *          - tail：写指针，初始为0
  *          - count：当前数据量，初始为0
  */
int fifo_init(fifo_t *fifo, uint8_t *buffer, size_t size) {
    if (fifo == NULL || buffer == NULL || size == 0) {  // 检查参数有效性
        return -1;  // 参数无效，返回错误
    }

    fifo->buffer = buffer;  // 设置用户提供的缓冲区指针
    fifo->size = size;      // 设置缓冲区大小
    fifo->head = 0;         // 读指针初始为0
    fifo->tail = 0;         // 写指针初始为0
    fifo->count = 0;        // 初始数据量为0

    return 0;  // 初始化成功，返回成功
}

/**
  * @brief  向FIFO写入单个数据
  * @param  fifo: FIFO结构体指针
  * @param  data: 要写入的数据
  * @retval 0 表示成功，-1 表示失败，FIFO 已满
  * @details 写入操作：
  *          1. 检查FIFO是否已满
  *          2. 在tail位置写入数据
  *          3. 移动tail指针，使用模运算实现循环
  *          4. 增加数据计数
  */
int fifo_write(fifo_t *fifo, uint8_t data) {
    if (fifo == NULL || fifo->count >= fifo->size) {  // 检查FIFO是否为空或已满
        return -1;  // FIFO已满或参数无效，返回错误
    }

    fifo->buffer[fifo->tail] = data;        // 在写指针位置写入数据
    fifo->tail = (fifo->tail + 1) % fifo->size;  // 移动tail指针，使用模运算实现循环
    fifo->count++;                          // 增加数据计数

    return 0;  // 写入成功，返回成功
}

/**
  * @brief  从FIFO读取单个数据
  * @param  fifo: FIFO结构体指针
  * @param  data: 读取的数据存储地址
  * @retval 0 表示成功，-1 表示失败，FIFO 为空
  * @details 读取操作：
  *          1. 检查FIFO是否为空
  *          2. 从head位置读取数据到目标地址
  *          3. 移动head指针，使用模运算实现循环
  *          4. 减少数据计数
  */
int fifo_read(fifo_t *fifo, uint8_t *data) {
    if (fifo == NULL || data == NULL || fifo->count == 0) {  // 检查参数有效性及FIFO是否为空
        return -1;  // FIFO为空或参数无效，返回错误
    }

    *data = fifo->buffer[fifo->head];       // 从head位置读取数据到data指向的位置
    fifo->head = (fifo->head + 1) % fifo->size;  // 移动head指针，使用模运算实现循环
    fifo->count--;                          // 减少数据计数

    return 0;  // 读取成功，返回成功
}

/**
  * @brief  向FIFO批量写入数据
  * @param  fifo: FIFO结构体指针
  * @param  data: 要写入的数据指针
  * @param  len: 要写入的数据长度
  * @retval 实际写入的数据长度
  * @details 批量写入操作：
  *          1. 计算FIFO可用空间
  *          2. 确定实际可写入长度
  *          3. 循环写入数据
  */
size_t fifo_write_multi(fifo_t *fifo, const uint8_t *data, size_t len) {
    if (fifo == NULL || data == NULL) {  // 检查参数有效性
        return 0;  // 参数无效，返回0
    }

    size_t available = fifo->size - fifo->count;  // 计算FIFO可用空间大小
    size_t write_len = (len < available) ? len : available;  // 确定实际可写入数据长度

    for (size_t i = 0; i < write_len; i++) {  // 循环写入数据
        fifo->buffer[fifo->tail] = data[i];  // 在写指针位置写入
        fifo->tail = (fifo->tail + 1) % fifo->size;  // 移动tail指针
    }

    fifo->count += write_len;  // 增加数据计数
    return write_len;  // 返回实际写入数据长度
}

/**
  * @brief  从FIFO批量读取数据
  * @param  fifo: FIFO结构体指针
  * @param  data: 读取的数据存储缓冲区指针
  * @param  len: 要读取的数据长度
  * @retval 实际读取的数据长度
  * @details 批量读取操作：
  *          1. 获取FIFO当前数据量
  *          2. 确定实际可读取长度
  *          3. 循环读取数据到目标缓冲区
  */
size_t fifo_read_multi(fifo_t *fifo, uint8_t *data, size_t len) {
    if (fifo == NULL || data == NULL) {  // 检查参数有效性
        return 0;  // 参数无效，返回0
    }

    size_t available = fifo->count;  // 获取FIFO中的当前数据量
    size_t read_len = (len < available) ? len : available;  // 确定实际可读取数据长度

    for (size_t i = 0; i < read_len; i++) {  // 循环读取数据
        data[i] = fifo->buffer[fifo->head];  // 从读指针位置读取数据到目标位置
        fifo->head = (fifo->head + 1) % fifo->size;  // 移动head指针
    }

    fifo->count -= read_len;  // 减少数据计数
    return read_len;  // 返回实际读取数据长度
}

/**
  * @brief  获取FIFO当前元素数量
  * @param  fifo: FIFO结构体指针
  * @retval 当前元素数量
  */
size_t fifo_get_count(fifo_t *fifo) {
    if (fifo == NULL) {  // 检查参数有效性
        return 0;  // 参数无效，返回0
    }
    return fifo->count;  // 返回当前元素数量
}

/**
  * @brief  获取FIFO剩余空间大小
  * @param  fifo: FIFO结构体指针
  * @retval 剩余空间大小
  */
size_t fifo_get_free_size(fifo_t *fifo) {
    if (fifo == NULL) {  // 检查参数有效性
        return 0;  // 参数无效，返回0
    }
    return fifo->size - fifo->count;  // 返回剩余空间大小
}

/**
  * @brief  判断FIFO是否为空
  * @param  fifo: FIFO结构体指针
  * @retval true 表示为空，false 表示不为空
  */
bool fifo_is_empty(fifo_t *fifo) {
    if (fifo == NULL) {  // 检查参数有效性
        return true;  // 参数无效时认为为空
    }
    return fifo->count == 0;  // 检查数据计数是否为0
}

/**
  * @brief  判断FIFO是否已满
  * @param  fifo: FIFO结构体指针
  * @retval true 表示已满，false 表示未满
  */
bool fifo_is_full(fifo_t *fifo) {
    if (fifo == NULL) {  // 检查参数有效性
        return true;  // 参数无效时认为已满
    }
    return fifo->count >= fifo->size;  // 检查数据计数是否已达到容量上限
}

/**
  * @brief  清空FIFO
  * @param  fifo: FIFO结构体指针
  * @retval 无
  * @details 将读写指针重置为0，并将数据计数清零
  */
void fifo_clear(fifo_t *fifo) {
    if (fifo != NULL) {  // 检查参数有效性
        fifo->head = 0;      // 读指针重置为0
        fifo->tail = 0;      // 写指针重置为0
        fifo->count = 0;     // 数据计数清零
    }
}

/**
  * @brief  FIFO调试信息打印函数（暂未启用）
  * @param  fifo: FIFO结构体指针
  * @retval 无
  * @details 在真实嵌入式系统中，通常不需要调试信息打印
  *          可以通过注释掉的 printf 来输出FIFO状态信息
  */
void fifo_debug_info(fifo_t *fifo) {
    if (fifo != NULL) {  // 检查参数有效性
        // 在真实嵌入式系统中，通常不需要调试信息打印
        // 可以通过串口打印 FIFO 状态信息
        // printf("FIFO: Size=%d, Count=%d, Head=%d, Tail=%d\n",
        //        fifo->size, fifo->count, fifo->head, fifo->tail);
    }
}
