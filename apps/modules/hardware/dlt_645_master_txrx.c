

/*================================================================
 *
 *
 *   文件名称：dlt_645_master_txrx.c
 *   创 建 者：肖飞
 *   创建日期：2020年05月21日 星期四 10时19分55秒
 *   修改日期：2021年05月25日 星期二 11时26分56秒
 *   描    述：
 *
 *================================================================*/
#include "dlt_645_master_txrx.h"
#include <string.h>
#include "object_class.h"

#define LOG_DISABLE
#include "log.h"

static object_class_t *dlt_645_master_class = NULL;

static void free_dlt_645_master_info(dlt_645_master_info_t *dlt_645_master_info)
{
	if(dlt_645_master_info == NULL) {
		return;
	}

	if(dlt_645_master_info->uart_info) {
		dlt_645_master_info->uart_info = NULL;
		dlt_645_master_info->rx_size = 0;
		dlt_645_master_info->tx_size = 0;
	}

	os_free(dlt_645_master_info);
}

static dlt_645_master_info_t *alloc_dlt_645_master_info(uart_info_t *uart_info)
{
	dlt_645_master_info_t *dlt_645_master_info = NULL;

	if(uart_info == NULL) {
		return dlt_645_master_info;
	}

	dlt_645_master_info = (dlt_645_master_info_t *)os_alloc(sizeof(dlt_645_master_info_t));

	if(dlt_645_master_info == NULL) {
		return dlt_645_master_info;
	}

	memset(dlt_645_master_info, 0, sizeof(dlt_645_master_info_t));

	dlt_645_master_info->uart_info = uart_info;
	dlt_645_master_info->rx_timeout = 100;
	dlt_645_master_info->tx_timeout = 100;

	return dlt_645_master_info;
}

static int object_filter(void *o, void *ctx)
{
	int ret = -1;
	dlt_645_master_info_t *dlt_645_master_info = (dlt_645_master_info_t *)o;
	uart_info_t *uart_info = (uart_info_t *)ctx;

	if(dlt_645_master_info->uart_info == uart_info) {
		ret = 0;
	}

	return ret;
}

dlt_645_master_info_t *get_or_alloc_dlt_645_master_info(uart_info_t *uart_info)
{
	dlt_645_master_info_t *dlt_645_master_info = NULL;

	os_enter_critical();

	if(dlt_645_master_class == NULL) {
		dlt_645_master_class = object_class_alloc();
	}

	os_leave_critical();

	dlt_645_master_info = (dlt_645_master_info_t *)object_class_get_or_alloc_object(dlt_645_master_class, object_filter, uart_info, (object_alloc_t)alloc_dlt_645_master_info, (object_free_t)free_dlt_645_master_info);

	return dlt_645_master_info;
}

typedef struct {
	uint8_t flag;
	uint8_t chennel;//0:总 0x01-0x3f:费率1-63 0xff:最大
	uint8_t type;
	uint8_t domain;
} data_type_t;

typedef struct {
	data_type_t data_type;
} request_energy_data_t;

typedef struct {
	dlt_645_frame_head_t head;
	request_energy_data_t data;
	uint8_t crc;
	uint8_t end_flag;//0x16
} request_energy_t;

typedef struct {
	uint8_t b0;
	uint8_t b1;
	uint8_t b2;
	uint8_t b3;
} energy_data_block_t;

typedef struct {
	data_type_t data_type;
	energy_data_block_t data_block;
} response_energy_data_t;

typedef struct {
	dlt_645_frame_head_t head;
	response_energy_data_t data;
	uint8_t crc;
	uint8_t end_flag;//0x16
} response_energy_t;

