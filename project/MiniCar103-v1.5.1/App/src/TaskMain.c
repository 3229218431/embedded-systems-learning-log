/**
  ******************************************************************************
  * @file           : TaskMain.c
  * @brief          : 任务调度主文件
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 浙江工业大学 2025-2035
  * All rights reserved.
  *
  * 本文件仅供学习使用，未经许可不得擅改，违者必究;
  * 浙江工业大学 信息科学与工程学院 人工智能专业
  * 源码地址：https://gitee.com/NEagle
  * 修改时间：2026/04/01
  * 版本： V1.5.2
  * 版权所有，禁止滥用
  * V1.5.2修改说明：
  * - 新增提前量预测补偿算法初始化
  *
  ******************************************************************************
  */
#include "main.h"
#include "TaskMain.h"
#include "Board.h"
#include "UsrTimer.h"  // 包含PID参数和状态变量
#include <string.h>    // 字符串处理函数（memcpy, strstr等）
#include <stdlib.h>    // 标准库函数（atoi, atof等）

/* 延迟补偿算法使用到的传感器权重数组 */
extern const int8_t sensor_weights[8];

/* 蓝牙数据传输控制变量定义 */
volatile uint8_t g_bluetooth_data_enable = 1;      // 蓝牙数据传输使能标志（1=启用，0=禁用）
volatile uint16_t g_bluetooth_data_interval = 50;  // 蓝牙数据传输间隔（毫秒，默认50ms）

/* 任务函数声明 */
void InitFunc(void);      // 初始化任务
void Led1Func(void);      // LED1闪烁任务
void Led2Func(void);      // LED2闪烁任务
void BuzzFunc(void);      // 蜂鸣器任务
void Uart1Func(void);     // UART1接收转发任务
void Uart3Func(void);     // UART3接收转发任务
void PntFunc(void);       // 打印计数任务
void ChkKey0Func(void);   // 按键0扫描任务
void ChkKey1Func(void);   // 按键1扫描任务
void ChkSenFunc(void);    // 传感器状态监控任务
void BluetoothDataFunc(void); // 蓝牙数据传输任务

/* JSON构建函数声明 */
int build_sensor_json(char *buf, int size);
int build_status_json(char *buf, int size, const char *cmd, const char *status, const char *data);

/* 任务组件数组 - 定义所有任务的调度信息
   每个任务包含：运行标志、当前定时器、调度周期、任务函数指针
   采用时间片轮询调度机制，SysTick每个1ms递减定时器
*/
TASK_COMPONENTS TaskComps[TASK_MAX] = {
    {0, 1, 1, InitFunc},             // Initialization task: 1ms period, priority
    {0, 1, 4, ChkKey0Func},          // Key0 scan task: 4ms period, fast response
    {0, 1, 4, ChkKey1Func},          // Key1 scan task: 4ms period, fast response
    {0, 5, 10, OLED_Update_InPages}, // OLED page refresh task: 10ms period
    {0, 3, 100, BuzzFunc},           // Buzzer task: 100ms period
    {0, 4, 100, Uart1Func},          // UART1 task: 100ms period
    {0, 4, 100, Uart3Func},          // UART3 task: 100ms period
    {0, 5, 50, BluetoothDataFunc},   // Bluetooth data transmission: 50ms period (20Hz)
    {0, 4, 100, ChkSenFunc},         // Sensor monitor task: 100ms period
    {0, 4, 500, AdcReadTask},        // Battery voltage reading task: 500ms period
    {0, 3, 1000, Led1Func},          // LED1 task: 1000ms period (1Hz blink)
    {0, 3, 2000, Led2Func},          // LED2 task: 2000ms period (0.5Hz blink)
    {0, 3, 1000, PntFunc},           // Print task: 1000ms period
};

/**
  * @brief  按键0扫描任务（循迹使能开关）
  * @param  无
  * @retval 无
  * @details 功能：
  *          - 切换循迹控制的使能状态
  *          - 在OLED上显示当前状态（ON/OFF）
  *          - 禁用时停止电机
  *
  * 按键消抖：通过连续检测4次以上保持一致来确认按键状态
  */
