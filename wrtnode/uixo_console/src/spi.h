/*
FileName    :SPI.h
Description :head file of SPI
Author      :SchumyHao
Email       :schumy.haojl@gmail.com
Version     :V01
Date        :2015.08.01
*/
#ifndef __SPI_H__
#define __SPI_H__

/*
    include files
*/
#include <stdbool.h>

/*
    global functions
*/
#define DEBUG
#ifdef DEBUG
extern long spi_malloc_count;
#endif
static inline void* spi_malloc(size_t size)
{
    void* ptr = NULL;
    ptr = malloc(size);
    if(NULL == ptr) {
        printf("%s: malloc error.\n", __func__);
        return NULL;
    }
#ifdef DEBUG
    spi_malloc_count++;
    printf("%s: malloc mem addr=0x%08x, len=%d malloc_count=%d.\n", __func__, (int)ptr, (int)size, (int)spi_malloc_count);
#endif
    return ptr;
}

static inline void* spi_calloc(size_t count, size_t size)
{
    void* ptr = NULL;
    ptr = calloc(count, size);
    if(NULL == ptr) {
        printf("%s: calloc error.\n", __func__);
        return NULL;
    }
#ifdef DEBUG
    spi_malloc_count++;
    printf("%s: calloc mem addr=0x%08x, len=%d malloc_count=%d.\n", __func__, (int)ptr, (int)count*size, (int)spi_malloc_count);
#endif
    return ptr;
}

static inline void spi_free(void* ptr)
{
    if(NULL != ptr) {
        free(ptr);
#ifdef DEBUG
        spi_malloc_count--;
        printf("%s: free mem addr=0x%08x, calloc_count=%d.\n", __func__, (int)ptr, (int)spi_malloc_count);
#endif
    }
    else {
        printf("%s: free error.\n", __func__);
    }
}

typedef enum {
    SPI_ERR_NODEV = 1,
    SPI_ERR_MEM,
    SPI_ERR_INPUT_NULL,
    SPI_ERR_NOT_SUPPORT,

    SPI_ERR_LAST
} spi_err_t;

typedef struct {
    unsigned int    sec;
    unsigned int    usec;
} spi_timeout_t;

/*
   basic spi port structure
*/
struct spi_base {
    //below values are internal variables. do not use them directly.
    bool             open_flag;
    char*            port;
    int              baudrate;
    int              bytesize;
    spi_timeout_t timeout;

    //about spi status.
    int          (*set_is_open)(struct spi_base* s);
    int          (*clean_is_open)(struct spi_base* s);
    int          (*is_open)(struct spi_base* s);
    //about spi port name.
    int          (*set_port)(struct spi_base* s, const char* name);
    const char*  (*get_port)(struct spi_base* s);
    int          (*clean_port)(struct spi_base* s);
    //about baudrate;
    int          (*set_baudrate)(struct spi_base* s, const char* br);
    int          (*get_baudrate)(struct spi_base* s);
    //about bytesize;
    int          (*set_bytesize)(struct spi_base* s, const char* bs);
    int          (*get_bytesize)(struct spi_base* s);
    //about timeout;
    int          (*set_timeout)(struct spi_base* s, const char* time);
    float        (*get_timeout)(struct spi_base* s);
};

typedef struct {
    char*            port;
    char*            baudrate;
    char*            bytesize;
    char*            timeout;
} spi_init_t;

/*
    global functions
*/
struct spi_base* base_spi_port_init(spi_init_t* sp);

#endif
