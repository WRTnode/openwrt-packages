/*
    FileName    :UART.c
    Description :Source code of UART config.
    Author      :SchumyHao
    Email       :schumy.haojl@gmail.com
    Version     :V10
    Data        :2014.04.07
*/

/*#define GNR_COM*/
/*#define USB_COM*/
#define ACM_COM
//TODO：If user didn't define the uart type.

/* Include files */
#include "UART.h"

/* Functions */
/* Congit UART function */
int ConfigUart(t_uart* pUart, struct termios* pOldCfg)
{
    assert(IS_FD(pUart->Fd));
    assert(IS_BAUD_RATE(pUart->BaudRate));
    assert(IS_DATA_BITS(pUart->DataBits));
    assert(IS_STOP_BITS(pUart->StopBits));
    assert(IS_PARITY(pUart->Parity));
    struct termios NewCfg;
    int Speed;

    //Read current TTY attributes.
    if(tcgetattr(pUart->Fd, pOldCfg) != 0) {
        perror("Can not got current com_config.\n");
        return -1;
    }

    NewCfg = *pOldCfg;
    //Set TTY as RAW.
    cfmakeraw(&NewCfg);
    NewCfg.c_cflag &= ~CSIZE;

    //Set TTY's baudrate.
    switch(pUart->BaudRate) {
        case BAUD_RATE_2400:
            Speed = B2400;
            break;
        case BAUD_RATE_4800:
            Speed = B4800;
            break;
        case BAUD_RATE_9600:
            Speed = B9600;
            break;
        case BAUD_RATE_19200:
            Speed = B19200;
            break;
        case BAUD_RATE_38400:
            Speed = B38400;
            break;
        case BAUD_RATE_57600:
            Speed = B57600;
            break;
        default:
            Speed = B9600;
    }

    cfsetispeed(&NewCfg, Speed);
    cfsetospeed(&NewCfg, Speed);

    //Set TTY's databits.
    switch(pUart->DataBits) {
        case DATA_BITS_7BITS:
            NewCfg.c_cflag |= CS7;
            break;
        case DATA_BITS_8BITS:
        default:
            NewCfg.c_cflag |= CS8;
    }

    //Set TTY's parity
    switch(pUart->Parity) {
        case PARITY_O:
            NewCfg.c_cflag |= (PARODD | PARENB);
            NewCfg.c_iflag |= INPCK;
            break;
        case PARITY_E:
            NewCfg.c_cflag |= PARENB;
            NewCfg.c_cflag &= ~PARODD;
            NewCfg.c_iflag |= INPCK;
            break;
        case PARITY_NONE:
        default:
            NewCfg.c_cflag &= ~PARENB;
            NewCfg.c_iflag &= ~INPCK;
    }

    //Set TTY's stopbits
    switch(pUart->StopBits) {
        case STOP_BITS_2BITS:
            NewCfg.c_cflag |= CSTOPB;
            break;
        case STOP_BITS_1BIT:
        default:
            NewCfg.c_cflag &= ~CSTOPB;
    }

    //Set TTY's receive wait time.
    NewCfg.c_cc[VTIME] = 0;         //Wait forever
    NewCfg.c_cc[VMIN] = 1;          //Return when recieve 1byte
    //Flush old data.
    tcflush(pUart->Fd, TCIOFLUSH);  //Clean input&output data

    if((tcsetattr(pUart->Fd, TCSANOW, &NewCfg)) != 0) {
        perror("Active configuration failed!\n");
        return -1;
    }

    return 0;
}

/* UART initialize function */
int InitUartStruct(t_uart* pUart)
{
    pUart->Fd = INIT_FD;
    pUart->pFp = NULL;
    pUart->BaudRate = BAUD_RATE_9600;
    pUart->DataBits = DATA_BITS_8BITS;
    pUart->StopBits = STOP_BITS_1BIT;
    pUart->Parity = PARITY_NONE;
    return 0;
}

/* Open UART's device file function */
int OpenPort(int const ComPort)
{
    assert(IS_COM_PORT(ComPort));
    int Fd;
#ifdef GNR_COM
    char* pDev[] = {"/dev/ttyS0","/dev/ttyS1","/dev/ttyS2"};
#endif
#ifdef USB_COM
    char* pDev[] = {"/dev/ttyUSB0","/dev/ttyUSB1","/dev/ttyUSB2"};
#endif
#ifdef ACM_COM
    char* pDev[] = {"/dev/ttyACM0","/dev/ttyACM1","/dev/ttyACM2"};
#endif
#ifdef DEBUG
    printf("UART device is %s.\n",pDev[ComPort]);
#endif

    if((Fd = open(pDev[ComPort], O_RDWR|O_NOCTTY|O_NONBLOCK)) < 0) {
        perror("Can not open serial port.\n");
        return -1;
    }

    if(fcntl(Fd, F_SETFL, 0) < 0) {
        perror("Can not block input.\n");
        return -1;
    }

    if(isatty(STDIN_FILENO) == 0) {
        perror("Serial is not a terminal device.\n");
        return -1;
    }

    return Fd;
}