void ChkKey0Func(void)
{
    static uint8_t Cont = 0;              // 连续检测计数器
    static uint8_t key_pressed = 0;       // 按键按下标志，防止重复触发
    static uint8_t initialized = 0;       // 初始化标志

    /* 首次执行时显示初始状态 */
    if (!initialized) {
        initialized = 1;
        if (g_track_enable) {
            OLED_ShowString(96, 0, "ON ", OLED_8X16);
        } else {
            OLED_ShowString(96, 0, "OFF", OLED_8X16);
        }
    }

    /* 检测按键状态 */
    if(GetKey0Val()) {  // 按键按下（低电平）
        Cont ++;         // 连续检测计数器增加
    } else {
        if(Cont > 4 && !key_pressed) {  // 确认按键有效且未处理过
            key_pressed = 1;             // 设置按键已处理标志
            /* 切换循迹使能标志 */
            g_track_enable = !g_track_enable;
            printf("\t\tTrack %s\r\n", g_track_enable ? "ENABLED" : "DISABLED");

            /* 在OLED上显示状态 */
            if (g_track_enable) {
                OLED_ShowString(96, 0, "ON ", OLED_8X16);
            } else {
                OLED_ShowString(96, 0, "OFF", OLED_8X16);
                /* 确保电机停止 */
                Motor_SetSpeed(&motor_left, 0);
                Motor_SetSpeed(&motor_right, 0);
            }
        } else if (Cont <= 4) {
            /* 短按或抖动，重置按键标志 */
            key_pressed = 0;
        }
        Cont = 0;  // 重置连续检测计数器
    }
}

/**
  * @brief  按键1扫描任务（速度模式切换）
  * @param  无
  * @retval 无
  * @details 功能：
  *          - 循环切换三种速度模式：低速->中速->高速->低速
  *          - 在OLED上显示当前速度模式
  *          - 对应的基础速度：0=400, 1=600, 2=800
  *
  * 按键消抖：通过连续检测4次以上保持一致来确认按键状态
  */
void ChkKey1Func(void)
{
    static uint8_t Cont = 0;              // 连续检测计数器
    static uint8_t key_pressed = 0;       // 按键按下标志，防止重复触发
    static uint8_t initialized = 0;       // 初始化标志

    /* 首次执行时显示初始状态 */
    if (!initialized) {
        initialized = 1;
        /* 根据当前速度模式显示 */
        switch(g_speed_mode) {
            case 0: OLED_ShowString(0, 0, "LOW ", OLED_8X16); break;
            case 1: OLED_ShowString(0, 0, "MID ", OLED_8X16); break;
            case 2: OLED_ShowString(0, 0, "HIGH", OLED_8X16); break;
        }
    }

    /* 检测按键状态 */
    if(GetKey1Val()) {  // 按键按下（低电平）
        Cont ++;         // 连续检测计数器增加
    } else {
        if(Cont > 4 && !key_pressed) {  // 确认按键有效且未处理过
            key_pressed = 1;             // 设置按键已处理标志

            /* 切换速度模式：0->1->2->0 */
            g_speed_mode = (g_speed_mode + 1) % 3;

            /* 根据模式设置基础速度 */
            switch(g_speed_mode) {
                case 0: baseSpeed = 400; break;  // 低速
                case 1: baseSpeed = 600; break;  // 中速（默认）
                case 2: baseSpeed = 800; break;  // 高速
            }

            printf("\t\tSpeed Mode: %d, BaseSpeed: %d\r\n", g_speed_mode, baseSpeed);

            /* 在OLED上显示速度模式 */
            switch(g_speed_mode) {
                case 0: OLED_ShowString(0, 0, "LOW ", OLED_8X16); break;
                case 1: OLED_ShowString(0, 0, "MID ", OLED_8X16); break;
                case 2: OLED_ShowString(0, 0, "HIGH", OLED_8X16); break;
            }

            /* 如果循迹禁用，更新显示（坐标与ChkKey0Func一致） */
            if (!g_track_enable) {
                OLED_ShowString(96, 0, "OFF", OLED_8X16);
            }
        } else if (Cont <= 4) {
            /* 短按，重置按键标志 */
            key_pressed = 0;
        }
        Cont = 0;  // 重置连续检测计数器
    }
}

/**
  * @brief  初始化任务（在其他任务之前执行）
  * @param  无
  * @retval 无
  * @details 在系统启动时执行一次，初始化延迟补偿算法参数
  */
