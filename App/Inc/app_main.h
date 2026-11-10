/**
 * @file    app_main.h
 * @brief   Điểm vào của ứng dụng: tạo Queue và các task FreeRTOS.
 *
 * App_Init() được gọi trong main.c (vùng USER CODE RTOS_THREADS) sau
 * osKernelInitialize() và trước osKernelStart(). Tạo task trong code thay vì
 * trong CubeMX giúp cấu hình task không bị mất khi generate lại.
 */
#ifndef APP_MAIN_H
#define APP_MAIN_H

#include "cmsis_os.h"

/** Handle các queue dùng chung, định nghĩa trong app_main.c. */
extern osMessageQueueId_t qMeasHandle;
extern osMessageQueueId_t qKeyHandle;
extern osMessageQueueId_t qDisplayHandle;
extern osMessageQueueId_t qUartHandle;

/**
 * @brief  Tạo 4 queue và 5 task của ứng dụng.
 * @note   Gọi đúng một lần trước osKernelStart(). Nếu tạo thất bại (thiếu
 *         heap FreeRTOS) sẽ gọi Error_Handler().
 */
void App_Init(void);

/* Hàm thân của từng task (mỗi task một file trong App/Tasks). */
void SensorTask_Run(void *argument);
void KeypadTask_Run(void *argument);
void MainTask_Run(void *argument);
void DisplayTask_Run(void *argument);
void UartTask_Run(void *argument);

#endif /* APP_MAIN_H */
