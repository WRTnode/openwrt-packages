/*
    FileName    :serial.h
    Description :head file of common serial.
    Author      :SchumyHao
    Email       :schumy.haojl@gmail.com
    Version     :V01
    Data        :2014.10.31
*/
#ifndef __SERIAL_H__
#define __SERIAL_H__
/*
    include files
*/
#include <stdbool.h>

/*
    serial specific values
*/
typedef enum {
//the order must be the same as support_parity
    SERIAL_PARITY_NONE = 0,
    SERIAL_PARITY_EVEN,
    SERIAL_PARITY_ODD,

    SERIAL_PARITY_INVALID,
    SERIAL_PARITY_DEFAULT = SERIAL_PARITY_NONE
} serial_parity_t;
#define IS_SERIAL_PARITY(p)      ((p) < SERIAL_PARITY_INVALID)

typedef enum {
//the order must be the same as support_bytesize
    SERIAL_BITS_7 = 0,
    SERIAL_BITS_8,

    SERIAL_BITS_INVALID,
    SERIAL_BITS_DEFAULT = SERIAL_BITS_8
} serial_bits_t;
#define IS_SERIAL_BITS(b)        ((b) < SERIAL_BITS_INVALID)

typedef enum {
//the order must be the same as support_baudrate
    SERIAL_BAUD_1200 = 0,
    SERIAL_BAUD_2400,
    SERIAL_BAUD_4800,
    SERIAL_BAUD_9600,
    SERIAL_BAUD_19200,
    SERIAL_BAUD_38400,
    SERIAL_BAUD_57600,
    SERIAL_BAUD_115200,

    SERIAL_BAUD_INVALID,
    SERIAL_BAUD_DEFAULT = SERIAL_BAUD_57600
} serial_baud_t;
#define IS_SERIAL_BAUD(b)        ((b) < SERIAL_BAUD_INVALID)

typedef enum {
//the order must be the same as support_stopbits
    SERIAL_STOPBIT_1 = 0,
    SERIAL_STOPBIT_2,

    SERIAL_STOPBIT_INVALID,
    SERIAL_STOPBIT_DEFAULT = SERIAL_STOPBIT_1
} serial_stopbit_t;
#define IS_SERIAL_STOP(s)        ((s) < SERIAL_STOPBIT_INVALID)

typedef struct {
    unsigned int    sec;
    unsigned int    usec;
} serial_timeout_t;

/* serial port error number */
typedef enum {
    SERIAL_ERR_NODEV = 1,
    SERIAL_ERR_MEM,
    SERIAL_ERR_INPUT_NULL,
    SERIAL_ERR_NOT_SUPPORT,

    SERIAL_ERR_LAST
} serial_err_t;

/*
    basic serial port structure
*/
struct serial_base {
    //below values are internal variables. do not use them directly.
    bool             open_flag;
    char*            port;
    serial_baud_t    baudrate;
    serial_bits_t    bytesize;
    serial_parity_t  parity;
    serial_stopbit_t stopbits;
    serial_timeout_t timeout;
    //TODO: mt7620 is not support flow control. add flow control support if this code used in other chip.

    //about serial status.
    int          (*set_is_open)(struct serial_base* s);
    int          (*clean_is_open)(struct serial_base* s);
    int          (*is_open)(struct serial_base* s);
    //about serial port name.
    int          (*set_port)(struct serial_base* s, const char* name);
    const char*  (*get_port)(struct serial_base* s);
    int          (*clean_port)(struct serial_base* s);
    //about baudrate;
    int          (*set_baudrate)(struct serial_base* s, const char* br);
    const char*  (*get_baudrate)(struct serial_base* s);
    const char** (*get_supported_baudrate)(struct serial_base* s);
    //about bytesize;
    int          (*set_bytesize)(struct serial_base* s, const char* bs);
    const char*  (*get_bytesize)(struct serial_base* s);
    const char** (*get_supported_bytesize)(struct serial_base* s);
    //about parity;
    int          (*set_parity)(struct serial_base* s, const char* pa);
    const char*  (*get_parity)(struct serial_base* s);
    const char** (*get_supported_parity)(struct serial_base* s);
    //about stopbits;
    int          (*set_stopbits)(struct serial_base* s, const char* sb);
    const char*  (*get_stopbits)(struct serial_base* s);
    const char** (*get_supported_stopbits)(struct serial_base* s);
    //about timeout;
    int          (*set_timeout)(struct serial_base* s, const char* time);
    float        (*get_timeout)(struct serial_base* s);
};

typedef struct {
    char*            port;
    char*            baudrate;
    char*            bytesize;
    char*            parity;
    char*            stopbits;
    char*            timeout;
} serial_init_t;

/*
    global functions
*/
struct serial_base* base_serial_port_init(serial_init_t* sp);

#endif
