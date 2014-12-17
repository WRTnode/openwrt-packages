/*
    FileName    :serial_posix.h
    Description :head file of posix serial port.
    Author      :SchumyHao
    Email       :schumy.haojl@gmail.com
    Version     :V01
    Data        :2014.10.31
*/

#ifndef __SERIAL_POSIX_H__
#define __SERIAL_POSIX_H__
/*
    include files
*/
#include <termios.h>
#include "serial.h"

/* posix serial port error number */
typedef enum {
    POSIX_SERIAL_ERR_OPEN = (int)SERIAL_ERR_LAST,
    POSIX_SERIAL_ERR_PARA,
    POSIX_SERIAL_ERR_FD,
    POSIX_SERIAL_ERR_SETTING,
    POSIX_SERIAL_ERR_READ,
    POSIX_SERIAL_ERR_WRITE,
    POSIX_SERIAL_ERR_DRAIN,

    POSIX_SERIAL_ERR_LAST
} posix_serial_err_t;
/*
    struct of posix serial port
*/
struct posix_serial {
    //about posix port variables
    int fd;
    struct termios oldtio;
    struct termios newtio;
    //serial base class
    struct serial_base* sb;

    //member functions
    int (*open)(struct posix_serial* s);
    int (*config_port)(struct posix_serial* s);
    int (*close)(struct posix_serial* s);
    int (*read_in_wait)(struct posix_serial* s);
    int (*read)(struct posix_serial* s, char* dst, int size);
    int (*write_in_wait)(struct posix_serial* s);
    int (*write)(struct posix_serial* s, const char* src, int size);
    int (*drain)(struct posix_serial* s);
    int (*flush_input)(struct posix_serial* s);
    int (*flush_output)(struct posix_serial* s);
};

typedef struct {
    serial_init_t sp;
} posix_serial_init_t;

/*
    global functions
*/
struct posix_serial* posix_serial_port_init(posix_serial_init_t* psp);
#endif