void InitFunc(void)
{
    static uint8_t initialized = 0;
    if (!initialized) {
        initialized = 1;
        /* 初始化延迟补偿算法 */
        AdvanceCompensation_Init();
    }
}

/**
  * @brief  LED1翻转任务
  * @param  无
  * @retval 无
  * @details 每1000ms（1秒）翻转一次LED1状态
  */
void Led1Func(void)
{
    CoreLedToggle();  // 翻转核心板LED
}

/**
  * @brief  LED2翻转任务
  * @param  无
  * @retval 无
  * @details 每2000ms（2秒）翻转一次LED2状态
  */
void Led2Func(void)
{
    BrdLedToggle();  // 翻转扩展板LED
}

/**
  * @brief  蜂鸣器任务
  * @param  无
  * @retval 无
  * @details 每100ms执行一次，控制蜂鸣器工作模式：
  *          - 开机前300ms连续鸣叫
  *          - 之后进入循环模式
  */
void BuzzFunc(void)
{
    static unsigned char cnt = 0;
    cnt ++;  // 增加计数器（每个100ms加1）

    switch(cnt) {
        case 3:  // 第3个100ms（300ms）时打开蜂鸣器
            BuzzON();
            break;
        case 4:  // 第4个100ms（400ms）时关闭蜂鸣器
            BuzzOFF();
            break;
        case 40:  // 第40个100ms（4秒）时重置计数器
            cnt = 0;
            break;
        default:
            break;
    }
}

/**
  * @brief  UART1接收转发任务
  * @param  无
  * @retval 无
  * @details 每100ms检查一次UART1接收缓冲区
  *          将接收到的数据直接转发回UART1（回显功能）
  */
void Uart1Func(void)
{
    int len;
    uint8_t buf[64];
    len = uart_fifo_gets(&g_uart1, buf, 64);  // 从FIFO读取数据
    if(len>0)
        uart_fifo_puts(&g_uart1, buf, len);   // 回显数据
}

/**
  * @brief  解析并处理蓝牙JSON命令
  * @param  buf: 接收到的数据缓冲区
  * @param  len: 数据长度
  * @retval 无
  * @details 支持的命令：
  *          {"cmd":"ENABLE"}  / {"cmd":"DISABLE"}
  *          {"cmd":"SETFREQ","value":50}
  *          {"cmd":"STATUS"}  / {"cmd":"HELP"}
  *          {"cmd":"SETPID","kp":15.0,"ki":0.0,"kd":5.0}
  *          {"cmd":"GETPID"}
  *          {"cmd":"TRACK","enable":1}
  *          {"cmd":"SPEED","mode":1}
  *          {"cmd":"MANUAL","left":400,"right":400}
  */
