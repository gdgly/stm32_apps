

/*================================================================
 *
 *
 *   文件名称：net_protocol_ws.c
 *   创 建 者：肖飞
 *   创建日期：2020年02月23日 星期日 12时23分31秒
 *   修改日期：2020年03月20日 星期五 11时36分11秒
 *   描    述：
 *
 *================================================================*/
#include "app_platform.h"
#include "cmsis_os.h"
#include "lwip.h"
#include "lwip/sockets.h"
#include "lwip/dhcp.h"
#include "main.h"

#include <string.h>
#include "os_utils.h"
#include "net_client.h"
#include "net_protocol.h"
#include "https.h"
#include "mbedtls/platform.h"
#define UDP_LOG
#include "task_probe_tool.h"

//static HTTP_INFO hi_instance;
static BOOL verify_cert = FALSE;
//static HTTP_INFO *hi = &hi_instance;
static HTTP_INFO *hi = NULL;

static void *os_calloc(size_t nmemb, size_t size)
{
	void *ptr = os_alloc(nmemb * size);

	if(ptr == NULL) {
		while(1);
	}

	memset(ptr, 0, nmemb * size);

	return ptr;
}

static uint8_t enable_connect = 0;

void set_connect_enable(uint8_t enable)
{
	enable_connect = enable;
}

uint8_t get_connect_enable(void)
{
	return enable_connect;
}

#include "test_https.h"
static int ws_client_connect(void *ctx)
{
	int ret = -1;
	net_client_info_t *net_client_info = (net_client_info_t *)ctx;
	//char *url = "https://httpbin.org/get";
	//char *url = "ws://192.168.41.2:8080/ocpp/";
	char *url = "ws://47.244.218.210:8080/OCPP/echoSocket/13623";

	if(get_connect_enable() != 1) {
		return ret;
	}

	//test_https();

	if(hi == NULL) {
		hi = (HTTP_INFO *)os_calloc(1, sizeof(HTTP_INFO));

		if(hi == NULL) {
			return ret;
		}
	}

	http_init(hi, verify_cert);

	mbedtls_platform_set_calloc_free(os_calloc, os_free);

	ret = http_open(hi, url);

	if(ret != 0) {
		return ret;
	}

	snprintf((char *)hi->request.method, 8, "GET");
	snprintf((char *)hi->request.content_type, H_FIELD_SIZE, "application/json; charset=utf-8");

	//ret = http_write_header(hi);
	ret = http_write_ws_header(hi);

	if(ret != 0) {
		return ret;
	}

	net_client_info->sock_fd = hi->tls.ssl_fd.fd;

	if(net_client_info->sock_fd == -1) {
		return ret;
	}

	ret = net_client_info->sock_fd;

	return ret;
}

static int ws_client_recv(void *ctx, void *buf, size_t len)
{
	net_client_info_t *net_client_info = (net_client_info_t *)ctx;

	net_client_info = net_client_info;
	return https_read(hi, (char *)buf, len);
}

static int ws_client_send(void *ctx, const void *buf, size_t len)
{
	net_client_info_t *net_client_info = (net_client_info_t *)ctx;

	net_client_info = net_client_info;
	return https_write(hi, (char *)buf, len);
}

static int ws_client_close(void *ctx)
{
	int ret = -1;
	net_client_info_t *net_client_info = (net_client_info_t *)ctx;

	if(hi != NULL) {
		ret = http_close(hi);
		os_free(hi);
		hi = NULL;
	}
	net_client_info->sock_fd = -1;
	set_connect_enable(0);

	return ret;
}

protocol_if_t protocol_if_ws = {
	.net_connect = ws_client_connect,
	.net_recv = ws_client_recv,
	.net_send = ws_client_send,
	.net_close = ws_client_close,
};