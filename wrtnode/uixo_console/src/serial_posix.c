/*
    FileName    :serial_posix.c
    Description :Source code of posix serial port.
    Author      :SchumyHao
    Email       :schumy.haojl@gmail.com
    Version     :V01
    Data        :2014.10.31
*/
/*
    include files
*/
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <sys/select.h>

#include "serial.h"
#include "serial_posix.h"

/*
    member functions
*/
static int posix_serial_open(struct posix_serial* s)
{
    int ret = 0;
    if(NULL == s) {
		printf("open fail1\n");
        ret = -SERIAL_ERR_INPUT_NULL;
        goto POSIX_SERIAL_OPEN_INPUT_ERROR;
    }
    //can not open posix serial port twice.
    if(s->sb->is_open(s->sb)) {
		printf("open fail2\n");
        ret = -POSIX_SERIAL_ERR_OPEN;
        goto POSIX_SERIAL_OPEN_REOPEN_ERROR;
    }
    //check base serial parameters.
    if(!IS_SERIAL_BAUD(s->sb->baudrate)) {
		printf("open fail3\n");
        ret = -POSIX_SERIAL_ERR_PARA;
        goto POSIX_SERIAL_OPEN_PARAMETER_ERROR;
    }
    if(!IS_SERIAL_BITS(s->sb->bytesize)) {
		printf("open fail4\n");
        ret = -POSIX_SERIAL_ERR_PARA;
        goto POSIX_SERIAL_OPEN_PARAMETER_ERROR;
    }
    if(!IS_SERIAL_PARITY(s->sb->parity)) {
		printf("open fail5\n");
        ret = -POSIX_SERIAL_ERR_PARA;
        goto POSIX_SERIAL_OPEN_PARAMETER_ERROR;
    }
    if(!IS_SERIAL_STOP(s->sb->stopbits)) {
		printf("open fail6\n");
        ret = -POSIX_SERIAL_ERR_PARA;
        goto POSIX_SERIAL_OPEN_PARAMETER_ERROR;
    }

    //open posix serial port
    s->fd = open(s->sb->get_port(s->sb), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if(s->fd < 0) {
		printf("open fail\n");
        ret = -POSIX_SERIAL_ERR_OPEN;
        goto POSIX_SERIAL_OPEN_OPEN_ERROR;
    }
    //set is_open flag.
    s->sb->set_is_open(s->sb);
    if(fcntl(s->fd, F_SETFL, 0) < 0) {
        ret = -POSIX_SERIAL_ERR_OPEN;
        goto POSIX_SERIAL_OPEN_CONFIG_ERROR;
    }
    if(tcgetattr(s->fd, &s->oldtio) < 0) {
        ret = -POSIX_SERIAL_ERR_OPEN;
        goto POSIX_SERIAL_OPEN_CONFIG_ERROR;
    }
    if(tcgetattr(s->fd, &s->newtio) < 0) {
        ret = -POSIX_SERIAL_ERR_OPEN;
        goto POSIX_SERIAL_OPEN_CONFIG_ERROR;
    }
    //config posix serial port. close it if config failed.
    if((ret = s->config_port(s)) < 0) {
        goto POSIX_SERIAL_OPEN_CONFIG_ERROR;
    }
    //clear input & output buffer.
    if((ret = s->flush_input(s)) < 0) {
        goto POSIX_SERIAL_OPEN_FLUSH_ERROR;
    }
    if((ret = s->flush_output(s)) < 0) {
        goto POSIX_SERIAL_OPEN_FLUSH_ERROR;
    }
    return 0;

POSIX_SERIAL_OPEN_FLUSH_ERROR:
POSIX_SERIAL_OPEN_CONFIG_ERROR:
    s->sb->clean_is_open(s->sb);
    close(s->fd);
POSIX_SERIAL_OPEN_OPEN_ERROR:
    s->fd = 0;
POSIX_SERIAL_OPEN_PARAMETER_ERROR:
POSIX_SERIAL_OPEN_REOPEN_ERROR:
POSIX_SERIAL_OPEN_INPUT_ERROR:
	printf("open port fail\n");
    return ret;
}

static int posix_serial_config_port(struct posix_serial* s)
{
    int ret = 0;
    speed_t  baudrate = 0;
    tcflag_t bytebits = 0;
    tcflag_t parity = 0;
    tcflag_t stopbits = 0;
    struct termios tmp_settings;

    if(NULL == s) {
        ret = -SERIAL_ERR_INPUT_NULL;
        goto POSIX_SERIAL_CONFIG_PORT_INPUT_ERROR;
    }
    //check serial port fd
    if(s->fd <= 0) {
        ret = -POSIX_SERIAL_ERR_FD;
        goto POSIX_SERIAL_CONFIG_PORT_FD_ERROR;
    }
    //enable local & read.
    s->newtio.c_cflag |= (CLOCAL|CREAD);
    //set up raw mode  cfmakeraw(&s->newtio);
    s->newtio.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                           | INLCR | IGNCR | ICRNL | IXON);
    s->newtio.c_oflag &= ~OPOST;
    s->newtio.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    s->newtio.c_cflag &= ~(CSIZE | PARENB);
    s->newtio.c_cflag |= CS8;

    //set up baudrate
    switch (s->sb->baudrate) {
        case SERIAL_BAUD_1200:
            baudrate = B1200;
            break;
        case SERIAL_BAUD_2400:
            baudrate = B2400;
            break;
        case SERIAL_BAUD_4800:
            baudrate = B4800;
            break;
        case SERIAL_BAUD_9600:
            baudrate = B9600;
            break;
        case SERIAL_BAUD_19200:
            baudrate = B19200;
            break;
        case SERIAL_BAUD_38400:
            baudrate = B38400;
            break;
        case SERIAL_BAUD_57600:
            baudrate = B57600;
            break;
        case SERIAL_BAUD_115200:
            baudrate = B115200;
            break;
        default:
            ret = -POSIX_SERIAL_ERR_PARA;
            goto POSIX_SERIAL_CONFIG_PORT_BAUDRATE_ERROR;
    }
    if(cfsetispeed(&s->newtio, baudrate) < 0) {
        ret = -POSIX_SERIAL_ERR_SETTING;
        goto POSIX_SERIAL_CONFIG_PORT_BAUDRATE_ERROR;
    }
    if(cfsetospeed(&s->newtio, baudrate) < 0) {
        ret = -POSIX_SERIAL_ERR_SETTING;
        goto POSIX_SERIAL_CONFIG_PORT_BAUDRATE_ERROR;
    }
    //set up byte bits
    switch (s->sb->bytesize) {
        case SERIAL_BITS_7:
            bytebits = CS7;
            break;
        case SERIAL_BITS_8:
            bytebits = CS8;
            break;
        default:
            ret = -POSIX_SERIAL_ERR_PARA;
            goto POSIX_SERIAL_CONFIG_PORT_BYTEBITS_ERROR;
    }
    s->newtio.c_cflag &= ~CSIZE;
    s->newtio.c_cflag |= bytebits;
    //set up stopbits
    switch (s->sb->stopbits) {
        case SERIAL_STOPBIT_1:
            stopbits = 0;
            break;
        case SERIAL_STOPBIT_2:
            stopbits = CSTOPB;
            break;
        default:
            ret = -POSIX_SERIAL_ERR_PARA;
            goto POSIX_SERIAL_CONFIG_PORT_STOPBITS_ERROR;
    }
    s->newtio.c_cflag &= ~CSTOPB;
    s->newtio.c_cflag |= stopbits;
    //set up parity
    switch (s->sb->parity) {
        case SERIAL_PARITY_NONE:
            parity = 0;
            break;
        case SERIAL_PARITY_EVEN:
            parity = INPCK | PARENB;
            break;
        case SERIAL_PARITY_ODD:
            parity = INPCK | PARENB | PARODD;
            break;
        default:
            ret = -POSIX_SERIAL_ERR_PARA;
            goto POSIX_SERIAL_CONFIG_PORT_PARITY_ERROR;
    }
    s->newtio.c_iflag &= ~(INPCK|ISTRIP);
    s->newtio.c_cflag &= ~(PARENB|PARODD);
    s->newtio.c_iflag |= (INPCK&parity);
    s->newtio.c_cflag |= ((PARENB|PARODD)&parity);
    //set up vmin & vtime. use select to delay, not vtime
    s->newtio.c_cc[VMIN] = 0;
    s->newtio.c_cc[VTIME] = 0;
    //flush buffer
    if((ret = s->flush_input(s)) < 0) {
        goto POSIX_SERIAL_CONFIG_PORT_FLUSH_ERROR;
    }
    if((ret = s->flush_output(s)) < 0) {
        goto POSIX_SERIAL_CONFIG_PORT_FLUSH_ERROR;
    }
    //active settings
    if(tcsetattr(s->fd, TCSANOW, &s->newtio) < 0) {
        ret = -POSIX_SERIAL_ERR_SETTING;
        goto POSIX_SERIAL_CONFIG_PORT_ACTIVE_SETTING_ERROR;
    }
    //confirm setting has been done
    if(tcgetattr(s->fd, &tmp_settings) < 0) {
        ret = -POSIX_SERIAL_ERR_SETTING;
        goto POSIX_SERIAL_CONFIG_PORT_CONFIRM_SETTING_ERROR;
    }
    if((tmp_settings.c_iflag != s->newtio.c_iflag) ||
       (tmp_settings.c_oflag != s->newtio.c_oflag) ||
       (tmp_settings.c_cflag != s->newtio.c_cflag) ||
       (tmp_settings.c_lflag != s->newtio.c_lflag)) {
        ret = -POSIX_SERIAL_ERR_SETTING;
        goto POSIX_SERIAL_CONFIG_PORT_CONFIRM_SETTING_ERROR;
    }
    return 0;

POSIX_SERIAL_CONFIG_PORT_CONFIRM_SETTING_ERROR:
    tcsetattr(s->fd, TCSANOW, &s->oldtio);
POSIX_SERIAL_CONFIG_PORT_ACTIVE_SETTING_ERROR:
POSIX_SERIAL_CONFIG_PORT_FLUSH_ERROR:
POSIX_SERIAL_CONFIG_PORT_PARITY_ERROR:
POSIX_SERIAL_CONFIG_PORT_STOPBITS_ERROR:
POSIX_SERIAL_CONFIG_PORT_BYTEBITS_ERROR:
POSIX_SERIAL_CONFIG_PORT_BAUDRATE_ERROR:
POSIX_SERIAL_CONFIG_PORT_FD_ERROR:
POSIX_SERIAL_CONFIG_PORT_INPUT_ERROR:
    return ret;
}

