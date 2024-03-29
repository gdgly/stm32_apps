

/*================================================================
 *   
 *   
 *   文件名称：uart_data_task.h
 *   创 建 者：肖飞
 *   创建日期：2021年01月25日 星期一 12时51分35秒
 *   修改日期：2021年01月25日 星期一 13时16分21秒
 *   描    述：
 *
 *================================================================*/
#ifndef _UART_DATA_TASK_H
#define _UART_DATA_TASK_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "app_platform.h"
#include "cmsis_os.h"
#include "usart_txrx.h"
#include "callback_chain.h"

#ifdef __cplusplus
}
#endif

typedef struct {
	uart_info_t *uart_info;
	callback_chain_t *uart_data_chain;
} uart_data_task_info_t;

int add_uart_data_task_info_cb(uart_data_task_info_t *uart_data_task_info, callback_item_t *callback_item);
int remove_uart_data_task_info_cb(uart_data_task_info_t *uart_data_task_info, callback_item_t *callback_item);
uart_data_task_info_t *get_or_alloc_uart_data_task_info(UART_HandleTypeDef *huart);
#endif //_UART_DATA_TASK_H
