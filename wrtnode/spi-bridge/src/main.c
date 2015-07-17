#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#ifdef DEBUG
#define DEBUG_PRINT        printf
#else
#define DEBUG_PRINT(...)
#endif

typedef struct spi_write_data {
    unsigned long address;
    unsigned long value;
    unsigned long size;
} SPI_WRITE;

#define RT2880_SPI_READ_STR     "read"  /* SPI read operation */
#define RT2880_SPI_WRITE_STR    "write" /* SPI read operation */
#define RT2880_SPI_STATUS_STR   "status"
#define RT2880_SPI_START_STR    "start"
#define RT2880_SPI_FORCE_STR    "-f"

#define RT2880_SPI_READ        (2)
#define RT2880_SPI_STATUS      (2)
#define RT2880_SPI_START       (2)
#define RT2880_SPI_WRITE       (3)

#define SPI_MCU_READ           (0x01)
#define SPI_MCU_READ_LEN       (0x04)
#define SPI_MCU_WRITE          (0x10)
#define SPI_MCU_WRITE_LEN      (0x40)
#define SPI_MCU_READ_STATUS    (0xff)
#define SPI_STATUS_7688_READ_FROM_STM32_E    (1<<0)
#define SPI_STATUS_7688_READ_FROM_STM32_NE   (0<<0)
#define SPI_STATUS_7688_WRITE_TO_STM32_F     (1<<1)
#define SPI_STATUS_7688_WRITE_TO_STM32_NF    (0<<1)
#define SPI_STATUS_OK                        (0x80)

char usage[] =  "spicmd read/write [-f] WRTnode2r stm32 data(if write)\n"\
                "spicmd format:\n"\
                "  spicmd read \n"\
                "  spicmd write [string]\n"\
                "  spicmd status \n"\
                "-f Force read/write. Do not block"\
                "NOTE -- read/write/status value are in string\n";

#define SPI_MCU_READ_DELAY_US   (200)
#define SPI_MCU_WRITE_DELAY_US  (200)
#define SPI_MCU_CHECK_STATUS_DELAY_US   (100000)


static inline unsigned char read_status(int fd)
{
    unsigned char ch = 0;
    ioctl(fd, SPI_MCU_READ_STATUS, &ch);
    usleep(SPI_MCU_READ_DELAY_US);
    ioctl(fd, SPI_MCU_READ_STATUS, &ch);
    usleep(SPI_MCU_READ_DELAY_US);

    return ch;
}

static inline unsigned char read_len(int fd)
{
    unsigned char ch = 0;
    ioctl(fd, SPI_MCU_READ_LEN, &ch);
    usleep(SPI_MCU_READ_DELAY_US);
    ioctl(fd, SPI_MCU_READ_LEN, &ch);
    usleep(SPI_MCU_READ_DELAY_US);

    return ch;
}

static inline unsigned char read_ch(int fd)
{
    unsigned char ch = 0;
    ioctl(fd, SPI_MCU_READ, &ch);
    usleep(SPI_MCU_READ_DELAY_US);
    ioctl(fd, SPI_MCU_READ, &ch);
    usleep(SPI_MCU_READ_DELAY_US);

    return ch;
}

static inline void put_ch(int fd, unsigned char ch)
{
    ioctl(fd, SPI_MCU_WRITE, &ch);
    usleep(SPI_MCU_WRITE_DELAY_US);
}

static inline void put_len(int fd, unsigned char len)
{
    ioctl(fd, SPI_MCU_WRITE_LEN, &len);
    usleep(SPI_MCU_WRITE_DELAY_US);
}

