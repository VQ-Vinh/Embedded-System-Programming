/**
 * @file    task_uart.c
 * @brief   UartTask: nhận ResultRecord_t và gửi một dòng CSV qua USART2.
 *
 * Người phụ trách: Người 3.
 *
 * Định dạng (115200 8N1), mỗi lượt một dòng:
 *   ID,Weight_g,Target_g,Tolerance_g,Result
 *   1,250.2,250,5,PASS
 * Dòng bắt đầu bằng '#' là chú thích (khởi động, xóa thống kê, đổi cấu hình);
 * script tools/serial_logger.py bỏ qua các dòng này khi ghi file CSV.
 */
#include "app_main.h"

#include <string.h>
#include "main.h"
#include "app_types.h"
#include "fmt.h"
#include "weigh_fsm.h"

extern UART_HandleTypeDef huart2;

/** @brief Gửi chuỗi qua USART2 ở chế độ chặn (chỉ UartTask gọi). */
static void uart_send(const char *s)
{
    (void)HAL_UART_Transmit(&huart2, (const uint8_t *)s, (uint16_t)strlen(s), 200U);
}

/** @brief Ghi giá trị x10, bỏ ".0" nếu là số nguyên (500.0 -> "500"). */
static void cat_value(char *dst, size_t cap, int32_t v_x10)
{
    if ((v_x10 % 10) == 0)
    {
        (void)fmt_cat_i32(dst, cap, v_x10 / 10);
    }
    else
    {
        (void)fmt_cat_x10(dst, cap, v_x10);
    }
}

/** @brief Thân UartTask: in tiêu đề CSV rồi gửi mỗi bản ghi thành một dòng. Không bao giờ trả về. */
void UartTask_Run(void *argument)
{
    ResultRecord_t rec;
    char line[64];

    (void)argument;
    uart_send("\r\n# CAN KIEM TRA KHOI LUONG - STM32F4 + FreeRTOS\r\n");
    uart_send("ID,Weight_g,Target_g,Tolerance_g,Result\r\n");

    for (;;)
    {
        if (osMessageQueueGet(qUartHandle, &rec, NULL, osWaitForever) != osOK)
        {
            continue;
        }
        line[0] = '\0';
        switch (rec.kind)
        {
        case REC_RESULT:
            (void)fmt_cat_i32(line, sizeof(line), (int32_t)rec.id);
            (void)fmt_cat(line, sizeof(line), ",");
            (void)fmt_cat_x10(line, sizeof(line), rec.weight_x10);
            (void)fmt_cat(line, sizeof(line), ",");
            cat_value(line, sizeof(line), rec.target_x10);
            (void)fmt_cat(line, sizeof(line), ",");
            cat_value(line, sizeof(line), rec.tol_x10);
            (void)fmt_cat(line, sizeof(line), ",");
            (void)fmt_cat(line, sizeof(line), weigh_result_csv((weigh_result_t)rec.result));
            break;
        case REC_STATS_CLEARED:
            (void)fmt_cat(line, sizeof(line), "# STATS CLEARED");
            break;
        default:
            (void)fmt_cat(line, sizeof(line), "# CONFIG Target_g=");
            cat_value(line, sizeof(line), rec.target_x10);
            (void)fmt_cat(line, sizeof(line), " Tolerance_g=");
            cat_value(line, sizeof(line), rec.tol_x10);
            break;
        }
        (void)fmt_cat(line, sizeof(line), "\r\n");
        uart_send(line);
    }
}
