

/*================================================================
 *
 *
 *   文件名称：test_https.c
 *   创 建 者：肖飞
 *   创建日期：2020年01月27日 星期一 12时22分23秒
 *   修改日期：2020年05月14日 星期四 13时09分27秒
 *   描    述：
 *
 *================================================================*/
#include "test_https.h"
#include "https.h"
#define UDP_LOG
#include "task_probe_tool.h"
#include "mbedtls/platform.h"
#include <string.h>
#include <stdio.h>

#define _printf udp_log_printf

static HTTP_INFO hi;
static BOOL verify_cert = FALSE;

static void *os_calloc(size_t nmemb, size_t size)
{
	void *ptr = os_alloc(nmemb * size);
	if(ptr == NULL) {
		while(1);
	}

	memset(ptr, 0, nmemb * size);

	return ptr;
}

static char response[1024] = {0};

int test_https(void)
{
	char *url = "https://httpbin.org/get";
	int ret = 0;

	http_init(&hi, verify_cert);

	mbedtls_platform_set_calloc_free(os_calloc, os_free);

	ret = http_open(&hi, url);

	if(ret != 0) {
		return ret;
	}

	snprintf((char *)&hi.request.method, 8, "GET");
	snprintf((char *)&hi.request.content_type, H_FIELD_SIZE, "application/json; charset=utf-8");

	ret = http_write_header(&hi);
	if(ret != 0) {
		return ret;
	}

	ret = http_read_chunked(&hi, response, 1024);

	_printf("return code: %d \n", ret);
	_printf("return body: %s \n", response);

	http_close(&hi);

	return ret;
}
