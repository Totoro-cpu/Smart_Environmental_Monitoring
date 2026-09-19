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
#include "cmsis_os.h"
#include "main.h"
#include "task.h"


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ap3216.h"
#include "my_lcd.h"
#include "myiic.h"
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
/* 光传感器数据结构 */
typedef struct {
  uint16_t ir;
  uint16_t als;
  uint16_t ps;
} ap3216_data_t;

/* 消息队列句柄 */
osMessageQueueId_t sensorQueueHandle;
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
    .name = "defaultTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void StartLedTask(void *argument);
void StartLcdTask(void *argument);
void StartAP3216SensorTask(void *argument);
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
  sensorQueueHandle = osMessageQueueNew(4, sizeof(ap3216_data_t), NULL);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle =
      osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* 创建 LED 任务 */
  osThreadNew(StartLedTask, NULL,
              &(osThreadAttr_t){.name = "LED_Task",
                                .stack_size = 128 * 4,
                                .priority = osPriorityNormal});
  /* 创建 LCD 任务 */
  osThreadNew(StartLcdTask, NULL,
              &(osThreadAttr_t){.name = "LCD_Task",
                                .stack_size = 512 * 4,
                                .priority = osPriorityNormal});
  /* 创建 AP3216 传感器任务 */
  osThreadNew(StartAP3216SensorTask, NULL,
              &(osThreadAttr_t){.name = "Sensor_Task",
                                .stack_size = 256 * 4,
                                .priority = osPriorityNormal});
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
void StartDefaultTask(void *argument) {
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
void StartLcdTask(void *argument) {
  ap3216_data_t data;
  char buf[32];

  /* 先清屏 */
  lcd_fill_area(0, 0, 239, 319, 0x0000); /* 黑色背景 */

  for (;;) {
    /* 从队列接收数据，最多等 100ms */
    if (osMessageQueueGet(sensorQueueHandle, &data, NULL, 100) == osOK) {
      /* IR：无效值显示 -- */
      if (data.ir == 0)
        sprintf(buf, "IR : 0    ");
      else
        sprintf(buf, "IR : %-5u", data.ir);
      lcd_show_string(20, 60, buf, 0xFFFF, 0x0000);

      /* ALS：始终有效 */
      sprintf(buf, "ALS: %-5u", data.als);
      lcd_show_string(20, 100, buf, 0x07E0, 0x0000);

      /* PS：无效值显示 -- */
      if (data.ps == 0)
        sprintf(buf, "PS : 0    ");
      else
        sprintf(buf, "PS : %-5u", data.ps);
      lcd_show_string(20, 140, buf, 0x001F, 0x0000);
    }
  }
}

void StartAP3216SensorTask(void *argument) {
  ap3216_data_t data;
  uint8_t buf[6];

  if (ap3216_init() != 0) {
    printf("AP3216 init failed\r\n");
  } else {
    printf("AP3216 init OK\r\n");
  }

  for (;;) {
    /* 逐个读 6 个寄存器，每次 1 字节 */
    iic_read_reg(0x1E, 0x0A, &buf[0], 1); /* IR 低 */
    iic_read_reg(0x1E, 0x0B, &buf[1], 1); /* IR 高 */
    iic_read_reg(0x1E, 0x0C, &buf[2], 1); /* ALS 低 */
    iic_read_reg(0x1E, 0x0D, &buf[3], 1); /* ALS 高 */
    iic_read_reg(0x1E, 0x0E, &buf[4], 1); /* PS 低 */
    iic_read_reg(0x1E, 0x0F, &buf[5], 1); /* PS 高 */

    // printf("Raw: %02X %02X %02X %02X %02X %02X\r\n",
    //        buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

    /* 拼装数据：IR 和 PS 都是 10 位，bit7/bit6 是无效标志 */
    data.ir = (buf[0] & 0x80) ? 0 : ((uint16_t)(buf[1] << 2) | (buf[0] & 0x03));
    data.als = (uint16_t)(buf[3] << 8) | buf[2];
    data.ps = (buf[4] & 0x40) ? 0 : ((uint16_t)(buf[5] << 2) | (buf[4] & 0x0F));

    // printf("IR:%u ALS:%u PS:%u\r\n", data.ir, data.als, data.ps);
    osMessageQueuePut(sensorQueueHandle, &data, 0, 10);
    osDelay(1000);
  }
}
/* USER CODE END Application */