static int posix_serial_close(struct posix_serial* s)
{
    int ret = 0;

    if(NULL == s) {
        ret = -SERIAL_ERR_INPUT_NULL;
        goto POSIX_SERIAL_CLOSE_INPUT_ERROR;
    }
    if(s->sb->is_open(s->sb)) {
        /* push all tx buffer to serial */
        s->drain(s);
        /* reconfig oldtio */
        tcsetattr(s->fd, TCSANOW, &s->oldtio);
        /* clean serial open flag */
        s->sb->clean_is_open(s->sb);
        /* closd fd and set fd to 0 */
        close(s->fd);
        s->fd = 0;
        /* free port name */
        s->sb->clean_port(s->sb);
        /* free serial_base */
        free(s->sb);
        /* free posix_serial */
        free(s);
    }
    else {
        printf("%s: posix serial(%s) is not open, close failed.\n", __func__, s->sb->get_port(s->sb));
    }
    return 0;

POSIX_SERIAL_CLOSE_INPUT_ERROR:
    return ret;
}

static int posix_serial_read_in_wait(struct posix_serial* s)
{
    // this ioctl function have not supported by 8250 driver.
    return -1;
}

static int posix_serial_read(struct posix_serial* s, char* dst, int size)
{
    int ret = 0;
    int nleft = size;
    int nread = 0;
    char* ptr = dst;

    if((NULL==s)||(NULL==dst)) {
        ret = -SERIAL_ERR_INPUT_NULL;
        goto POSIX_SERIAL_READ_INPUT_ERROR;
    }
    //check whether the port is open or not
    if(!(s->sb->is_open(s->sb))) {
        ret = -POSIX_SERIAL_ERR_OPEN;
        goto POSIX_SERIAL_READ_PORT_NOT_OPEN_ERROR;
    }
    //read until nleft is empty
    while(nleft > 0) {
        int readyfds = 0;
        fd_set readfds;
        struct timeval tv;

        tv.tv_sec = s->sb->timeout.sec;
        tv.tv_usec = s->sb->timeout.usec;
        FD_ZERO(&readfds);
        FD_SET(s->fd, &readfds);
        readyfds = select(s->fd+1, &readfds, NULL, NULL, &tv);
        if(0 == readyfds) { //timeout. return read byte.
            break;
        }
        if(readyfds < 0) { //error
            ret = -POSIX_SERIAL_ERR_READ;
            goto POSIX_SERIAL_READ_READ_ERROR;
        }
        nread = read(s->fd, ptr, nleft);
        if(nread < 0) {
            ret = -POSIX_SERIAL_ERR_READ;
            goto POSIX_SERIAL_READ_READ_ERROR;
        }
        nleft -= nread;
        ptr += nread;
    }
    return (size-nleft);

POSIX_SERIAL_READ_READ_ERROR:
POSIX_SERIAL_READ_PORT_NOT_OPEN_ERROR:
POSIX_SERIAL_READ_INPUT_ERROR:
    return ret;
}