static void parse_bluetooth_command(uint8_t *buf, int len) {
    /* 确保以null终止 */
    if (len >= 64) len = 63;
    char str_buf[64];
    memcpy(str_buf, buf, len);
    str_buf[len] = 0;

    /* 简单JSON解析 - 查找关键字段 */
    char *cmd_ptr = strstr(str_buf, "\"cmd\"");
    if (cmd_ptr == NULL) {
        /* 不是有效的JSON命令，保持回显功能 */
        uart_fifo_puts(&g_uart3, buf, len);
        return;
    }

    /* 提取命令值 */
    char *colon = strstr(cmd_ptr, ":");
    if (colon == NULL) return;
    char *quote1 = strstr(colon, "\"");
    if (quote1 == NULL) return;
    char *quote2 = strstr(quote1 + 1, "\"");
    if (quote2 == NULL) return;

    /* 提取命令字符串 */
    char cmd_name[32];
    int cmd_len = quote2 - (quote1 + 1);
    if (cmd_len >= sizeof(cmd_name)) cmd_len = sizeof(cmd_name) - 1;
    memcpy(cmd_name, quote1 + 1, cmd_len);
    cmd_name[cmd_len] = 0;

    /* 准备响应缓冲区 */
    char resp_buf[256];
    char data_buf[128];

    /* 根据命令执行相应操作 */
    if (strcmp(cmd_name, "ENABLE") == 0) {
        g_bluetooth_data_enable = 1;
        snprintf(data_buf, sizeof(data_buf), "{\"enabled\":1}");
        build_status_json(resp_buf, sizeof(resp_buf), "ENABLE", "OK", data_buf);
        uart_fifo_puts(&g_uart3, (uint8_t *)resp_buf, strlen(resp_buf));
    }
    else if (strcmp(cmd_name, "DISABLE") == 0) {
        g_bluetooth_data_enable = 0;
        snprintf(data_buf, sizeof(data_buf), "{\"enabled\":0}");
        build_status_json(resp_buf, sizeof(resp_buf), "DISABLE", "OK", data_buf);
        uart_fifo_puts(&g_uart3, (uint8_t *)resp_buf, strlen(resp_buf));
    }
    else if (strcmp(cmd_name, "SETFREQ") == 0) {
        /* 提取value字段 */
        char *value_ptr = strstr(str_buf, "\"value\"");
        if (value_ptr) {
            char *val_colon = strstr(value_ptr, ":");
            if (val_colon) {
                int value = atoi(val_colon + 1);
                if (value >= 10 && value <= 1000) {
                    g_bluetooth_data_interval = value;
                    snprintf(data_buf, sizeof(data_buf), "{\"interval\":%d}", value);
                    build_status_json(resp_buf, sizeof(resp_buf), "SETFREQ", "OK", data_buf);
                } else {
                    snprintf(data_buf, sizeof(data_buf), "{\"error\":\"Value out of range (10-1000)\"}");
                    build_status_json(resp_buf, sizeof(resp_buf), "SETFREQ", "ERROR", data_buf);
                }
                uart_fifo_puts(&g_uart3, (uint8_t *)resp_buf, strlen(resp_buf));
                return;
            }
        }
        snprintf(data_buf, sizeof(data_buf), "{\"error\":\"Missing value parameter\"}");
        build_status_json(resp_buf, sizeof(resp_buf), "SETFREQ", "ERROR", data_buf);
        uart_fifo_puts(&g_uart3, (uint8_t *)resp_buf, strlen(resp_buf));
    }
    else if (strcmp(cmd_name, "STATUS") == 0) {
        /* 返回当前状态信息 */
        snprintf(data_buf, sizeof(data_buf),
            "{\"bluetooth\":{\"enabled\":%d,\"interval\":%d},"
            "\"track\":{\"enable\":%d,\"speedMode\":%d,\"baseSpeed\":%d},"
            "\"pid\":{\"Kp\":%.1f,\"Ki\":%.1f,\"Kd\":%.1f}}",
            g_bluetooth_data_enable, g_bluetooth_data_interval,
            g_track_enable, g_speed_mode, baseSpeed,
            Kp, Ki, Kd);
        build_status_json(resp_buf, sizeof(resp_buf), "STATUS", "OK", data_buf);
        uart_fifo_puts(&g_uart3, (uint8_t *)resp_buf, strlen(resp_buf));
    }
    else if (strcmp(cmd_name, "HELP") == 0) {
        /* 返回帮助信息 */
        const char *help_text =
            "{\"commands\":["
            "\"ENABLE\",\"DISABLE\",\"SETFREQ\",\"STATUS\",\"HELP\","
            "\"SETPID\",\"GETPID\",\"TRACK\",\"SPEED\",\"MANUAL\""
            "]}";
        build_status_json(resp_buf, sizeof(resp_buf), "HELP", "OK", help_text);
        uart_fifo_puts(&g_uart3, (uint8_t *)resp_buf, strlen(resp_buf));
    }
    else if (strcmp(cmd_name, "SETPID") == 0) {
        /* 提取PID参数 */
        float kp = Kp, ki = Ki, kd = Kd;
        char *kp_ptr = strstr(str_buf, "\"kp\"");
        char *ki_ptr = strstr(str_buf, "\"ki\"");
        char *kd_ptr = strstr(str_buf, "\"kd\"");

        if (kp_ptr) { char *colon = strstr(kp_ptr, ":"); if (colon) kp = atof(colon + 1); }
        if (ki_ptr) { char *colon = strstr(ki_ptr, ":"); if (colon) ki = atof(colon + 1); }
        if (kd_ptr) { char *colon = strstr(kd_ptr, ":"); if (colon) kd = atof(colon + 1); }

        Kp = kp; Ki = ki; Kd = kd;
        snprintf(data_buf, sizeof(data_buf), "{\"Kp\":%.1f,\"Ki\":%.1f,\"Kd\":%.1f}", kp, ki, kd);
        build_status_json(resp_buf, sizeof(resp_buf), "SETPID", "OK", data_buf);
        uart_fifo_puts(&g_uart3, (uint8_t *)resp_buf, strlen(resp_buf));
    }
    else if (strcmp(cmd_name, "GETPID") == 0) {
        snprintf(data_buf, sizeof(data_buf), "{\"Kp\":%.1f,\"Ki\":%.1f,\"Kd\":%.1f}", Kp, Ki, Kd);
        build_status_json(resp_buf, sizeof(resp_buf), "GETPID", "OK", data_buf);
        uart_fifo_puts(&g_uart3, (uint8_t *)resp_buf, strlen(resp_buf));
    }
    else if (strcmp(cmd_name, "TRACK") == 0) {
        /* 提取enable字段 */
        char *enable_ptr = strstr(str_buf, "\"enable\"");
        if (enable_ptr) {
            char *colon = strstr(enable_ptr, ":");
            if (colon) {
                int enable = atoi(colon + 1);
                g_track_enable = (enable != 0);
                snprintf(data_buf, sizeof(data_buf), "{\"trackEnable\":%d}", g_track_enable);
                build_status_json(resp_buf, sizeof(resp_buf), "TRACK", "OK", data_buf);
                uart_fifo_puts(&g_uart3, (uint8_t *)resp_buf, strlen(resp_buf));
                return;
            }
        }
        snprintf(data_buf, sizeof(data_buf), "{\"error\":\"Missing enable parameter\"}");
        build_status_json(resp_buf, sizeof(resp_buf), "TRACK", "ERROR", data_buf);
        uart_fifo_puts(&g_uart3, (uint8_t *)resp_buf, strlen(resp_buf));
    }
    else if (strcmp(cmd_name, "SPEED") == 0) {
        /* 提取mode字段 */
        char *mode_ptr = strstr(str_buf, "\"mode\"");
        if (mode_ptr) {
            char *colon = strstr(mode_ptr, ":");
            if (colon) {
                int mode = atoi(colon + 1);
                if (mode >= 0 && mode <= 2) {
                    g_speed_mode = mode;
                    switch(mode) {
                        case 0: baseSpeed = 400; break;
                        case 1: baseSpeed = 600; break;
                        case 2: baseSpeed = 800; break;
                    }
                    snprintf(data_buf, sizeof(data_buf), "{\"speedMode\":%d,\"baseSpeed\":%d}", mode, baseSpeed);
                    build_status_json(resp_buf, sizeof(resp_buf), "SPEED", "OK", data_buf);
                } else {
                    snprintf(data_buf, sizeof(data_buf), "{\"error\":\"Mode must be 0,1,2\"}");
                    build_status_json(resp_buf, sizeof(resp_buf), "SPEED", "ERROR", data_buf);
                }
                uart_fifo_puts(&g_uart3, (uint8_t *)resp_buf, strlen(resp_buf));
                return;
            }
        }
        snprintf(data_buf, sizeof(data_buf), "{\"error\":\"Missing mode parameter\"}");
        build_status_json(resp_buf, sizeof(resp_buf), "SPEED", "ERROR", data_buf);
        uart_fifo_puts(&g_uart3, (uint8_t *)resp_buf, strlen(resp_buf));
    }
    else if (strcmp(cmd_name, "MANUAL") == 0) {
        /* 手动控制电机速度 */
        char *left_ptr = strstr(str_buf, "\"left\"");
        char *right_ptr = strstr(str_buf, "\"right\"");
        if (left_ptr && right_ptr) {
            char *left_colon = strstr(left_ptr, ":");
            char *right_colon = strstr(right_ptr, ":");
            if (left_colon && right_colon) {
                int left = atoi(left_colon + 1);
                int right = atoi(right_colon + 1);
                /* 限制范围 */
                if (left < 0) left = 0; if (left > 999) left = 999;
                if (right < 0) right = 0; if (right > 999) right = 999;

                /* 设置电机速度 */
                Motor_SetSpeed(&motor_left, left);
                Motor_SetSpeed(&motor_right, right);
                /* 暂时禁用循迹控制 */
                g_track_enable = 0;

                snprintf(data_buf, sizeof(data_buf), "{\"left\":%d,\"right\":%d,\"trackEnabled\":0}", left, right);
                build_status_json(resp_buf, sizeof(resp_buf), "MANUAL", "OK", data_buf);
                uart_fifo_puts(&g_uart3, (uint8_t *)resp_buf, strlen(resp_buf));
                return;
            }
        }
        snprintf(data_buf, sizeof(data_buf), "{\"error\":\"Missing left/right parameters\"}");
        build_status_json(resp_buf, sizeof(resp_buf), "MANUAL", "ERROR", data_buf);
        uart_fifo_puts(&g_uart3, (uint8_t *)resp_buf, strlen(resp_buf));
    }
    else {
        /* 未知命令 */
        snprintf(data_buf, sizeof(data_buf), "{\"error\":\"Unknown command\"}");
        build_status_json(resp_buf, sizeof(resp_buf), cmd_name, "ERROR", data_buf);
        uart_fifo_puts(&g_uart3, (uint8_t *)resp_buf, strlen(resp_buf));
    }
}

