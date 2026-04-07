/**
  ******************************************************************************
  * @file           : Board.c
  * @brief          : 板级支持包HAL初始化文件
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
#include "Board.h"
//#include "bmp.h"

/**
  * @brief  板级初始化函数
  * @param  无
  * @retval 无
  * @details 初始化所有外设，包括：
  *          - UART1和UART3串口通信
  *          - OLED显示模块
  *          - 电机驱动模块
  */
void BoardInit(void)
{
    uint32_t i;

    /* 初始化串口通信 */
    InitMwUart1();      // 初始化串口1
    InitMwUart3();      // 初始化串口3

    /* 初始化OLED显示屏 */
    OLED_Init();

    // OLED显示初始界面
    //OLED_ShowPicture(0,0, 128, 64, BMP1, 1);
    //OLED_Reverse();

    /* 显示欢迎文字（第0-6个中文字符） */
    for(i=0; i<7; i++) {
        OLED_ShowChinese((8+(i<<4)),0,i,16);
    }

    // OLED_ShowString(8,16, "E-Track Traning", OLED_8X16);  // 移除以避免显示重叠
    // OLED_ShowString(72, 48, "mV", OLED_8X16);            // 移除以避免显示重叠

    /* 初始化电机驱动 */
    InitMotor();

    /* 更新OLED显示 */
    OLED_Update();
}