static int posix_serial_write_in_wait(struct posix_serial* s)
{
    // this ioctl function have not supported by 8250 driver.
    return -1;
}

static int posix_serial_write(struct posix_serial* s, const char* src, int size)
{
    int ret = 0;
    int nleft = size;
    int nwrite = 0;
    const char* ptr = src;

    if((NULL==s)||(NULL==src)) {
        ret = -SERIAL_ERR_INPUT_NULL;
        goto POSIX_SERIAL_WRITE_INPUT_ERROR;
    }
    //check whether the port is open or not
    if(!(s->sb->is_open(s->sb))) {
        ret = -POSIX_SERIAL_ERR_OPEN;
        goto POSIX_SERIAL_WRITE_PORT_NOT_OPEN_ERROR;
    }
    //write until nleft is empty
    while(nleft > 0) {
        int readyfds = 0;
        fd_set writefds;
        struct timeval tv;

        tv.tv_sec = s->sb->timeout.sec;
        tv.tv_usec = s->sb->timeout.usec;
        FD_ZERO(&writefds);
        FD_SET(s->fd, &writefds);
        readyfds = select(s->fd+1, NULL, &writefds, NULL, &tv);
        if(0 == readyfds) { //timeout. return read byte.
            break;
        }
        if(readyfds < 0) { //error
            ret = -POSIX_SERIAL_ERR_WRITE;
            goto POSIX_SERIAL_WRITE_WRITE_ERROR;
        }
        nwrite = write(s->fd, ptr, nleft);
        if(nwrite < 0) {
            ret = -POSIX_SERIAL_ERR_WRITE;
            goto POSIX_SERIAL_WRITE_WRITE_ERROR;
        }
        nleft -= nwrite;
        ptr += nwrite;
    }
    return (size-nleft);

POSIX_SERIAL_WRITE_WRITE_ERROR:
POSIX_SERIAL_WRITE_PORT_NOT_OPEN_ERROR:
POSIX_SERIAL_WRITE_INPUT_ERROR:
    return ret;
}

