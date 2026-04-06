/**
  ******************************************************************************
  * @file           : oled_api.c
  * @brief          : OLED֡��֮�ϵĸ�����д/���ƺ�����װ
  ******************************************************************************
  * @attention
  *
  * Copyright (c) �Ϻ�ʦ����ѧ 2025-2035
  * All rights reserved.
  *
  * ������ֻ��ѧϰʹ�ã�δ���������ɣ��������������κ���;
  * �Ϻ�ʦ����ѧ ��Ϣ����繤��ѧԺ ͨ�Ź���רҵ
  * ��Դ��ַ��https://gitee.com/NEagle
  * �޸����ڣ�2025/12/10
  * �汾�� V1.0
  * ��Ȩ���У�����ؾ�
  * V1.0�޸�˵��
  *
  ******************************************************************************
  */

#include "i2c_oled.h"
#include "oled_api.h"
#include <string.h>
#include "oledfont.h"
#include <math.h>
#include <stdio.h>
#include <stdarg.h>

/**
 * OLED�Դ�����
 * ���е���ʾ��������ֻ�ǶԴ��Դ�������ж�д
 * ������OLED_Update������OLED_UpdateArea����
 * �ŻὫ�Դ���������ݷ��͵�OLEDӲ����������ʾ
 */
uint8_t OLED_DisplayBuf[8][128];
// ��Ҫˢ�µ�Pageָʾ
volatile uint8_t PageNeedFlash = 0;




/**
 * @brief ��OLED�Դ�����ȫ������
 * @param ��
 * @return ��
 * @note ���ô˺�����Ҫ�������س�������Ļ�ϣ�������ø��º���
 */
void OLED_Clear(void)
{
    uint8_t i, j;
    for (j = 0; j < 8; j++) // ����8ҳ
    {
        for (i = 0; i < 128; i++) // ����128��
        {
            OLED_DisplayBuf[j][i] = 0x00; // ���Դ���������ȫ������
        }
    }
	PageNeedFlash = 0xFF;
}

/**
 * @brief OLED������ʾ���λ��
 * @param Page ָ��������ڵ�ҳ����Χ��0-7
 * @param X ָ��������ڵ�X�����꣬��Χ��0-127
 * @return ��
 * @note OLEDĬ�ϵ�Y�ᣬֻ��8��BitΪһ��д�룬��1ҳ����8��Y������
 */
void OLED_SetCursor(uint8_t Page, uint8_t X)
{
    //	X += 2;

    OLED_WriteCommand(0xB0 | Page);              // ����ҳλ��
    OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4)); // ����Xλ�ø�4λ
    OLED_WriteCommand(0x00 | (X & 0x0F));        // ����Xλ�õ�4λ
}

/**
 * @brief ��OLED�Դ�������µ�OLED��Ļ
 * @param ��
 * @return ��
 * @note ���е���ʾ��������ֻ�Ƕ�OLED�Դ�������ж�д
 *           ������OLED_Update������OLED_UpdateArea����
 *           �ŻὫ�Դ���������ݷ��͵�OLEDӲ����������ʾ
 *           �ʵ�����ʾ������Ҫ�������س�������Ļ�ϣ�������ø��º���
 */
void OLED_Update(void)
{
    uint8_t j;
    for (j = 0; j < 8; j++) {
        OLED_SetCursor(j, 0);
        OLED_WriteData(OLED_DisplayBuf[j], 128);
    }
	PageNeedFlash = 0x00;
}

/**
 * @brief ��OLED�Դ����鰴��һ�θ���һ��page�ķ�ʽ���µ�OLED��Ļ���˺������������������
 * @param ��
 * @return ��
 * @note ���е���ʾ��������ֻ�Ƕ�OLED�Դ�������ж�д
 *           ������OLED_Update������OLED_UpdateArea����
 *           �ŻὫ�Դ���������ݷ��͵�OLEDӲ����������ʾ
 *           �ʵ�����ʾ������Ҫ�������س�������Ļ�ϣ�������ø��º���
 */
void OLED_Update_InPages(void)
{
	static uint8_t index = 0;
	if(PageNeedFlash & (0x01 << index))	{	// ���Page��Ҫˢ��
		OLED_SetCursor(index, 0);
		OLED_WriteData(OLED_DisplayBuf[index], 128);
		PageNeedFlash &= ~(0x01 << index);
	}
	index ++;
}


/**
 * @brief OLED��ʼ��
 * @param ��
 * @return ��
 * @note ʹ��ǰ����Ҫ���ô˳�ʼ������
 */
