/**
  ******************************************************************************
  * @file           : i2c_oled.c
  * @brief          : I2C接口OLED显示屏驱动封装
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

#include "i2c_oled.h"
//#include <string.h>
//#include <math.h>
//#include <stdio.h>
//#include <stdarg.h>

/*
选择OLED显示驱动方式：默认使用模拟I2C总线
如果需要使用硬件I2C，请取消注释相应的宏定义并参考硬件I2C的配置注释
stm32cubemx初始化时要将SCL和SDA引脚的"user label"分别设置为I2C3_SCL和I2C3_SDA
*/
//#define OLED_USE_HW_I2C    // 使用硬件I2C
#define OLED_USE_SW_I2C    // 使用模拟I2C

/* I2C引脚定义 - 使用模拟I2C时的GPIO配置 */
#define I2C3_SDA_Pin      GPIO_PIN_9
#define I2C3_SDA_GPIO_Port GPIOB
#define I2C3_SCL_Pin      GPIO_PIN_8
#define I2C3_SCL_GPIO_Port GPIOB

/* 硬件I2C句柄 - 当使用硬件I2C时需要 */
#ifdef OLED_USE_HW_I2C
#define OLED_I2C          hi2c3
extern I2C_HandleTypeDef hi2c3;  // HAL库使用，指定硬件IIC接口
#endif

/* OLED I2C地址定义
   0x3C是OLED模块的7位从地址，左移1位是因为HAL库使用8位地址格式
   0x78 = (0x3C << 1) | 0，最低位0表示写操作
*/
#define OLED_ADDRESS      (0x3C << 1)

#define OLED_I2C_TIMEOUT  10      // I2C超时时间
#define Delay_time        3       // I2C时钟延迟时间

/**
  * @brief  数据存储格式说明
  * @details OLED显存按页（Page）组织，共8页，每页8行
  *          每一个Bit对应一个像素点
  *
  *      B0 B0                  B0 B0
  *      B1 B1                  B1 B1
  *      B2 B2                  B2 B2
  *      B3 B3  ------------->  B3 B3 --  新格式：Bit7在前（MSB First）
  *      B4 B4                  B4 B4  |
  *      B5 B5                  B5 B5  |
  *      B6 B6                  B6 B6  |
  *      B7 B7                  B7 B7  v
  *
  *  -----------------------------------
  *
  *      B0 B0                  B0 B0
  *      B1 B1                  B1 B1
  *      B2 B2                  B2 B2
  *      --> B3 B3  ------------->  B3 B3  旧格式：Bit0在前（LSB First）
  *      B4 B4                  B4 B4
  *      B5 B5                  B5 B5
  *      B6 B6                  B6 B6
  *      B7 B7                  B7 B7
  *
  *  本驱动采用新格式（MSB First）
  */

/**
  * @brief  OLED坐标系统
  * @details 屏幕分辨率为128x64
  *
  *  X坐标：0-127（列）
  *  Y坐标：0-63（行）
  *
  *  OLED显示内存按页组织，共8页（Page 0-7），每页8行
  *  Y坐标除以8得到页号，Y坐标模8得到页内行号
  *
  *       0             X（列）          127
  *      .------------------------------->
  *    0 |
  *      |
  *      |
  *      |
  *  Y（行）|
  *      |
  *      |
  *      |
  *   63 |
  *      v
  *
  *  显存映射：
  *  Page 0: Y = 0-7
  *  Page 1: Y = 8-15
  *  Page 2: Y = 16-23
  *  Page 3: Y = 24-31
  *  Page 4: Y = 32-39
  *  Page 5: Y = 40-47
  *  Page 6: Y = 48-55
  *  Page 7: Y = 56-63
  */

#ifdef OLED_USE_SW_I2C

/**
  * @brief  设置I2C时钟线SCL电平
  * @param  BitValue: 电平值（0或1）
  * @retval 无
  * @details 使用GPIO模拟I2C协议的时钟信号
  */
void OLED_W_SCL(uint8_t BitValue)
{
    HAL_GPIO_WritePin(I2C3_SCL_GPIO_Port, I2C3_SCL_Pin, (GPIO_PinState)BitValue);
    for (volatile uint16_t i = 0; i < Delay_time; i++) {
        // 添加延迟以满足I2C时序要求
    }
}

/**
  * @brief  设置I2C数据线SDA电平
  * @param  BitValue: 电平值（0或1）
  * @retval 无
  * @note   在SCL为高电平期间，SDA不能随意改变电平
  *         只有在SCL为低电平时才能改变SDA电平
  *         start条件：SCL高电平时，SDA下拉
  *         stop条件：SCL高电平时，SDA上拉
  */
void OLED_W_SDA(uint8_t BitValue)
{
    HAL_GPIO_WritePin(I2C3_SDA_GPIO_Port, I2C3_SDA_Pin, (GPIO_PinState)BitValue);
    for (volatile uint16_t i = 0; i < Delay_time; i++) {
        // 添加延迟以满足I2C时序要求
    }
}
#endif

/**
  * @brief  OLED GPIO初始化
  * @param  无
  * @retval 无
  * @details 初始化I2C引脚为开漏输出模式
  *          并添加上拉电阻（硬件已存在）
  *          延时等待系统稳定
  */