static int posix_serial_drain(struct posix_serial* s)
{
    int ret = 0;

    if(NULL == s) {
        ret = -SERIAL_ERR_INPUT_NULL;
        goto POSIX_SERIAL_DRAIN_INPUT_ERROR;
    }
    if(!(s->sb->is_open(s->sb))) {
        ret = -POSIX_SERIAL_ERR_OPEN;
        goto POSIX_SERIAL_DRAIN_PORT_NOT_OPEN_ERROR;
    }
    if(tcdrain(s->fd)<0) { //send all waiting bytes to port.
        ret = -POSIX_SERIAL_ERR_DRAIN;
        goto POSIX_SERIAL_DRAIN_DRAIN_ERROR;
    }
    return 0;

POSIX_SERIAL_DRAIN_DRAIN_ERROR:
POSIX_SERIAL_DRAIN_PORT_NOT_OPEN_ERROR:
POSIX_SERIAL_DRAIN_INPUT_ERROR:
    return ret;
}

static int posix_serial_flush_input(struct posix_serial* s)
{
    int ret = 0;

    if(NULL == s) {
        ret = -SERIAL_ERR_INPUT_NULL;
        goto POSIX_SERIAL_FLUSH_INPUT_INPUT_ERROR;
    }
    if(!(s->sb->is_open(s->sb))) {
        ret = -POSIX_SERIAL_ERR_OPEN;
        goto POSIX_SERIAL_FLUSH_INPUT_PORT_NOT_OPEN_ERROR;
    }
    if(tcflush(s->fd, TCIFLUSH)<0) { //clear all input buffer bytes.
        ret = -POSIX_SERIAL_ERR_DRAIN;
        goto POSIX_SERIAL_FLUSH_INPUT_FLUSH_ERROR;
    }
    return 0;

POSIX_SERIAL_FLUSH_INPUT_FLUSH_ERROR:
POSIX_SERIAL_FLUSH_INPUT_PORT_NOT_OPEN_ERROR:
POSIX_SERIAL_FLUSH_INPUT_INPUT_ERROR:
    return ret;
}