int dlt_645_master_get_energy(dlt_645_master_info_t *dlt_645_master_info, dlt_645_addr_t *addr, uint32_t *energy)/*100*/
{
	int ret = -1;
	int rx_size;
	int i;
	uint8_t *data;
	uint8_t crc;
	uint32_t data_type = 0x00010000;
	u_dlt_645_control_t u_dlt_645_control;

	request_energy_t *request_energy = (request_energy_t *)dlt_645_master_info->tx_buffer;
	response_energy_t *response_energy = (response_energy_t *)dlt_645_master_info->rx_buffer;

	request_energy->head.addr_start_flag = 0x68;
	memcpy(&request_energy->head.addr, addr, sizeof(dlt_645_addr_t));
	request_energy->head.content_start_flag = 0x68;
	u_dlt_645_control.v = 0;
	u_dlt_645_control.s.fn = 0x11;
	request_energy->head.control.v = u_dlt_645_control.v;
	request_energy->head.len = sizeof(request_energy_data_t);

	request_energy->data.data_type.domain = get_u8_b3_from_u32(data_type);
	request_energy->data.data_type.type = get_u8_b2_from_u32(data_type);;
	request_energy->data.data_type.chennel = get_u8_b1_from_u32(data_type);;
	request_energy->data.data_type.flag = get_u8_b0_from_u32(data_type);;

	data = (uint8_t *)&request_energy->data;

	for(i = 0; i < sizeof(request_energy_data_t); i++) {
		data[i] = data[i] + 0x33;
	}

	request_energy->crc = sum_crc8((uint8_t *)request_energy,
	                                  (uint8_t *)&request_energy->crc - (uint8_t *)request_energy);
	request_energy->end_flag = 0x16;

	dlt_645_master_info->tx_size = sizeof(request_energy_data_t);
	dlt_645_master_info->rx_size = sizeof(response_energy_data_t);

	rx_size = uart_tx_rx_data(dlt_645_master_info->uart_info,
	                          (uint8_t *)request_energy, dlt_645_master_info->tx_size,
	                          (uint8_t *)response_energy, dlt_645_master_info->rx_size,
	                          dlt_645_master_info->rx_timeout);

	if(rx_size != dlt_645_master_info->rx_size) {
		debug("rx_size:%d", rx_size);
		return ret;
	}

	crc = sum_crc8((uint8_t *)response_energy,
	                  (uint8_t *)&response_energy->crc - (uint8_t *)response_energy);

	if(response_energy->crc != crc) {
		return ret;
	}

	u_dlt_645_control.s.frame_type = 1;

	if(response_energy->head.control.v != u_dlt_645_control.v) {
		return ret;
	}

	for(i = 0; i < sizeof(response_energy_data_t); i++) {
		data[i] = data[i] - 0x33;
	}

	data_type = get_u32_from_u8_b0123(
	                request_energy->data.data_type.flag,
	                request_energy->data.data_type.chennel,
	                request_energy->data.data_type.type,
	                request_energy->data.data_type.domain);

	if(data_type != get_u32_from_u8_b0123(response_energy->data.data_type.flag,
	                                      response_energy->data.data_type.chennel,
	                                      response_energy->data.data_type.type,
	                                      response_energy->data.data_type.domain)) {
		return ret;
	}

	*energy = get_u32_from_bcd_b0123(
	              response_energy->data.data_block.b0,
	              response_energy->data.data_block.b1,
	              response_energy->data.data_block.b2,
	              response_energy->data.data_block.b3);
	ret = 0;

	return ret;
}

typedef struct {
	data_type_t data_type;
} request_voltage_data_t;

typedef struct {
	dlt_645_frame_head_t head;
	request_voltage_data_t data;
	uint8_t crc;
	uint8_t end_flag;//0x16
} request_voltage_t;

typedef struct {
	uint8_t va_b0;
	uint8_t va_b1;
	uint8_t vb_b0;
	uint8_t vb_b1;
	uint8_t vc_b0;
	uint8_t vc_b1;
} voltage_data_block_t;

typedef struct {
	data_type_t data_type;
	voltage_data_block_t data_block;
} response_voltage_data_t;

typedef struct {
	dlt_645_frame_head_t head;
	response_voltage_data_t data;
	uint8_t crc;
	uint8_t end_flag;//0x16
} response_voltage_t;

