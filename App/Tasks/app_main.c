/**
 * @file    app_main.c
 * @brief   Tạo Queue và 5 task của ứng dụng cân kiểm tra.
 *
 * | Task        | Ưu tiên       | Stack (word) | Chu kỳ                         |
 * |-------------|---------------|--------------|--------------------------------|
 * | SensorTask  | AboveNormal   | 256          | kiểm tra HX711 mỗi 5 ms        |
 * | KeypadTask  | Normal        | 256          | quét mỗi 10 ms                 |
 * | MainTask    | Normal        | 512          | chờ sự kiện, tối đa 20 ms      |
 * | DisplayTask | BelowNormal   | 512          | vẽ lại tối đa mỗi 200 ms       |
 * | UartTask    | Low           | 384          | chờ bản ghi                    |
 *
 * Mỗi ngoại vi chỉ thuộc về một task (HX711 + cảm biến quang: SensorTask;
 * keypad: KeypadTask; LED/còi + flash: MainTask; SPI/TFT: DisplayTask;
 * USART2: UartTask) nên không cần mutex.
 */
#include "app_main.h"

#include "main.h"
#include "app_types.h"

osMessageQueueId_t qMeasHandle;
osMessageQueueId_t qKeyHandle;
osMessageQueueId_t qDisplayHandle;
osMessageQueueId_t qUartHandle;

static const osMessageQueueAttr_t qMeas_attr    = { .name = "qMeas" };
static const osMessageQueueAttr_t qKey_attr     = { .name = "qKey" };
static const osMessageQueueAttr_t qDisplay_attr = { .name = "qDisplay" };
static const osMessageQueueAttr_t qUart_attr    = { .name = "qUart" };

static const osThreadAttr_t sensor_attr = {
    .name = "SensorTask", .stack_size = 256U * 4U, .priority = osPriorityAboveNormal };
static const osThreadAttr_t keypad_attr = {
    .name = "KeypadTask", .stack_size = 256U * 4U, .priority = osPriorityNormal };
static const osThreadAttr_t main_attr = {
    .name = "MainTask", .stack_size = 512U * 4U, .priority = osPriorityNormal };
static const osThreadAttr_t display_attr = {
    .name = "DisplayTask", .stack_size = 512U * 4U, .priority = osPriorityBelowNormal };
static const osThreadAttr_t uart_attr = {
    .name = "UartTask", .stack_size = 384U * 4U, .priority = osPriorityLow };

/** @brief Xem mô tả trong app_main.h. */
void App_Init(void)
{
    qMeasHandle    = osMessageQueueNew(4U, sizeof(Measurement_t), &qMeas_attr);
    qKeyHandle     = osMessageQueueNew(8U, sizeof(KeyEvent_t), &qKey_attr);
    // ! qDisplay phải dài đúng 1 phần tử vì MainTask dùng xQueueOverwrite().
    qDisplayHandle = osMessageQueueNew(1U, sizeof(DisplaySnapshot_t), &qDisplay_attr);
    qUartHandle    = osMessageQueueNew(8U, sizeof(ResultRecord_t), &qUart_attr);

    if ((qMeasHandle == NULL) || (qKeyHandle == NULL) || (qDisplayHandle == NULL) || (qUartHandle == NULL))
    {
        Error_Handler();
    }

    if ((osThreadNew(SensorTask_Run, NULL, &sensor_attr) == NULL) ||
        (osThreadNew(KeypadTask_Run, NULL, &keypad_attr) == NULL) ||
        (osThreadNew(MainTask_Run, NULL, &main_attr) == NULL) ||
        (osThreadNew(DisplayTask_Run, NULL, &display_attr) == NULL) ||
        (osThreadNew(UartTask_Run, NULL, &uart_attr) == NULL))
    {
        // ! Thiếu heap FreeRTOS (configTOTAL_HEAP_SIZE) nếu rơi vào đây.
        Error_Handler();
    }
}
