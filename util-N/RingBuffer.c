#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "RingBuffer.h"

#define MIN(a, b)           ((a) > (b) ? (b) : (a))
#define MAX(a, b)           ((a) > (b) ? (a) : (b))
#define CALLOC(size, type)  (type *)calloc(size, sizeof(type))

//size_t rb_get_space_free(struct ringbuffer *rb)
int rb_get_space_free(struct ringbuffer *rb)
{
    if (!rb) {
        return -1;
    }
    if (rb->end >= rb->start) {
        return rb->length - (rb->end - rb->start)-1;
    } else {
        return rb->start - rb->end-1;
    }
}

//size_t rb_get_space_used(struct ringbuffer *rb)
int rb_get_space_used(struct ringbuffer *rb)
{
    if (!rb) {
        return -1;
    }
    if (rb->end >= rb->start) {
        return rb->end - rb->start;
    } else {
        return rb->length - (rb->start - rb->end);
    }
}

/**2020-11-23,meituan not allocate memory in heap, now allocate staticlly*/
#ifdef _RING_BUFFER_STATIC
void rb_init(struct ringbuffer *rb)
{
    memset(rb->buffer, 0x00, RING_BUFFER_MAX_SIZE + 1);
    rb->length = RING_BUFFER_MAX_SIZE + 1;
    rb->start = 0;
    rb->end = 0;    
}
#else
struct ringbuffer *rb_create(int len)
{
    struct ringbuffer *rb = CALLOC(1, struct ringbuffer);
    if (!rb) {
        printf("malloc ringbuffer failed!\n");
        return NULL;
    }

    rb->length = len + 1;
    rb->start = 0;
    rb->end = 0;

    rb->buffer = (char*)calloc(1, rb->length);
    if (!rb->buffer) {
        printf("malloc rb->buffer failed!\n");
        free(rb);
        return NULL;
    }

    return rb;
}

void rb_destroy(struct ringbuffer *rb)
{
    if (!rb) {
        return;
    }
    free(rb->buffer);
    free(rb);
}
#endif

void *rb_end_ptr(struct ringbuffer *rb)
{
    return (void *)((char *)rb->buffer + rb->end);
}

void *rb_start_ptr(struct ringbuffer *rb)
{
    return (void *)((char *)rb->buffer + rb->start);
}

//size_t rb_write(struct ringbuffer *rb, const char *buf, size_t len)
int rb_write(struct ringbuffer *rb, const char *buf, size_t len)
{
    if (!rb) {
        return -1;
    }
    size_t left = rb_get_space_free(rb);
    if (len > left) {
        printf("Not enough space: %zu request, %zu available\n", len, left);
        return -1;
    }

    if ((rb->length - rb->end) < len) {
        int half_tail = rb->length - rb->end;
        memcpy(rb_end_ptr(rb), buf, half_tail);
        rb->end = (rb->end + half_tail) % rb->length;

        int half_head = len - half_tail;
        memcpy(rb_end_ptr(rb), buf+half_tail, half_head);
        rb->end = (rb->end + half_head) % rb->length;
    } else {
        memcpy(rb_end_ptr(rb), buf, len);
        rb->end = (rb->end + len) % rb->length;
    }
    return len;
}

//size_t rb_read(struct ringbuffer *rb, char *buf, size_t len)
int rb_read(struct ringbuffer *rb, char *buf, size_t len)
{
    if (!rb) {
        return -1;
    }
    size_t rlen = MIN(len, rb_get_space_used(rb));

    if ((rb->length - rb->start) < rlen) {
        int half_tail = rb->length - rb->start;
        memcpy(buf, rb_start_ptr(rb), half_tail);
        rb->start = (rb->start + half_tail) % rb->length;

        int half_head = rlen - half_tail;
        memcpy(buf+half_tail, rb_start_ptr(rb), half_head);
        rb->start = (rb->start + half_head) % rb->length;
    } else {
        memcpy(buf, rb_start_ptr(rb), rlen);
        rb->start = (rb->start + rlen) % rb->length;
    }

    if ((rb->start == rb->end) || (rb_get_space_used(rb) == 0)) {
        rb->start = rb->end = 0;
    }
    return rlen;
}

void *rb_dump(struct ringbuffer *rb, size_t *blen)
{
    if (!rb) {
        return NULL;
    }
    char *buf = NULL;
    size_t len = rb_get_space_used(rb);
    if (len <= 0) {
        return NULL;
    }
    buf = (char*)calloc(1, len);
    if (!buf) {
        printf("malloc %zu failed!\n", len);
        return NULL;
    }
    *blen = len;

    if ((rb->length - rb->start) < len) {
        int half_tail = rb->length - rb->start;
        memcpy(buf, rb_start_ptr(rb), half_tail);

        int half_head = len - half_tail;
        memcpy(buf+half_tail, rb->buffer, half_head);
    } else {
        memcpy(buf, rb_start_ptr(rb), len);
    }
    return buf;
}

void rb_cleanup(struct ringbuffer *rb)
{
    if (!rb) {
        return;
    }
    rb->start = rb->end = 0;
}