int dlt_645_master_get_voltage(dlt_645_master_info_t *dlt_645_master_info, dlt_645_addr_t *addr, uint16_t *va, uint16_t *vb, uint16_t *vc)
{
	int ret = -1;
	int rx_size;
	int i;
	uint8_t *data;
	uint8_t crc;
	uint32_t data_type = 0x0201ff00;
	u_dlt_645_control_t u_dlt_645_control;

	request_voltage_t *request_voltage = (request_voltage_t *)dlt_645_master_info->tx_buffer;
	response_voltage_t *response_voltage = (response_voltage_t *)dlt_645_master_info->rx_buffer;

	request_voltage->head.addr_start_flag = 0x68;
	memcpy(&request_voltage->head.addr, addr, sizeof(dlt_645_addr_t));
	request_voltage->head.content_start_flag = 0x68;
	u_dlt_645_control.v = 0;
	u_dlt_645_control.s.fn = 0x11;
	request_voltage->head.control.v = u_dlt_645_control.v;
	request_voltage->head.len = sizeof(request_voltage_data_t);

	request_voltage->data.data_type.domain = get_u8_b3_from_u32(data_type);
	request_voltage->data.data_type.type = get_u8_b2_from_u32(data_type);;
	request_voltage->data.data_type.chennel = get_u8_b1_from_u32(data_type);;
	request_voltage->data.data_type.flag = get_u8_b0_from_u32(data_type);;

	data = (uint8_t *)&request_voltage->data;

	for(i = 0; i < sizeof(request_voltage_data_t); i++) {
		data[i] = data[i] + 0x33;
	}

	request_voltage->crc = sum_crc8((uint8_t *)request_voltage,
	                                   (uint8_t *)&request_voltage->crc - (uint8_t *)request_voltage);
	request_voltage->end_flag = 0x16;

	dlt_645_master_info->tx_size = sizeof(request_voltage_data_t);
	dlt_645_master_info->rx_size = sizeof(response_voltage_data_t);

	rx_size = uart_tx_rx_data(dlt_645_master_info->uart_info,
	                          (uint8_t *)request_voltage, dlt_645_master_info->tx_size,
	                          (uint8_t *)response_voltage, dlt_645_master_info->rx_size,
	                          dlt_645_master_info->rx_timeout);

	if(rx_size != dlt_645_master_info->rx_size) {
		debug("rx_size:%d", rx_size);
		return ret;
	}

	crc = sum_crc8((uint8_t *)response_voltage,
	                  (uint8_t *)&response_voltage->crc - (uint8_t *)response_voltage);

	if(response_voltage->crc != crc) {
		return ret;
	}

	u_dlt_645_control.s.frame_type = 1;

	if(response_voltage->head.control.v != u_dlt_645_control.v) {
		return ret;
	}

	for(i = 0; i < sizeof(response_voltage_data_t); i++) {
		data[i] = data[i] - 0x33;
	}

	data_type = get_u32_from_u8_b0123(
	                response_voltage->data.data_type.flag,
	                response_voltage->data.data_type.chennel,
	                response_voltage->data.data_type.type,
	                response_voltage->data.data_type.domain);

	if(data_type != 0x00010000) {
		return ret;
	}

	*va = get_u16_from_bcd_b01(
	          response_voltage->data.data_block.va_b0,
	          response_voltage->data.data_block.va_b1);

	*vb = get_u16_from_bcd_b01(
	          response_voltage->data.data_block.vb_b0,
	          response_voltage->data.data_block.vb_b1);

	*vc = get_u16_from_bcd_b01(
	          response_voltage->data.data_block.vc_b0,
	          response_voltage->data.data_block.vc_b1);
	ret = 0;

	return ret;
}

typedef struct {
	data_type_t data_type;
} request_current_data_t;

typedef struct {
	dlt_645_frame_head_t head;
	request_current_data_t data;
	uint8_t crc;
	uint8_t end_flag;//0x16
} request_current_t;

typedef struct {
	uint8_t ca_b0;
	uint8_t ca_b1;
	uint8_t ca_b2;
	uint8_t cb_b0;
	uint8_t cb_b1;
	uint8_t cb_b2;
	uint8_t cc_b0;
	uint8_t cc_b1;
	uint8_t cc_b2;
} current_data_block_t;

