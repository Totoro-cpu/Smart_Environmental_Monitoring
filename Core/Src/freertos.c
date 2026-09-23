/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ap3216.h"
#include "aht20.h"
#include "my_lcd.h"
#include "myiic.h"
#include <stdint.h>
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
/* 传感器数据结构 */
typedef struct {
  uint16_t ir;
  uint16_t als;
  uint16_t ps;
  float    temperature;   /* AHT20 温度 */
  float    humidity;      /* AHT20 湿度 */
} env_data_t;

env_data_t g_env_data = {0}; 

/* 消息队列句柄 */
osMessageQueueId_t sensorQueueHandle;
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void StartLedTask(void *argument);
void StartLcdTask(void *argument);
void StartAP3216SensorTask(void *argument);
void StartAHT20SensorTask(void *argument);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  sensorQueueHandle = osMessageQueueNew(8, sizeof(uint8_t), NULL);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* 创建 LED 任务 */
//   osThreadNew(StartLedTask, NULL,
//               &(osThreadAttr_t){.name = "LED_Task",
//                                 .stack_size = 128 * 4,
//                                 .priority = osPriorityNormal});
  /* 创建 LCD 任务 */
//   osThreadNew(StartLcdTask, NULL,
//               &(osThreadAttr_t){.name = "LCD_Task",
//                                 .stack_size = 512 * 4,
//                                 .priority = osPriorityNormal});
//   /* 创建 AP3216 传感器任务 */
//   osThreadNew(StartAP3216SensorTask, NULL,
//               &(osThreadAttr_t){.name = "Sensor_Task",
//                                 .stack_size = 256 * 4,
//                                 .priority = osPriorityNormal});
  /* AHT20 任务 */
//   osThreadNew(StartAHT20SensorTask, NULL,
//               &(osThreadAttr_t){.name = "AHT20_Task",
//                                 .stack_size = 256 * 4,
//                                 .priority = osPriorityNormal});
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for (;;) {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/* LED 任务：1Hz 闪烁 */
void StartLedTask(void *argument) {
  for (;;) {
    HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
    osDelay(500);
  }
}

/* LCD 任务：1Hz 循环变色 */
void StartLcdTask(void *argument)
{
    uint16_t color = 0;
    for(;;)
    {
        lcd_fill_area(0, 0, 239, 319, color);
        color += 0x1111;
        if(color > 0xFFFF) color = 0;
        osDelay(500);
    }
}

void StartAP3216SensorTask(void *argument)
{
    uint16_t tmp1, tmp2, tmp3;
    uint8_t  notify = 1;

    if (ap3216_init() != 0) printf("AP3216 init failed\r\n");
    else                    printf("AP3216 init OK\r\n");

    for(;;)
    {
        /* 第一次读丢弃 */
        ap3216_read_data(&tmp1, &tmp2, &tmp3);
        osDelay(50);

        /* 第二次读有效，写入全局结构体 */
        if (ap3216_read_data(&g_env_data.ir, &g_env_data.als, &g_env_data.ps) == 0)
        {
            printf("AP3216: IR=%u ALS=%u PS=%u\r\n",
                   g_env_data.ir, g_env_data.als, g_env_data.ps);
            osMessageQueuePut(sensorQueueHandle, &notify, 0, 0);
        }

        osDelay(1000);
    }
}

void StartAHT20SensorTask(void *argument)
{
    float temp, humi;
    uint8_t notify = 1;

    if (aht20_init() != 0) printf("AHT20 init failed\r\n");
    else                   printf("AHT20 init OK\r\n");

    for(;;)
    {
        if (aht20_read_data(&temp, &humi) == 0)
        {
            g_env_data.temperature = temp;
            g_env_data.humidity    = humi;

            printf("AHT20: T=%.1f C  H=%.1f %%\r\n", temp, humi);

            /* 通知 LCD 任务刷新 */
            osMessageQueuePut(sensorQueueHandle, &notify, 0, 0);
        }
        else
        {
            printf("AHT20 read failed\r\n");
        }

        osDelay(1000);
    }
}
/* USER CODE END Application */