int main(int argc, char* argv[])
{
    int chk_match, size, fd;
    int is_force = 0;
    unsigned char buf;

    if (argc < 2) {
        fprintf(stderr, "Usage:\n%s\n", usage);
        return -1;
    }

    if ((argc > 2) && (0 == strcmp(argv[2], RT2880_SPI_FORCE_STR))) {
        is_force = 1;
        argc --;
    }

    /* We use the last specified parameters, unless new ones are entered */
    switch (argc) {
        //case RT2880_SPI_STATUS:
        //case RT2880_SPI_START:
        case RT2880_SPI_READ:
            if(0 == strcmp(argv[1], RT2880_SPI_READ_STR)) {
                unsigned char status = SPI_STATUS_OK;
                unsigned char len = 0;
                unsigned char i = 0;
                char* data = NULL;

                fd = open("/dev/spiS0", O_RDONLY);
                if (fd <= 0) {
                    fprintf(stderr, "Please insmod module spi_drv.o!\n");
                    return -1;
                }
                if (is_force) {
                    status = read_status(fd);
                    DEBUG_PRINT("read status = 0x%x\n", status);
                    if(!(status & SPI_STATUS_OK)) {
                        fprintf(stderr, "stm32 spi read error.\n");
                        return -1;
                    }
                    if(status & SPI_STATUS_7688_READ_FROM_STM32_E) {
                        fprintf(stderr, "stm32 read buf empty.\n");
                        return -1;
                    }
                }
                else {
                    do {
                        status = read_status(fd);
                        DEBUG_PRINT("read status = 0x%x\n", status);
                        if(status & (SPI_STATUS_OK) && 
                          (!(status & SPI_STATUS_7688_READ_FROM_STM32_E))) {
                            break;
                        }
                        usleep(SPI_MCU_CHECK_STATUS_DELAY_US);
                    }while(1);
                }
                len = read_len(fd);
                DEBUG_PRINT("read len = %d\n", len);
                if(0 == len) {
                    fprintf(stderr, "read length is 0.\n");
                    return -1;
                }
                if(NULL == (data = (char*)malloc(len*sizeof(char)))) {
                    fprintf(stderr, "read data malloc error!\n");
                    return -1;
                }
                for(i=0; i<len; i++) {
                    data[i] = read_ch(fd);
                    DEBUG_PRINT("read data[%d] = 0x%x %c\n", i, data[i] , data[i]);
                }
                printf("%s\n",data);
                free(data);
                close(fd);
            }
            else if (0 == strcmp(argv[1], RT2880_SPI_STATUS_STR)) {
                unsigned char status = 0;
                fd = open("/dev/spiS0", O_RDONLY);
                if (fd <= 0) {
                    fprintf(stderr, "Please insmod module spi_drv.o!\n");
                    return -1;
                }
                status = read_status(fd);
                if(status & SPI_STATUS_OK) {
                    if(status & SPI_STATUS_7688_READ_FROM_STM32_E) {
                        printf("Can not read. STM32 read buf empty.\n");
                    }
                    if(status & SPI_STATUS_7688_WRITE_TO_STM32_F) {
                        printf("Can not write. STM32 write buf full.\n");
                    }
                    if(status == SPI_STATUS_OK) {
                        printf("OK\n");
                    }
                }
                else {
                    printf("spi stm32 read error.\n");
                }
                close(fd);
            }
            else if (0 == strcmp(argv[1], RT2880_SPI_START_STR)) {
                unsigned char len = 0;
                unsigned char i = 0;
                const char data[] = RT2880_SPI_START_STR;

                fd = open("/dev/spiS0", O_RDWR);
                if (fd <= 0) {
                    fprintf(stderr, "Please insmod module spi_drv.o!\n");
                    return -1;
                }

                len = strlen(data);
                if(0 == len) {
                    fprintf(stderr, "write length is 0.\n");
                    return -1;
                }
                put_len(fd, len);
                for(i=0; i<len; i++) {
                    put_ch(fd, data[i]); 
                    DEBUG_PRINT("write data[%d] = 0x%x %c\n", i, data[i] , data[i]);
                }
                close(fd);
            }
            else {
                fprintf(stderr, "Usage:\n%s\n", usage);
                return -1;
            }
            break;
        case RT2880_SPI_WRITE:
            if(0 == strcmp(argv[1], RT2880_SPI_WRITE_STR)) {
                unsigned char status = 0;
                unsigned char len = 0;
                unsigned char i = 0;
                char* data = (is_force)? argv[3]: argv[2];

                fd = open("/dev/spiS0", O_RDWR);
                if (fd <= 0) {
                    fprintf(stderr, "Please insmod module spi_drv.o!\n");
                    return -1;
                }

                if (is_force) {
                    status = read_status(fd);
                    DEBUG_PRINT("write status = 0x%x\n", status);
                    if(status & SPI_STATUS_OK) {
                        if(status & SPI_STATUS_7688_WRITE_TO_STM32_F) {
                            fprintf(stderr, "stm32 write buf full.\n");
                            return -1;
                        }
                    }
                    else {
                        fprintf(stderr, "stm32 spi read error.\n");
                    }
                }
                else {
                    do {
                        status = read_status(fd);
                        DEBUG_PRINT("write status = 0x%x\n", status);
                        if((status & SPI_STATUS_OK) &&
                           (!(status & SPI_STATUS_7688_WRITE_TO_STM32_F))) {
                            break;
                        }
                        usleep(SPI_MCU_CHECK_STATUS_DELAY_US);
                    } while(1);
                }
                len = strlen(data);
                if(0 == len) {
                    fprintf(stderr, "write length is 0.\n");
                    return -1;
                }
                put_len(fd, len);
                for(i=0; i<len; i++) {
                    put_ch(fd, data[i]); 
                    DEBUG_PRINT("write data[%d] = 0x%x %c\n", i, data[i] , data[i]);
                }
                close(fd);
            }
            else {
                fprintf(stderr, "Usage:\n%s\n", usage);
                return -1;
            }
            break;
        default:
            fprintf(stderr, "Usage:\n%s\n\n", usage);
            return -1;
    }

    return 0;
}