void OLED_GPIO_Init(void)
{
    uint32_t i, j;

    /* 延时等待系统稳定 */
    for (i = 0; i < 1000; i++) {
        for (j = 0; j < 1000; j++)
            ;
    }
#ifdef OLED_USE_SW_I2C
    OLED_W_SCL(1);  // 设置SCL为高电平
    OLED_W_SDA(1);  // 设置SDA为高电平
#endif
}

/**
  * @brief  I2C起始信号
  * @param  无
  * @retval 无
  * @details I2C起始条件：
  *          1. 在SCL为高电平时，将SDA从高电平拉低
  *          2. 然后将SCL拉低，进入数据传输阶段
  */
void OLED_I2C_Start(void)
{
#ifdef OLED_USE_SW_I2C
    OLED_W_SDA(1);  // 设置SDA为高电平，确保SDA为高电平
    OLED_W_SCL(1);  // 设置SCL为高电平，确保SCL为高电平
    OLED_W_SDA(0);  // 在SCL高电平时，将SDA拉低，产生起始信号
    OLED_W_SCL(0);  // 将SCL拉低，进入数据传输阶段，也为 subsequent bit传输做准备
#endif
}

/**
  * @brief  I2C停止信号
  * @param  无
  * @retval 无
  * @details I2C停止条件：
  *          1. 将SDA拉低
  *          2. 将SCL拉高
  *          3. 在SCL高电平时，将SDA拉高，产生停止信号
  */
void OLED_I2C_Stop(void)
{
#ifdef OLED_USE_SW_I2C
    OLED_W_SDA(0);  // 设置SDA为低电平，确保SDA为低电平
    OLED_W_SCL(1);  // 设置SCL为高电平，使SCL保持高电平
    OLED_W_SDA(1);  // 在SCL高电平时，将SDA拉高，产生停止信号
#endif
}

/**
  * @brief  I2C发送一个字节
  * @param  Byte: 要发送的字节数据，范围0x00~0xFF
  * @retval 无
  * @details 从最高位开始发送，每个位的过程：
  *          1. 设置SDA电平
  *          2. 拉高SCL，此时SDA上的数据被从设备读取
  *          3. 拉低SCL，开始传输下一位
  */
void OLED_I2C_SendByte(uint8_t Byte)
{
#ifdef OLED_USE_SW_I2C
    uint8_t i;
    for (i = 0; i < 8; i++) {
        OLED_W_SDA(!!(Byte & (0x80 >> i)));  // 发送最高位
        OLED_W_SCL(1);  // 拉高SCL，从设备读取SDA上的数据
        OLED_W_SCL(0);  // 拉低SCL，开始传输下一位
    }
    OLED_W_SCL(1);  // 额外的时钟周期，用于从设备发送ACK
    OLED_W_SCL(0);
#endif
}

/**
  * @brief  OLED写命令
  * @param  Command: 要写入的命令字节，范围0x00~0xFF
  * @retval 无
  * @details OLED I2C协议：
  *          1. 发送起始信号
  *          2. 发送从地址（0x78 = OLED地址 << 1，最低位0表示写）
  *          3. 发送控制字节（0x00表示后续为命令）
  *          4. 发送命令字节
  *          5. 发送停止信号
  */
void OLED_WriteCommand(uint8_t Command)
{
#ifdef OLED_USE_SW_I2C
    OLED_I2C_Start();           // I2C起始信号
    OLED_I2C_SendByte(0x78);    // 发送OLED I2C从地址（写模式）
    OLED_I2C_SendByte(0x00);    // 控制字节，0x00表示后续为命令数据
    OLED_I2C_SendByte(Command); // 发送命令字节
    OLED_I2C_Stop();            // I2C停止信号
#elif defined(OLED_USE_HW_I2C)
    uint8_t TxData[2] = {0x00, Command};
    HAL_I2C_Master_Transmit(&OLED_I2C, OLED_ADDRESS, (uint8_t*)TxData, 2, OLED_I2C_TIMEOUT);
#endif
}

/**
  * @brief  OLED写数据
  * @param  Data: 要写入的数据缓冲区起始地址
  * @param  Count: 要写入的数据数量
  * @retval 无
  * @details 数据写入过程与命令类似，但控制字节为0x40
  *          表示后续为显示数据
  */
void OLED_WriteData(uint8_t *Data, uint8_t Count)
{
    uint8_t i;
#ifdef OLED_USE_SW_I2C
    OLED_I2C_Start();           // I2C起始信号
    OLED_I2C_SendByte(0x78);    // 发送OLED I2C从地址（写模式）

    OLED_I2C_SendByte(0x40);    // 控制字节，0x40表示后续为显示数据
    for (i = 0; i < Count; i++) {
        OLED_I2C_SendByte(Data[i]); // 依次发送Data中的每一个字节
    }
    OLED_I2C_Stop();            // I2C停止信号
#elif defined(OLED_USE_HW_I2C)
    uint8_t TxData[Count + 1]; // 创建一个缓冲区，大小为Count + 1
    TxData[0] = 0x40;           // 第一个字节是控制字节
    // 将Data指针指向的数据逐字节复制到TxData数组中， IDX偏移为1
    for (i = 0; i < Count; i++) {
        TxData[i + 1] = Data[i];
    }
    HAL_I2C_Master_Transmit(&OLED_I2C, OLED_ADDRESS, (uint8_t*)TxData, Count + 1, OLED_I2C_TIMEOUT);
#endif
}
