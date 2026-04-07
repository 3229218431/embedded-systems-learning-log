/**
  ******************************************************************************
  * @file           : FIFO.h
  * @brief          : 环形缓冲区FIFO通用接口头文件
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
#ifndef _FIFO_H_
#define _FIFO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>    // 标准整数类型定义
#include <stddef.h>    // size_t类型定义
#include <stdbool.h>   // 布尔类型定义

/**
  * @brief  通用FIFO结构体
  * @details 实现环形缓冲区的数据存储和管理
  *          采用循环数组实现，避免数据移动
  */
typedef struct {
    uint8_t *buffer;        // FIFO缓冲区指针，指向实际存储数据的内存区域
    size_t size;            // 缓冲区大小，表示缓冲区最大可存储的数据字节数
    size_t head;            // 读指针，指向将要读取的数据位置
    size_t tail;            // 写指针，指向将要写入的数据位置
    size_t count;           // 当前数据数量，表示当前缓冲区中有效数据的字节数
} fifo_t;

/* FIFO基本操作函数声明 */
int fifo_init(fifo_t *fifo, uint8_t *buffer, size_t size);
int fifo_write(fifo_t *fifo, uint8_t data);
int fifo_read(fifo_t *fifo, uint8_t *data);
size_t fifo_write_multi(fifo_t *fifo, const uint8_t *data, size_t len);
size_t fifo_read_multi(fifo_t *fifo, uint8_t *data, size_t len);
size_t fifo_get_count(fifo_t *fifo);
size_t fifo_get_free_size(fifo_t *fifo);
bool fifo_is_empty(fifo_t *fifo);
bool fifo_is_full(fifo_t *fifo);
void fifo_clear(fifo_t *fifo);
void fifo_debug_info(fifo_t *fifo);

#ifdef __cplusplus
}
#endif

#endif /* _FIFO_H_ */