/**
  * @brief  UART3接收转发任务（扩展为蓝牙命令解析）
  * @param  无
  * @retval 无
  * @details 每100ms检查一次UART3接收缓冲区
  *          如果是JSON格式命令则解析执行，否则保持回显功能
  */
void Uart3Func(void)
{
    int len;
    uint8_t buf[64];
    len = uart_fifo_gets(&g_uart3, buf, 64);  // 从FIFO读取数据
    if(len > 0) {
        /* 检查是否为JSON命令（以{开头） */
        if (len >= 2 && buf[0] == '{') {
            parse_bluetooth_command(buf, len);
        } else {
            /* 非JSON数据，保持回显功能 */
            uart_fifo_puts(&g_uart3, buf, len);
        }
    }
}

/**
  * @brief  打印计数任务
  * @param  无
  * @retval 无
  * @details 每1000ms通过串口打印一次当前执行次数
  *          用于验证任务调度是否正常工作
  */
void PntFunc(void)
{
    static int cnt = 0;
    printf("In %03d Times\r\n", cnt++);  // 打印执行次数
}

/**
  * @brief  传感器状态监控任务
  * @param  无
  * @retval 无
  * @details 每100ms执行一次，用于监控和显示传感器状态
  *          第0行：显示速度模式和循迹状态
  *          第1行：显示实时参数（基础速度和误差）
  *          第2行：显示8个传感器的状态（I=检测到黑线，O=未检测到）
  *          第3行： cruis
  *
  * 传感器布局（从左到右）：
  * Sen0 Sen1 Sen2 Sen3 Sen4 Sen5 Sen6 Sen7
  *  I    O    O    I    I    O    O    O
  */
