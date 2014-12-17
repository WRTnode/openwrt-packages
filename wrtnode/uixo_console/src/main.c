/*
    FileName    :main.c
    Description :The main function of uixo_console.
    Author      :SchumyHao
    Email       :schumy.haojl@gmail.com
    Version     :V02
    Data        :2014.12.13
*/
/* Include files */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/select.h>

#include "serial.h"
#include "serial_posix.h"
#include "uixo_console.h"

int main (int argc, char* argv[])
{
    int ret = 0;

    char* uixo_cmd_rx_file = "/tmp/uixo_cmd_rx";
    char* uixo_cmd_tx_file = "/tmp/uixo_cmd_tx";
    uixo_port_t uixo_port;

    /* config uixo port */
    uixo_port.rx_cmd_off = 0;
    uixo_port.tx_cmd_off = 0;
    uixo_port.rx_cmd_fd = 0;
    uixo_port.tx_cmd_fd = 0;
    uixo_port.rx_head = "HY>";
    uixo_port.tx_head = "HY<";
    uixo_port.port = NULL;
    /* make fifo */
    umask(0);
    if(access(uixo_cmd_rx_file,F_OK)==0) { /* file exist */
        if(unlink(uixo_cmd_rx_file)<0) {
            printf("delet exist file failed\n");
            return -1;
        }
    }
    if(access(uixo_cmd_tx_file,F_OK)==0) { /* file exist */
        if(unlink(uixo_cmd_tx_file)<0) {
            printf("delet exist file failed\n");
            return -1;
        }
    }
    if(mkfifo(uixo_cmd_rx_file,
              S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) {
        printf("make fifo failed\n");
        return -2;
    }
    if(mkfifo(uixo_cmd_tx_file,
              S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) {
        printf("make fifo failed\n");
        return -2;
    }
    /* open fifo */
    uixo_port.rx_cmd_fd = open(uixo_cmd_rx_file,O_WRONLY);
    uixo_port.tx_cmd_fd = open(uixo_cmd_tx_file,O_RDONLY);
    printf("open fifo success\n");
    //serial test.
    posix_serial_init_t* psp = NULL;
    struct posix_serial* ps = NULL;
    psp = (posix_serial_init_t*)calloc(1,sizeof(posix_serial_init_t));
    if(NULL==psp) {
        return -1;
    }
    /*
     support_baudrate[] = {"1200","2400","4800","9600",
                           "19200","38400","57600","115200"};
     support_bytesize[] = {"7","8"};
     support_parity[]   = {"none","even","odd"};
     support_stopbits[] = {"1","2"};
    */
    psp->sp.baudrate = "115200";
    psp->sp.bytesize = "8";
    psp->sp.parity   = "none";
    psp->sp.port     = "/dev/ttyS0";
    psp->sp.stopbits = "1";
    psp->sp.timeout  = "0.005";
    ps = posix_serial_port_init(psp);
    free(psp);
    psp = NULL;
    if(NULL==ps) {
    	printf("make serial port failed\n");
        return -1;
    }
    if((ret = ps->open(ps))<0) {
    	printf("make open serial port failed\n");
        return -2;
    }
    uixo_port.port = ps;

    while(true) { /* loop */
        /* select when port or tx_fd can read */
        fd_set readfds;

        FD_ZERO(&readfds);
        FD_SET(ps->fd, &readfds);
        FD_SET(uixo_port.tx_cmd_fd, &readfds);
        int max_fd = (ps->fd > uixo_port.tx_cmd_fd)? ps->fd: uixo_port.tx_cmd_fd;
        /* forever wait */
        if(select(max_fd+1, &readfds, NULL, NULL, NULL) < 0) {
            printf("select err\n");
            return -1;
        }
        else {
            if(FD_ISSET(ps->fd,&readfds)) { /* port can read, do rx handler */
                if(uixo_rx_handler(&uixo_port) < 0) {
                    printf("uixo rx handler err\n");
                    //return -2;
                }
            }
            if(FD_ISSET(uixo_port.tx_cmd_fd,&readfds)) { /* tx file can read, do tx handler */
                if(uixo_tx_handler(&uixo_port) < 0) {
                    printf("uixo tx handler err\n");
                    //return -3;
                }
            }
        }
    }
    return 0;
}