void OLED_Init(void)
{
    OLED_GPIO_Init(); // �ȵ��õײ�Ķ˿ڳ�ʼ��

    OLED_WriteCommand(0xAE); // ������ʾ����/�رգ�0xAE�رգ�0xAF����

    OLED_WriteCommand(0xD5); // ������ʾʱ�ӷ�Ƶ��/����Ƶ��
    OLED_WriteCommand(0x80); // 0x00~0xFF

    OLED_WriteCommand(0xA8); // ���ö�·������
    OLED_WriteCommand(0x3F); // 0x0E~0x3F

    OLED_WriteCommand(0xD3); // ������ʾƫ��
    OLED_WriteCommand(0x00); // 0x00~0x7F

    OLED_WriteCommand(0x40); // ������ʾ��ʼ�У�0x40~0x7F

    OLED_WriteCommand(0xA1); // �������ҷ���0xA1������0xA0���ҷ���

    OLED_WriteCommand(0xC8); // �������·���0xC8������0xC0���·���

    OLED_WriteCommand(0xDA); // ����COM����Ӳ������
    OLED_WriteCommand(0x12);

    OLED_WriteCommand(0x81); // ���öԱȶ�
    OLED_WriteCommand(0xCF); // 0x00~0xFF

    OLED_WriteCommand(0xD9); // ����Ԥ�������
    OLED_WriteCommand(0xF1);

    OLED_WriteCommand(0xDB); // ����VCOMHȡ��ѡ�񼶱�
    OLED_WriteCommand(0x30);

    OLED_WriteCommand(0xA4); // ����������ʾ��/�ر�

    OLED_WriteCommand(0xA6); // ��������/��ɫ��ʾ��0xA6������0xA7��ɫ

    OLED_WriteCommand(0x8D); // ���ó���
    OLED_WriteCommand(0x14);

    OLED_WriteCommand(0xAF); // ������ʾ

    OLED_Clear();  // ����Դ�����
    OLED_Update(); // ������ʾ����������ֹ��ʼ����δ��ʾ����ʱ����
}

/**
 * @brief �η�����
 * @param X ����
 * @param Y ָ��
 * @return ����X��Y�η�
 */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1; // ���Ĭ��Ϊ1
    while (Y--)          // �۳�Y��
    {
        Result *= X; // ÿ�ΰ�X�۳˵������
    }
    return Result;
}

/**
 * @brief �ж�ָ�����Ƿ���ָ��������ڲ�
 * @param nvert ����εĶ�����
 * @param vertx verty ��������ζ����x��y���������
 * @param testx testy ���Ե��X��y����
 * @return ָ�����Ƿ���ָ��������ڲ���1�����ڲ���0�������ڲ�
 */
uint8_t OLED_pnpoly(uint8_t nvert, int16_t *vertx, int16_t *verty, int16_t testx, int16_t testy)
{
    int16_t i, j, c = 0;

    for (i = 0, j = nvert - 1; i < nvert; j = i++) {
        if (((verty[i] > testy) != (verty[j] > testy)) &&
            (testx < (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) + vertx[i])) {
            c = !c;
        }
    }
    return c;
}

/**
 * @brief �ж�ָ�����Ƿ���ָ���Ƕ��ڲ�
 * @param X Y ָ���������
 * @param StartAngle EndAngle ��ʼ�ǶȺ���ֹ�Ƕȣ���Χ��-180-180
 *           ˮƽ����Ϊ0�ȣ�ˮƽ����Ϊ180�Ȼ�-180�ȣ��·�Ϊ�������Ϸ�Ϊ������˳ʱ����ת
 * @return ָ�����Ƿ���ָ���Ƕ��ڲ���1�����ڲ���0�������ڲ�
 */
uint8_t OLED_IsInAngle(int16_t X, int16_t Y, int16_t StartAngle, int16_t EndAngle)
{
    int16_t PointAngle;
    PointAngle = atan2(Y, X) / 3.14 * 180; // ����ָ����Ļ��ȣ���ת��Ϊ�Ƕȱ�ʾ
    if (StartAngle < EndAngle)             // ��ʼ�Ƕ�С����ֹ�Ƕȵ����
    {
        if (PointAngle >= StartAngle && PointAngle <= EndAngle) {
            return 1;
        }
    } else // ��ʼ�Ƕȴ�������ֹ�Ƕȵ����
    {
        if (PointAngle >= StartAngle || PointAngle <= EndAngle) {
            return 1;
        }
    }
    return 0; // �������������������ж��ж�ָ���㲻��ָ���Ƕ�
}



/**
 * @brief ��OLED�Դ����鲿�ָ��µ�OLED��Ļ
 * @param X ָ���������Ͻǵĺ����꣬��Χ��0-127
 * @param Y ָ���������Ͻǵ������꣬��Χ��0-63
 * @param Width ָ������Ŀ��ȣ���Χ��0-128
 * @param Height ָ������ĸ߶ȣ���Χ��0-64
 * @return ��
 * @note �˺��������ٸ��²���ָ��������
 *           �����������Y��ֻ��������ҳ����ͬһҳ��ʣ�ಿ�ֻ����һ�����
 * @note ���е���ʾ��������ֻ�Ƕ�OLED�Դ�������ж�д
 *           ������OLED_Update������OLED_UpdateArea����
 *           �ŻὫ�Դ���������ݷ��͵�OLEDӲ����������ʾ
 *           �ʵ�����ʾ������Ҫ�������س�������Ļ�ϣ�������ø��º���
 */
void OLED_UpdateArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height)
{
    uint8_t j;

    if (X > 127) { return; }
    if (Y > 63) { return; }
    if (X + Width > 128) { Width = 128 - X; }
    if (Y + Height > 64) { Height = 64 - Y; }

    for (j = Y / 8; j < (Y + Height - 1) / 8 + 1; j++) {
        OLED_SetCursor(j, X);
        OLED_WriteData(&OLED_DisplayBuf[j][X], Width);
		PageNeedFlash &= ~(0x01 << j);
    }
}


/**
 * @brief ��OLED�Դ����鲿������
 * @param X ָ���������Ͻǵĺ����꣬��Χ��0-127
 * @param Y ָ���������Ͻǵ������꣬��Χ��0-63
 * @param Width ָ������Ŀ��ȣ���Χ��0-128
 * @param Height ָ������ĸ߶ȣ���Χ��0-64
 * @return ��
 * @note ���ô˺�����Ҫ�������س�������Ļ�ϣ�������ø��º���
 */