static int posix_serial_flush_output(struct posix_serial* s)
{
    int ret = 0;

    if(NULL == s) {
        ret = -SERIAL_ERR_INPUT_NULL;
        goto POSIX_SERIAL_FLUSH_OUTPUT_INPUT_ERROR;
    }
    if(!(s->sb->is_open(s->sb))) {
        ret = -POSIX_SERIAL_ERR_OPEN;
        goto POSIX_SERIAL_FLUSH_OUTPUT_PORT_NOT_OPEN_ERROR;
    }
    if(tcflush(s->fd, TCOFLUSH)<0) { //clear all output buffer bytes.
        ret = -POSIX_SERIAL_ERR_DRAIN;
        goto POSIX_SERIAL_FLUSH_OUTPUT_FLUSH_ERROR;
    }
    return 0;

POSIX_SERIAL_FLUSH_OUTPUT_FLUSH_ERROR:
POSIX_SERIAL_FLUSH_OUTPUT_PORT_NOT_OPEN_ERROR:
POSIX_SERIAL_FLUSH_OUTPUT_INPUT_ERROR:
    return ret;
}

/*
    global functions
*/
struct posix_serial* posix_serial_port_init(posix_serial_init_t* psp)
{
    serial_init_t* sp = NULL;
    struct posix_serial* ps = NULL;

    if (NULL==psp) {
		printf("fail1\n");
        goto POSIX_SERIAL_PORT_INIT_INPUT_ERROR;
    }

    /* malloc a posix_serial port */
    ps = (struct posix_serial*)malloc(1*sizeof(struct posix_serial));
    if(NULL==ps) {
		printf("fail2\n");
        goto POSIX_SERIAL_PORT_INIT_MALLOC_PS_ERROR;
    }
    /* copy basic serial init values and ini basic serial */
    sp = &psp->sp;
    ps->sb = base_serial_port_init(sp);
    if(NULL==ps->sb) {
		printf("fail3\n");
        goto POSIX_SERIAL_PORT_INIT_INIT_SB_ERROR;
    }
    /* init all posix serial parameters */
    ps->fd = 0;
    ps->open = posix_serial_open;
    ps->config_port = posix_serial_config_port;
    ps->close = posix_serial_close;
    ps->read_in_wait = posix_serial_read_in_wait;
    ps->read = posix_serial_read;
    ps->write_in_wait = posix_serial_write_in_wait;
    ps->write = posix_serial_write;
    ps->drain = posix_serial_drain;
    ps->flush_input = posix_serial_flush_input;
    ps->flush_output = posix_serial_flush_output;
    /* return posix serial */
    return ps;

POSIX_SERIAL_PORT_INIT_INIT_SB_ERROR:
    free(ps);
POSIX_SERIAL_PORT_INIT_MALLOC_PS_ERROR:
POSIX_SERIAL_PORT_INIT_INPUT_ERROR:
    return NULL;
}
