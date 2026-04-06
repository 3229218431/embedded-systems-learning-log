/**
  ******************************************************************************
  * @file           : oled_api.h
  * @brief          : I2C OLED显示驱动API头文件
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

#ifndef _OLED_API_H_
#define _OLED_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* 字体尺寸定义 */
#define OLED_8X16     8   // 8x16字体
#define OLED_6X8      6   // 6x8字体

/* 绘图模式定义 */
#define OLED_UNFILLED 0   // 空心（描边）
#define OLED_FILLED   1   // 实心（填充）

/* OLED初始化和刷新函数 */
void OLED_Init(void);                     // OLED初始化
void OLED_Update(void);                   // 完整刷新OLED显示
void OLED_Update_InPages(void);           // 分页刷新OLED显示（节省资源）
void OLED_UpdateArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height);  // 刷新指定区域

/* OLED清屏函数 */
void OLED_Clear(void);                    // 清空整个屏幕
void OLED_ClearArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height);   // 清空指定区域
void OLED_Reverse(void);                  // 全屏反色显示
void OLED_ReverseArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height); // 指定区域反色

/* OLED字符显示函数 */
void OLED_ShowChar(uint8_t X, uint8_t Y, char Char, uint8_t FontSize);         // 显示单个字符
void OLED_ShowString(uint8_t X, uint8_t Y, char *String, uint8_t FontSize);   // 显示字符串
void OLED_ShowNum(uint8_t X, uint8_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);        // 显示无符号数字
void OLED_ShowSignedNum(uint8_t X, uint8_t Y, int32_t Number, uint8_t Length, uint8_t FontSize);   // 显示有符号数字
void OLED_ShowHexNum(uint8_t X, uint8_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);     // 显示十六进制数
void OLED_ShowBinNum(uint8_t X, uint8_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);     // 显示二进制数
void OLED_ShowFloatNum(uint8_t X, uint8_t Y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize);  // 显示浮点数
//void OLED_ShowChinese(uint8_t X, uint8_t Y, char *Chinese);
void OLED_ShowChinese(uint8_t x, uint8_t y, uint8_t num, uint8_t size1);  // 显示中文字符
void OLED_ShowImage(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image);  // 显示图片

/* OLED格式化打印函数 */
void OLED_Printf(uint8_t X, uint8_t Y, uint8_t FontSize, char *format, ...);  // 类似printf的格式化输出

/* OLED绘图函数 */
void OLED_DrawPoint(uint8_t X, uint8_t Y);       // 绘制像素点
uint8_t OLED_GetPoint(uint8_t X, uint8_t Y);     // 获取像素点状态
void OLED_DrawLine(uint8_t X0, uint8_t Y0, uint8_t X1, uint8_t Y1);  // 绘制直线
void OLED_DrawRectangle(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height, uint8_t IsFilled);  // 绘制矩形
void OLED_DrawTriangle(uint8_t X0, uint8_t Y0, uint8_t X1, uint8_t Y1, uint8_t X2, uint8_t Y2, uint8_t IsFilled);  // 绘制三角形
void OLED_DrawCircle(uint8_t X, uint8_t Y, uint8_t Radius, uint8_t IsFilled);  // 绘制圆形
void OLED_DrawEllipse(uint8_t X, uint8_t Y, uint8_t A, uint8_t B, uint8_t IsFilled);  // 绘制椭圆
void OLED_DrawArc(uint8_t X, uint8_t Y, uint8_t Radius, int16_t StartAngle, int16_t EndAngle, uint8_t IsFilled);  // 绘制圆弧

#ifdef __cplusplus
}
#endif

#endif /* _OLED_API_H_ */