void OLED_ClearArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height)
{
    uint8_t i, j;

    if (X > 127) { return; }
    if (Y > 63) { return; }
    if (X + Width > 128) { Width = 128 - X; }
    if (Y + Height > 64) { Height = 64 - Y; }

    for (j = Y; j < Y + Height; j++) // ����ָ��ҳ
    {
		PageNeedFlash |= (0x01 << (j>>3));
        for (i = X; i < X + Width; i++) // ����ָ����
        {
            OLED_DisplayBuf[j / 8][i] &= ~(0x01 << (j % 8)); // ���Դ�����ָ����������
        }
    }
}

/**
 * @brief ��OLED�Դ�����ȫ��ȡ��
 * @param ��
 * @return ��
 * @note ���ô˺�����Ҫ�������س�������Ļ�ϣ�������ø��º���
 */
void OLED_Reverse(void)
{
    uint8_t i, j;
    for (j = 0; j < 8; j++) // ����8ҳ
    {
        for (i = 0; i < 128; i++) // ����128��
        {
            OLED_DisplayBuf[j][i] ^= 0xFF; // ���Դ���������ȫ��ȡ��
        }
    }
	PageNeedFlash = 0xFF;
}

/**
 * @brief ��OLED�Դ����鲿��ȡ��
 * @param X ָ���������Ͻǵĺ����꣬��Χ��0-127
 * @param Y ָ���������Ͻǵ������꣬��Χ��0-63
 * @param Width ָ������Ŀ��ȣ���Χ��0-128
 * @param Height ָ������ĸ߶ȣ���Χ��0-64
 * @return ��
 * @note ���ô˺�����Ҫ�������س�������Ļ�ϣ�������ø��º���
 */
void OLED_ReverseArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height)
{
    uint8_t i, j;

    if (X > 127) { return; }
    if (Y > 63) { return; }
    if (X + Width > 128) { Width = 128 - X; }
    if (Y + Height > 64) { Height = 64 - Y; }

    for (j = Y; j < Y + Height; j++) // ����ָ��ҳ
    {
        for (i = X; i < X + Width; i++) // ����ָ����
        {
            OLED_DisplayBuf[j / 8][i] ^= 0x01 << (j % 8); // ���Դ�����ָ������ȡ��
        }
		PageNeedFlash |= (0x01 << (j>>3));
    }
}

/**
 * @brief OLED��ʾһ���ַ�
 * @param X ָ���ַ����Ͻǵĺ����꣬��Χ��0-127
 * @param Y ָ���ַ����Ͻǵ������꣬��Χ��0-63
 * @param Char ָ��Ҫ��ʾ���ַ�����Χ��ASCII��ɼ��ַ�
 * @param FontSize ָ�������С
 *           ��Χ��OLED_8X16		��8���أ���16����
 *                 OLED_6X8		��6���أ���8����
 * @return ��
 * @note ���ô˺�����Ҫ�������س�������Ļ�ϣ�������ø��º���
 */
void OLED_ShowChar(uint8_t X, uint8_t Y, char Char, uint8_t FontSize)
{
    if (FontSize == OLED_8X16) // ����Ϊ��8���أ���16����
    {
        OLED_ShowImage(X, Y, 8, 16, OLED_F8x16[Char - ' ']);
    } else if (FontSize == OLED_6X8) // ����Ϊ��6���أ���8����
    {
        OLED_ShowImage(X, Y, 6, 8, OLED_F6x8[Char - ' ']);
    }
}

/**
 * @brief OLED��ʾ�ַ���
 * @param X ָ���ַ������Ͻǵĺ����꣬��Χ��0-127
 * @param Y ָ���ַ������Ͻǵ������꣬��Χ��0-63
 * @param String ָ��Ҫ��ʾ���ַ�������Χ��ASCII��ɼ��ַ���ɵ��ַ���
 * @param FontSize ָ�������С
 *           ��Χ��OLED_8X16		��8���أ���16����
 *                 OLED_6X8		��6���أ���8����
 * @return ��
 * @note ���ô˺�����Ҫ�������س�������Ļ�ϣ�������ø��º���
 */
void OLED_ShowString(uint8_t X, uint8_t Y, char *String, uint8_t FontSize)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i++) // �����ַ�����ÿ���ַ�
    {
        OLED_ShowChar(X + i * FontSize, Y, String[i], FontSize);
    }
}

/**
 * @brief OLED��ʾ���֣�ʮ���ƣ���������
 * @param X ָ���������Ͻǵĺ����꣬��Χ��0-127
 * @param Y ָ���������Ͻǵ������꣬��Χ��0-63
 * @param Number ָ��Ҫ��ʾ�����֣���Χ��0-4294967295
 * @param Length ָ�����ֵĳ��ȣ���Χ��0-10
 * @param FontSize ָ�������С
 *           ��Χ��OLED_8X16		��8���أ���16����
 *                 OLED_6X8		��6���أ���8����
 * @return ��
 * @note ���ô˺�����Ҫ�������س�������Ļ�ϣ�������ø��º���
 */
void OLED_ShowNum(uint8_t X, uint8_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i;
    for (i = 0; i < Length; i++) // �������ֵ�ÿһλ
    {
        OLED_ShowChar(X + i * FontSize, Y, Number / OLED_Pow(10, Length - i - 1) % 10 + '0', FontSize);
    }
}