typedef struct {
	data_type_t data_type;
	current_data_block_t data_block;
} response_current_data_t;

typedef struct {
	dlt_645_frame_head_t head;
	response_current_data_t data;
	uint8_t crc;
	uint8_t end_flag;//0x16
} response_current_t;

int dlt_645_master_get_current(dlt_645_master_info_t *dlt_645_master_info, dlt_645_addr_t *addr, uint16_t *ca, uint16_t *cb, uint16_t *cc)
{
	int ret = -1;
	int rx_size;
	int i;
	uint8_t *data;
	uint8_t crc;
	uint32_t data_type = 0x0201ff00;
	u_dlt_645_control_t u_dlt_645_control;

	request_current_t *request_current = (request_current_t *)dlt_645_master_info->tx_buffer;
	response_current_t *response_current = (response_current_t *)dlt_645_master_info->rx_buffer;

	request_current->head.addr_start_flag = 0x68;
	memcpy(&request_current->head.addr, addr, sizeof(dlt_645_addr_t));
	request_current->head.content_start_flag = 0x68;
	u_dlt_645_control.v = 0;
	u_dlt_645_control.s.fn = 0x11;
	request_current->head.control.v = u_dlt_645_control.v;
	request_current->head.len = sizeof(request_current_data_t);

	request_current->data.data_type.domain = get_u8_b3_from_u32(data_type);
	request_current->data.data_type.type = get_u8_b2_from_u32(data_type);;
	request_current->data.data_type.chennel = get_u8_b1_from_u32(data_type);;
	request_current->data.data_type.flag = get_u8_b0_from_u32(data_type);;

	data = (uint8_t *)&request_current->data;

	for(i = 0; i < sizeof(request_current_data_t); i++) {
		data[i] = data[i] + 0x33;
	}

	request_current->crc = sum_crc8((uint8_t *)request_current,
	                                   (uint8_t *)&request_current->crc - (uint8_t *)request_current);
	request_current->end_flag = 0x16;

	dlt_645_master_info->tx_size = sizeof(request_current_data_t);
	dlt_645_master_info->rx_size = sizeof(response_current_data_t);

	rx_size = uart_tx_rx_data(dlt_645_master_info->uart_info,
	                          (uint8_t *)request_current, dlt_645_master_info->tx_size,
	                          (uint8_t *)response_current, dlt_645_master_info->rx_size,
	                          dlt_645_master_info->rx_timeout);

	if(rx_size != dlt_645_master_info->rx_size) {
		debug("rx_size:%d", rx_size);
		return ret;
	}

	crc = sum_crc8((uint8_t *)response_current,
	                  (uint8_t *)&response_current->crc - (uint8_t *)response_current);

	if(response_current->crc != crc) {
		return ret;
	}

	u_dlt_645_control.s.frame_type = 1;

	if(response_current->head.control.v != u_dlt_645_control.v) {
		return ret;
	}

	for(i = 0; i < sizeof(response_current_data_t); i++) {
		data[i] = data[i] - 0x33;
	}

	data_type = get_u32_from_u8_b0123(
	                response_current->data.data_type.flag,
	                response_current->data.data_type.chennel,
	                response_current->data.data_type.type,
	                response_current->data.data_type.domain);

	if(data_type != 0x00010000) {
		return ret;
	}

	*ca = get_u32_from_bcd_b0123(
	          response_current->data.data_block.ca_b0,
	          response_current->data.data_block.ca_b1,
	          response_current->data.data_block.ca_b2,
	          0);

	*cb = get_u32_from_bcd_b0123(
	          response_current->data.data_block.cb_b0,
	          response_current->data.data_block.cb_b1,
	          response_current->data.data_block.cb_b2,
	          0);

	*cc = get_u32_from_bcd_b0123(
	          response_current->data.data_block.cc_b0,
	          response_current->data.data_block.cc_b1,
	          response_current->data.data_block.cc_b2,
	          0);
	ret = 0;

	return ret;
}