void ChkSenFunc(void)
{
    /* 第2行(Y=32)：显示实时参数 */
    char paramStr[17];  // 16字符+null终止符
    /* 格式："Spd:600 Err: 0" 或类似 */
    snprintf(paramStr, sizeof(paramStr), "S:%d E:%d", baseSpeed, lastError);
    OLED_ShowString(0, 32, paramStr, OLED_8X16);

    /* 第3行(Y=48)：显示传感器状态（更直观的方式） */
    uint8_t SensorVal[18];  // 18字符（16个状态字符+方括号+null终止符）
    /* 格式：[I O O I I O O O] 带方括号 */
    SensorVal[0] = '[';
    SensorVal[1] = GetSen0Val()? 'I':'O';
    SensorVal[2] = ' ';
    SensorVal[3] = GetSen1Val()? 'I':'O';
    SensorVal[4] = ' ';
    SensorVal[5] = GetSen2Val()? 'I':'O';
    SensorVal[6] = ' ';
    SensorVal[7] = GetSen3Val()? 'I':'O';
    SensorVal[8] = ' ';
    SensorVal[9] = GetSen4Val()? 'I':'O';
    SensorVal[10] = ' ';
    SensorVal[11] = GetSen5Val()? 'I':'O';
    SensorVal[12] = ' ';
    SensorVal[13] = GetSen6Val()? 'I':'O';
    SensorVal[14] = ' ';
    SensorVal[15] = GetSen7Val()? 'I':'O';
    SensorVal[16] = ']';
    SensorVal[17] = 0;  // null终止符
    /* 居中显示传感器状态 */
    OLED_ShowString(16, 48, (char *)SensorVal, OLED_8X16);

    /* 注意：此函数每100ms执行一次，用于监控传感器状态和显示实时参数 */
    /* 实际循迹控制由TIM1中断中的PID算法处理 */
}