/**
 * @brief OLED��ʾ�з������֣�ʮ���ƣ�������
 * @param X ָ���������Ͻǵĺ����꣬��Χ��0-127
 * @param Y ָ���������Ͻǵ������꣬��Χ��0-63
 * @param Number ָ��Ҫ��ʾ�����֣���Χ��-2147483648-2147483647
 * @param Length ָ�����ֵĳ��ȣ���Χ��0-10
 * @param FontSize ָ�������С
 *           ��Χ��OLED_8X16		��8���أ���16����
 *                 OLED_6X8		��6���أ���8����
 * @return ��
 * @note ���ô˺�����Ҫ�������س�������Ļ�ϣ�������ø��º���
 */
void OLED_ShowSignedNum(uint8_t X, uint8_t Y, int32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i;
    uint32_t Number1;

    if (Number >= 0) // ���ִ��ڵ���0
    {
        OLED_ShowChar(X, Y, '+', FontSize); // ��ʾ+��
        Number1 = Number;                   // Number1ֱ�ӵ���Number
    } else                                  // ����С��0
    {
        OLED_ShowChar(X, Y, '-', FontSize); // ��ʾ-��
        Number1 = -Number;                  // Number1����Numberȡ��
    }

    for (i = 0; i < Length; i++) // �������ֵ�ÿһλ
    {
        OLED_ShowChar(X + (i + 1) * FontSize, Y, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0', FontSize);
    }
}

/**
 * @brief OLED��ʾʮ���������֣�ʮ�����ƣ���������
 * @param X ָ���������Ͻǵĺ����꣬��Χ��0~127
 * @param Y ָ���������Ͻǵ������꣬��Χ��0~63
 * @param Number ָ��Ҫ��ʾ�����֣���Χ��0x00000000~0xFFFFFFFF
 * @param Length ָ�����ֵĳ��ȣ���Χ��0~8
 * @param FontSize ָ�������С
 *           ��Χ��OLED_8X16		��8���أ���16����
 *                 OLED_6X8		��6���أ���8����
 * @return ��
 * @note ���ô˺�����Ҫ�������س�������Ļ�ϣ�������ø��º���
 */
void OLED_ShowHexNum(uint8_t X, uint8_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i, SingleNumber;
    for (i = 0; i < Length; i++) // �������ֵ�ÿһλ
    {
        SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;

        if (SingleNumber < 10) // ��������С��10
        {
            OLED_ShowChar(X + i * FontSize, Y, SingleNumber + '0', FontSize);
        } else // �������ִ���10
        {
            OLED_ShowChar(X + i * FontSize, Y, SingleNumber - 10 + 'A', FontSize);
        }
    }
}

/**
 * @brief OLED��ʾ���������֣������ƣ���������
 * @param X ָ���������Ͻǵĺ����꣬��Χ��0~127
 * @param Y ָ���������Ͻǵ������꣬��Χ��0~63
 * @param Number ָ��Ҫ��ʾ�����֣���Χ��0x00000000~0xFFFFFFFF
 * @param Length ָ�����ֵĳ��ȣ���Χ��0~16
 * @param FontSize ָ�������С
 *           ��Χ��OLED_8X16		��8���أ���16����
 *                 OLED_6X8		��6���أ���8����
 * @return ��
 * @note ���ô˺�����Ҫ�������س�������Ļ�ϣ�������ø��º���
 */
void OLED_ShowBinNum(uint8_t X, uint8_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i;
    for (i = 0; i < Length; i++) // �������ֵ�ÿһλ
    {
        OLED_ShowChar(X + i * FontSize, Y, Number / OLED_Pow(2, Length - i - 1) % 2 + '0', FontSize);
    }
}

/**
 * @brief OLED��ʾ�������֣�ʮ���ƣ�С����
 * @param X ָ���������Ͻǵĺ����꣬��Χ��0-127
 * @param Y ָ���������Ͻǵ������꣬��Χ��0-63
 * @param Number ָ��Ҫ��ʾ�����֣���Χ��-4294967295.0-4294967295.0
 * @param IntLength ָ�����ֵ�����λ���ȣ���Χ��0-10
 * @param FraLength ָ�����ֵ�С��λ���ȣ���Χ��0-9��С����������������ʾ
 * @param FontSize ָ�������С
 *           ��Χ��OLED_8X16		��8���أ���16����
 *                  OLED_6X8	    ��6���أ���8����
 * @return ��
 * @note ���ô˺�����Ҫ�������س�������Ļ�ϣ�������ø��º���
 */
void OLED_ShowFloatNum(uint8_t X, uint8_t Y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize)
{
    uint32_t PowNum, IntNum, FraNum;

    if (Number >= 0) // ���ִ��ڵ���0
    {
        OLED_ShowChar(X, Y, '+', FontSize); // ��ʾ+��
    } else                                  // ����С��0
    {
        OLED_ShowChar(X, Y, '-', FontSize); // ��ʾ-��
        Number = -Number;                   // Numberȡ��
    }

    IntNum = Number;                  // ֱ�Ӹ�ֵ�����ͱ�������ȡ����
    Number -= IntNum;                 // ��Number��������������ֹ֮��С���˵�����ʱ����������ɴ���
    PowNum = OLED_Pow(10, FraLength); // ����ָ��С����λ����ȷ������
    FraNum = round(Number * PowNum);  // ��С���˵�������ͬʱ�������룬������ʾ���
    IntNum += FraNum / PowNum;        // ��������������˽�λ������Ҫ�ټӸ�����

    OLED_ShowNum(X + FontSize, Y, IntNum, IntLength, FontSize);

    OLED_ShowChar(X + (IntLength + 1) * FontSize, Y, '.', FontSize);

    OLED_ShowNum(X + (IntLength + 2) * FontSize, Y, FraNum, FraLength, FontSize);
}

/**
 * ��ʾ����
 * @param x ��ʼx��ֵ
 * @param y ��ʼy��ֵ
 * @param num ��ʾ�ĺ������
 * @param size1 ѡ������ 6x8/6x12/8x16/12x24
 * @return NONE
 */
void OLED_ShowChinese(uint8_t x,uint8_t y,uint8_t num,uint8_t size1)
{
    volatile uint8_t m,temp;
    uint8_t x0=x,y0=y;
	// ��Ӧ��������
	OLED_ClearArea(x,y,size1, size1);
    uint16_t i,size3=(size1/8+((size1%8)?1:0))*size1;  //�õ�����һ���ַ���Ӧ������ռ���ֽ���
    for(i=0;i<size3;i++)
    {
        if(size1==16)
                {temp=Hzk1[num][i];}//����16*16����
        else if(size1==24)
                {temp=Hzk2[num][i];}//����24*24����
        else if(size1==32)
                {temp=Hzk3[num][i];}//����32*32����
        else if(size1==64)
                {temp=Hzk4[num][i];}//����64*64����
        else return;
        for(m=0;m<8;m++)
        {
            if(temp&0x01)OLED_DrawPoint(x,y);
            //else OLED_DrawPoint(x,y);
            temp>>=1;
            y++;
        }
        x++;
        if((x-x0)==size1)
        {x=x0;y0=y0+8;}
        y=y0;
    }
}

/**
 * @brief OLED��ʾͼ��
 * @param X ָ��ͼ�����Ͻǵĺ����꣬��Χ��0-127
 * @param Y ָ��ͼ�����Ͻǵ������꣬��Χ��0-63
 * @param Width ָ��ͼ��Ŀ��ȣ���Χ��0-128
 * @param Height ָ��ͼ��ĸ߶ȣ���Χ��0-64
 * @param Image ָ��Ҫ��ʾ��ͼ��
 * @return ��
 * @note ���ô˺�����Ҫ�������س�������Ļ�ϣ�������ø��º���
 */
void OLED_ShowImage(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image)
{
    uint8_t i, j;

    if (X > 127) { return; }
    if (Y > 63) { return; }

    OLED_ClearArea(X, Y, Width, Height);

    for (j = 0; j < (Height - 1) / 8 + 1; j++) {
        for (i = 0; i < Width; i++) {
            if (X + i > 127) { break; }
            if (Y / 8 + j > 7) { return; }

            OLED_DisplayBuf[Y / 8 + j][X + i] |= Image[j * Width + i] << (Y % 8);

            if (Y / 8 + j + 1 > 7) { continue; }

            OLED_DisplayBuf[Y / 8 + j + 1][X + i] |= Image[j * Width + i] >> (8 - Y % 8);
        }
    }
}

/**
 * @brief OLEDʹ��printf������ӡ��ʽ���ַ���
 * @param X ָ����ʽ���ַ������Ͻǵĺ����꣬��Χ��0-127
 * @param Y ָ����ʽ���ַ������Ͻǵ������꣬��Χ��0-63
 * @param FontSize ָ�������С
 *           ��Χ��OLED_8X16		��8���أ���16����
 *                 OLED_6X8		��6���أ���8����
 * @param format ָ��Ҫ��ʾ�ĸ�ʽ���ַ�������Χ��ASCII��ɼ��ַ���ɵ��ַ���
 * @param ... ��ʽ���ַ��������б�
 * @return ��
 * @note ���ô˺�����Ҫ�������س�������Ļ�ϣ�������ø��º���
 */
void OLED_Printf(uint8_t X, uint8_t Y, uint8_t FontSize, char *format, ...)
{
    char String[30];                         // �����ַ�����
    va_list arg;                             // ����ɱ�����б��������͵ı���arg
    va_start(arg, format);                   // ��format��ʼ�����ղ����б���arg����
    vsprintf(String, format, arg);           // ʹ��vsprintf��ӡ��ʽ���ַ����Ͳ����б����ַ�������
    va_end(arg);                             // ��������arg
    OLED_ShowString(X, Y, String, FontSize); // OLED��ʾ�ַ����飨�ַ�����
}

/**
 * @brief OLED��ָ��λ�û�һ����
 * @param X ָ����ĺ����꣬��Χ��0-127
 * @param Y ָ����������꣬��Χ��0-63
 * @return ��
 * @note ���ô˺�����Ҫ�������س�������Ļ�ϣ�������ø��º���
 */
void OLED_DrawPoint(uint8_t X, uint8_t Y)
{
    if (X > 127) { return; }
    if (Y > 63) { return; }

    OLED_DisplayBuf[Y / 8][X] |= 0x01 << (Y % 8);
	PageNeedFlash |= (0x01 << (Y>>3));
}

/**
 * @brief OLED��ȡָ��λ�õ��ֵ
 * @param X ָ����ĺ����꣬��Χ��0-127
 * @param Y ָ����������꣬��Χ��0-63
 * @return ָ��λ�õ��Ƿ��ڵ���״̬��1��������0��Ϩ��
 */
uint8_t OLED_GetPoint(uint8_t X, uint8_t Y)
{
    if (X > 127) { return 0; }
    if (Y > 63) { return 0; }

    if (OLED_DisplayBuf[Y / 8][X] & 0x01 << (Y % 8)) {
        return 1; // Ϊ1������1
    }

    return 0; // ���򣬷���0
}

/**
 * @brief OLED����
 * @param X0 ָ��һ���˵�ĺ����꣬��Χ��0-127
 * @param Y0 ָ��һ���˵�������꣬��Χ��0-63
 * @param X1 ָ����һ���˵�ĺ����꣬��Χ��0-127
 * @param Y1 ָ����һ���˵�������꣬��Χ��0-63
 * @return ��
 * @note ���ô˺�����Ҫ�������س�������Ļ�ϣ�������ø��º���
 */
void OLED_DrawLine(uint8_t X0, uint8_t Y0, uint8_t X1, uint8_t Y1)
{
    int16_t x, y, dx, dy, d, incrE, incrNE, temp;
    int16_t x0 = X0, y0 = Y0, x1 = X1, y1 = Y1;
    uint8_t yflag = 0, xyflag = 0;

    if (y0 == y1) // ���ߵ�������
    {
        if (x0 > x1) {
            temp = x0;
            x0   = x1;
            x1   = temp;
        }

        for (x = x0; x <= x1; x++) {
            OLED_DrawPoint(x, y0); // ���λ���
        }
    } else if (x0 == x1) // ���ߵ�������
    {
        if (y0 > y1) {
            temp = y0;
            y0   = y1;
            y1   = temp;
        }

        for (y = y0; y <= y1; y++) {
            OLED_DrawPoint(x0, y); // ���λ���
        }
    } else // б��
    {

        if (x0 > x1) // 0�ŵ�X�������1�ŵ�X����
        {
            temp = x0;
            x0   = x1;
            x1   = temp;
            temp = y0;
            y0   = y1;
            y1   = temp;
        }

        if (y0 > y1) // 0�ŵ�Y�������1�ŵ�Y����
        {
            y0 = -y0;
            y1 = -y1;

            yflag = 1;
        }

        if (y1 - y0 > x1 - x0) // ����б�ʴ���1
        {
            temp = x0;
            x0   = y0;
            y0   = temp;
            temp = x1;
            x1   = y1;
            y1   = temp;

            xyflag = 1;
        }

        dx     = x1 - x0;
        dy     = y1 - y0;
        incrE  = 2 * dy;
        incrNE = 2 * (dy - dx);
        d      = 2 * dy - dx;
        x      = x0;
        y      = y0;

        if (yflag && xyflag) {
            OLED_DrawPoint(y, -x);
        } else if (yflag) {
            OLED_DrawPoint(x, -y);
        } else if (xyflag) {
            OLED_DrawPoint(y, x);
        } else {
            OLED_DrawPoint(x, y);
        }

        while (x < x1) // ����X���ÿ����
        {
            x++;
            if (d < 0) // ��һ�����ڵ�ǰ�㶫��
            {
                d += incrE;
            } else // ��һ�����ڵ�ǰ�㶫����
            {
                y++;
                d += incrNE;
            }

            if (yflag && xyflag) {
                OLED_DrawPoint(y, -x);
            } else if (yflag) {
                OLED_DrawPoint(x, -y);
            } else if (xyflag) {
                OLED_DrawPoint(y, x);
            } else {
                OLED_DrawPoint(x, y);
            }
        }
    }
}

/**
 * @brief OLED����
 * @param X ָ���������Ͻǵĺ����꣬��Χ��0~127
 * @param Y ָ���������Ͻǵ������꣬��Χ��0~63
 * @param Width ָ�����εĿ��ȣ���Χ��0~128
 * @param Height ָ�����εĸ߶ȣ���Χ��0~64
 * @param IsFilled ָ�������Ƿ����
 *           ��Χ��OLED_UNFILLED		�����
 *                 OLED_FILLED			���
 * @return ��
 * @note ���ô˺�����Ҫ�������س�������Ļ�ϣ�������ø��º���
 */
void OLED_DrawRectangle(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height, uint8_t IsFilled)
{
    uint8_t i, j;
    if (!IsFilled) // ָ�����β����
    {
        for (i = X; i < X + Width; i++) {
            OLED_DrawPoint(i, Y);
            OLED_DrawPoint(i, Y + Height - 1);
        }
        for (i = Y; i < Y + Height; i++) {
            OLED_DrawPoint(X, i);
            OLED_DrawPoint(X + Width - 1, i);
        }
    } else // ָ���������
    {
        for (i = X; i < X + Width; i++) {
            for (j = Y; j < Y + Height; j++) {
                OLED_DrawPoint(i, j);
            }
        }
    }
}

/**
 * @brief OLED������
 * @param X0 ָ����һ���˵�ĺ����꣬��Χ��0-127
 * @param Y0 ָ����һ���˵�������꣬��Χ��0-63
 * @param X1 ָ���ڶ����˵�ĺ����꣬��Χ��0-127
 * @param Y1 ָ���ڶ����˵�������꣬��Χ��0-63
 * @param X2 ָ���������˵�ĺ����꣬��Χ��0-127
 * @param Y2 ָ���������˵�������꣬��Χ��0-63
 * @param IsFilled ָ���������Ƿ����
 *           ��Χ��OLED_UNFILLED		�����
 *                 OLED_FILLED			���
 * @return ��
 * @note ���ô˺�����Ҫ�������س�������Ļ�ϣ�������ø��º���
 */
void OLED_DrawTriangle(uint8_t X0, uint8_t Y0, uint8_t X1, uint8_t Y1, uint8_t X2, uint8_t Y2, uint8_t IsFilled)
{
    uint8_t minx = X0, miny = Y0, maxx = X0, maxy = Y0;
    uint8_t i, j;
    int16_t vx[] = {X0, X1, X2};
    int16_t vy[] = {Y0, Y1, Y2};

    if (!IsFilled) // ָ�������β����
    {
        OLED_DrawLine(X0, Y0, X1, Y1);
        OLED_DrawLine(X0, Y0, X2, Y2);
        OLED_DrawLine(X1, Y1, X2, Y2);
    } else // ָ�����������
    {
        if (X1 < minx) { minx = X1; }
        if (X2 < minx) { minx = X2; }
        if (Y1 < miny) { miny = Y1; }
        if (Y2 < miny) { miny = Y2; }

        if (X1 > maxx) { maxx = X1; }
        if (X2 > maxx) { maxx = X2; }
        if (Y1 > maxy) { maxy = Y1; }
        if (Y2 > maxy) { maxy = Y2; }

        for (i = minx; i <= maxx; i++) {
            for (j = miny; j <= maxy; j++) {
                if (OLED_pnpoly(3, vx, vy, i, j)) { OLED_DrawPoint(i, j); }
            }
        }
    }
}

/**
 * @brief OLED��Բ
 * @param X ָ��Բ��Բ�ĺ����꣬��Χ��0~127
 * @param Y ָ��Բ��Բ�������꣬��Χ��0~63
 * @param Radius ָ��Բ�İ뾶����Χ��0~255
 * @param IsFilled ָ��Բ�Ƿ����
 *           ��Χ��OLED_UNFILLED		�����
 *                 OLED_FILLED			���
 * @return ��
 * @note ���ô˺�����Ҫ�������س�������Ļ�ϣ�������ø��º���
 */
void OLED_DrawCircle(uint8_t X, uint8_t Y, uint8_t Radius, uint8_t IsFilled)
{
    int16_t x, y, d, j;


    d = 1 - Radius;
    x = 0;
    y = Radius;

    OLED_DrawPoint(X + x, Y + y);
    OLED_DrawPoint(X - x, Y - y);
    OLED_DrawPoint(X + y, Y + x);
    OLED_DrawPoint(X - y, Y - x);

    if (IsFilled) // ָ��Բ���
    {
        for (j = -y; j < y; j++) {
            OLED_DrawPoint(X, Y + j);
        }
    }

    while (x < y) // ����X���ÿ����
    {
        x++;
        if (d < 0) // ��һ�����ڵ�ǰ�㶫��
        {
            d += 2 * x + 1;
        } else // ��һ�����ڵ�ǰ�㶫�Ϸ�
        {
            y--;
            d += 2 * (x - y) + 1;
        }

        OLED_DrawPoint(X + x, Y + y);
        OLED_DrawPoint(X + y, Y + x);
        OLED_DrawPoint(X - x, Y - y);
        OLED_DrawPoint(X - y, Y - x);
        OLED_DrawPoint(X + x, Y - y);
        OLED_DrawPoint(X + y, Y - x);
        OLED_DrawPoint(X - x, Y + y);
        OLED_DrawPoint(X - y, Y + x);

        if (IsFilled) // ָ��Բ���
        {
            for (j = -y; j < y; j++) {
                OLED_DrawPoint(X + x, Y + j);
                OLED_DrawPoint(X - x, Y + j);
            }

            for (j = -x; j < x; j++) {
                OLED_DrawPoint(X - y, Y + j);
                OLED_DrawPoint(X + y, Y + j);
            }
        }
    }
}

/**
 * @brief OLED����Բ
 * @param X ָ����Բ��Բ�ĺ����꣬��Χ��0~127
 * @param Y ָ����Բ��Բ�������꣬��Χ��0~63
 * @param A ָ����Բ�ĺ�����᳤�ȣ���Χ��0~255
 * @param B ָ����Բ��������᳤�ȣ���Χ��0~255
 * @param IsFilled ָ����Բ�Ƿ����
 *           ��Χ��OLED_UNFILLED		�����
 *                 OLED_FILLED			���
 * @return ��
 * @note ���ô˺�����Ҫ�������س�������Ļ�ϣ�������ø��º���
 */
void OLED_DrawEllipse(uint8_t X, uint8_t Y, uint8_t A, uint8_t B, uint8_t IsFilled)
{
    int16_t x, y, j;
    int16_t a = A, b = B;
    float d1, d2;


    x  = 0;
    y  = b;
    d1 = b * b + a * a * (-b + 0.5);

    if (IsFilled) // ָ����Բ���
    {
        for (j = -y; j < y; j++) {
            OLED_DrawPoint(X, Y + j);
            OLED_DrawPoint(X, Y + j);
        }
    }

    OLED_DrawPoint(X + x, Y + y);
    OLED_DrawPoint(X - x, Y - y);
    OLED_DrawPoint(X - x, Y + y);
    OLED_DrawPoint(X + x, Y - y);

    while (b * b * (x + 1) < a * a * (y - 0.5)) {
        if (d1 <= 0) // ��һ�����ڵ�ǰ�㶫��
        {
            d1 += b * b * (2 * x + 3);
        } else // ��һ�����ڵ�ǰ�㶫�Ϸ�
        {
            d1 += b * b * (2 * x + 3) + a * a * (-2 * y + 2);
            y--;
        }
        x++;

        if (IsFilled) // ָ����Բ���
        {
            for (j = -y; j < y; j++) {
                OLED_DrawPoint(X + x, Y + j);
                OLED_DrawPoint(X - x, Y + j);
            }
        }

        OLED_DrawPoint(X + x, Y + y);
        OLED_DrawPoint(X - x, Y - y);
        OLED_DrawPoint(X - x, Y + y);
        OLED_DrawPoint(X + x, Y - y);
    }

    d2 = b * b * (x + 0.5) * (x + 0.5) + a * a * (y - 1) * (y - 1) - a * a * b * b;

    while (y > 0) {
        if (d2 <= 0) // ��һ�����ڵ�ǰ�㶫��
        {
            d2 += b * b * (2 * x + 2) + a * a * (-2 * y + 3);
            x++;

        } else // ��һ�����ڵ�ǰ�㶫�Ϸ�
        {
            d2 += a * a * (-2 * y + 3);
        }
        y--;

        if (IsFilled) // ָ����Բ���
        {
            for (j = -y; j < y; j++) {
                OLED_DrawPoint(X + x, Y + j);
                OLED_DrawPoint(X - x, Y + j);
            }
        }

        OLED_DrawPoint(X + x, Y + y);
        OLED_DrawPoint(X - x, Y - y);
        OLED_DrawPoint(X - x, Y + y);
        OLED_DrawPoint(X + x, Y - y);
    }
}

/**
 * @brief OLED��Բ��
 * @param X ָ��Բ����Բ�ĺ����꣬��Χ��0~127
 * @param Y ָ��Բ����Բ�������꣬��Χ��0~63
 * @param Radius ָ��Բ���İ뾶����Χ��0~255
 * @param StartAngle ָ��Բ������ʼ�Ƕȣ���Χ��-180~180
 *           ˮƽ����Ϊ0�ȣ�ˮƽ����Ϊ180�Ȼ�-180�ȣ��·�Ϊ�������Ϸ�Ϊ������˳ʱ����ת
 * @param EndAngle ָ��Բ������ֹ�Ƕȣ���Χ��-180~180
 *           ˮƽ����Ϊ0�ȣ�ˮƽ����Ϊ180�Ȼ�-180�ȣ��·�Ϊ�������Ϸ�Ϊ������˳ʱ����ת
 * @param IsFilled ָ��Բ���Ƿ���䣬����Ϊ����
 *           ��Χ��OLED_UNFILLED		�����
 *                 OLED_FILLED			���
 * @return ��
 * @note ���ô˺�����Ҫ�������س�������Ļ�ϣ�������ø��º���
 */
void OLED_DrawArc(uint8_t X, uint8_t Y, uint8_t Radius, int16_t StartAngle, int16_t EndAngle, uint8_t IsFilled)
{
    int16_t x, y, d, j;


    d = 1 - Radius;
    x = 0;
    y = Radius;

    if (OLED_IsInAngle(x, y, StartAngle, EndAngle)) { OLED_DrawPoint(X + x, Y + y); }
    if (OLED_IsInAngle(-x, -y, StartAngle, EndAngle)) { OLED_DrawPoint(X - x, Y - y); }
    if (OLED_IsInAngle(y, x, StartAngle, EndAngle)) { OLED_DrawPoint(X + y, Y + x); }
    if (OLED_IsInAngle(-y, -x, StartAngle, EndAngle)) { OLED_DrawPoint(X - y, Y - x); }

    if (IsFilled) // ָ��Բ�����
    {
        for (j = -y; j < y; j++) {
            if (OLED_IsInAngle(0, j, StartAngle, EndAngle)) { OLED_DrawPoint(X, Y + j); }
        }
    }

    while (x < y) // ����X���ÿ����
    {
        x++;
        if (d < 0) // ��һ�����ڵ�ǰ�㶫��
        {
            d += 2 * x + 1;
        } else // ��һ�����ڵ�ǰ�㶫�Ϸ�
        {
            y--;
            d += 2 * (x - y) + 1;
        }

        if (OLED_IsInAngle(x, y, StartAngle, EndAngle)) { OLED_DrawPoint(X + x, Y + y); }
        if (OLED_IsInAngle(y, x, StartAngle, EndAngle)) { OLED_DrawPoint(X + y, Y + x); }
        if (OLED_IsInAngle(-x, -y, StartAngle, EndAngle)) { OLED_DrawPoint(X - x, Y - y); }
        if (OLED_IsInAngle(-y, -x, StartAngle, EndAngle)) { OLED_DrawPoint(X - y, Y - x); }
        if (OLED_IsInAngle(x, -y, StartAngle, EndAngle)) { OLED_DrawPoint(X + x, Y - y); }
        if (OLED_IsInAngle(y, -x, StartAngle, EndAngle)) { OLED_DrawPoint(X + y, Y - x); }
        if (OLED_IsInAngle(-x, y, StartAngle, EndAngle)) { OLED_DrawPoint(X - x, Y + y); }
        if (OLED_IsInAngle(-y, x, StartAngle, EndAngle)) { OLED_DrawPoint(X - y, Y + x); }

        if (IsFilled) // ָ��Բ�����
        {
            for (j = -y; j < y; j++) {
                if (OLED_IsInAngle(x, j, StartAngle, EndAngle)) { OLED_DrawPoint(X + x, Y + j); }
                if (OLED_IsInAngle(-x, j, StartAngle, EndAngle)) { OLED_DrawPoint(X - x, Y + j); }
            }

            for (j = -x; j < x; j++) {
                if (OLED_IsInAngle(-y, j, StartAngle, EndAngle)) { OLED_DrawPoint(X - y, Y + j); }
                if (OLED_IsInAngle(y, j, StartAngle, EndAngle)) { OLED_DrawPoint(X + y, Y + j); }
            }
        }
    }
}

