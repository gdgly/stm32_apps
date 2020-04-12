

/*================================================================
 *   
 *   
 *   文件名称：bitmap_ops.h
 *   创 建 者：肖飞
 *   创建日期：2020年01月19日 星期日 13时16分54秒
 *   修改日期：2020年04月12日 星期日 13时34分37秒
 *   描    述：
 *
 *================================================================*/
#ifndef _BITMAP_OPS_H
#define _BITMAP_OPS_H
#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include <stdint.h>
#include "cmsis_os.h"

#ifdef __cplusplus
}
#endif

typedef uint32_t cell_type_t;
#define BIT_PER_CELL 32
#define BYTES_PER_CELL sizeof(cell_type_t)

typedef struct {
	int size;
	cell_type_t *data;
	osMutexId bitmap_mutex;
} bitmap_t;

bitmap_t *alloc_bitmap(int size);
void free_bitmap(bitmap_t *bitmap);
int get_first_value_index(bitmap_t *bitmap, uint8_t value);
int set_bitmap_value(bitmap_t *bitmap, int index, uint8_t value);

#endif //_BITMAP_OPS_H
