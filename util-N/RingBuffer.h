#pragma once
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define  RING_BUFFER_MAX_SIZE       40960

typedef struct ringbuffer {
    int length;
    size_t start;
    size_t end;
/**2020-11-23,meituan not allocate memory in heap,
 * and calling RTK lib in single task of RTOS
*/
#ifdef _RING_BUFFER_STATIC
    char buffer[RING_BUFFER_MAX_SIZE+1];
#else
    char *buffer;
#endif
} ringbuffer;

//static,not need call to rb_create allocate memory ,but call rb_init
/**2020-11-23,meituan not allocate memory in heap*/
#ifdef _RING_BUFFER_STATIC
void rb_init(struct ringbuffer *rb);
#else
struct ringbuffer *rb_create(int len);
void rb_destroy(struct ringbuffer *rb);
#endif

int rb_write(struct ringbuffer *rb, const char *buf, size_t len);
int rb_read(struct ringbuffer *rb, char *buf, size_t len);
void *rb_dump(struct ringbuffer *rb, size_t *len);
void rb_cleanup(struct ringbuffer *rb);
int rb_get_space_free(struct ringbuffer *rb);
int rb_get_space_used(struct ringbuffer *rb);



#ifdef __cplusplus
}
#endif