/**
  * @brief  构建传感器数据JSON字符串
  * @param  buf: 输出缓冲区
  * @param  size: 缓冲区大小（至少128字节）
  * @retval 实际写入的字符数（不含null终止符）
  * @details 格式：
  *          {
  *            "type":"DATA",
  *            "tick":123456,
  *            "sensors":[1,0,1,0,1,0,0,1],
  *            "baseSpeed":600,
  *            "leftSpeed":450,
  *            "rightSpeed":550,
  *            "error":10,
  *            "integral":5,
  *            "advance":2,
  *            "trackEnable":1,
  *            "speedMode":1
  *          }
  */
int build_sensor_json(char *buf, int size) {
    static uint32_t tick = 0;
    tick++;

    /* 读取8个传感器值 */
    int sensors[8];
    sensors[0] = GetSen0Val();
    sensors[1] = GetSen1Val();
    sensors[2] = GetSen2Val();
    sensors[3] = GetSen3Val();
    sensors[4] = GetSen4Val();
    sensors[5] = GetSen5Val();
    sensors[6] = GetSen6Val();
    sensors[7] = GetSen7Val();

    int len = snprintf(buf, size,
        "{\"type\":\"DATA\",\"tick\":%lu,\"sensors\":[%d,%d,%d,%d,%d,%d,%d,%d],"
        "\"baseSpeed\":%d,\"leftSpeed\":%d,\"rightSpeed\":%d,\"error\":%d,"
        "\"integral\":%d,\"advance\":%d,\"trackEnable\":%d,\"speedMode\":%d}",
        tick,
        sensors[0], sensors[1], sensors[2], sensors[3],
        sensors[4], sensors[5], sensors[6], sensors[7],
        baseSpeed, leftSpeed, rightSpeed, lastError,
        integral, advance_compensation, g_track_enable, g_speed_mode
    );

    return (len < size) ? len : size - 1;
}

/**
  * @brief  构建状态响应JSON字符串
  * @param  buf: 输出缓冲区
  * @param  size: 缓冲区大小（至少128字节）
  * @param  cmd: 命令类型字符串（如"STATUS","HELP"等）
  * @retval 实际写入的字符数（不含null终止符）
  * @details 格式：
  *          {
  *            "type":"RESPONSE",
  *            "cmd":"STATUS",
  *            "status":"OK",
  *            "data":{...}
  *          }
  */
int build_status_json(char *buf, int size, const char *cmd, const char *status, const char *data) {
    int len = snprintf(buf, size,
        "{\"type\":\"RESPONSE\",\"cmd\":\"%s\",\"status\":\"%s\",\"data\":%s}",
        cmd, status, data ? data : "{}"
    );
    return (len < size) ? len : size - 1;
}

/**
  * @brief  蓝牙数据传输任务函数
  * @param  无
  * @retval 无
  * @details 每50ms执行一次（可配置），当使能时通过UART3发送传感器数据JSON
  *          传输间隔通过g_bluetooth_data_interval配置（单位：毫秒）
  *          实际发送周期 = max(50ms, g_bluetooth_data_interval)
  */
void BluetoothDataFunc(void) {
    static uint16_t timer_count = 0;
    static uint16_t required_cycles = 1;  // 需要多少个50ms周期

    /* 检查传输使能标志 */
    if (!g_bluetooth_data_enable) {
        return;
    }

    /* 计算需要的周期数（至少1个周期） */
    required_cycles = g_bluetooth_data_interval / 50;
    if (required_cycles < 1) required_cycles = 1;

    /* 累计周期计数 */
    timer_count++;
    if (timer_count < required_cycles) {
        return;
    }
    timer_count = 0;

    /* 构建并发送JSON数据 */
    char json_buf[256];
    int len = build_sensor_json(json_buf, sizeof(json_buf));
    if (len > 0) {
        /* 添加换行符便于PC端解析 */
        json_buf[len] = '\r';
        json_buf[len+1] = '\n';
        json_buf[len+2] = 0;

        /* 通过UART3发送（使用FIFO缓冲） */
        uart_fifo_puts(&g_uart3, (uint8_t *)json_buf, len+2);
    }
}
