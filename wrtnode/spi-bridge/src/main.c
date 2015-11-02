#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>

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

#define SPI_FRAME_MAX_LEN      (255)

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

char usage[] =  "spicmd [read/write] [-f] WRTnode2r stm32 data(if write)\n"\
                "spicmd format:\n"\
                "  spicmd read \n"\
                "  spicmd write [string]\n"\
                "  spicmd status \n"\
                "-f Force read/write. Do not block\n"
                "NOTE -- read/write/status value are in string\n"\
                "spicmd\n  Open spiconsole.\n";

#define SPI_MCU_READ_DELAY_US   (200)
#define SPI_MCU_WRITE_DELAY_US  (200)
#define SPI_MCU_CHECK_STATUS_DELAY_US   (100000)


static inline unsigned char read_status(int fd)
{
    unsigned char ch = 0;
    ioctl(fd, SPI_MCU_READ_STATUS, &ch);
    usleep(SPI_MCU_READ_DELAY_US);

    return ch;
}

static inline unsigned char read_len(int fd)
{
    unsigned char ch = 0;
    ioctl(fd, SPI_MCU_READ_LEN, &ch);
    usleep(SPI_MCU_READ_DELAY_US);

    return ch;
}

static inline unsigned char read_ch(int fd)
{
    unsigned char ch = 0;
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

static pthread_t read_mcu_tidp;
static pthread_t read_stdin_tidp;
static pthread_mutex_t spi_mutex;

/* *
 * NOTICE: If 7688 read one byte from STM32, It will got right byte in next read cmd.
 * */
static void* read_mcu_handler(void* arg)
{
    int fd = *(int*)arg;
    unsigned char len = 0;
    char* data = NULL;
    int i = 0;

    while(1) {
        do {
            unsigned char status = SPI_STATUS_OK;
            pthread_mutex_lock(&spi_mutex);
	    read_status(fd); /* dummy read */
            status = read_status(fd);
            DEBUG_PRINT("read status = 0x%x\n", status);
            if(status & (SPI_STATUS_OK) &&
              (!(status & SPI_STATUS_7688_READ_FROM_STM32_E))) {
                break;
            }
            pthread_mutex_unlock(&spi_mutex);
            usleep(SPI_MCU_CHECK_STATUS_DELAY_US);
        }while(1);
        {
            read_len(fd);  /* dummy read */
            len = read_len(fd);
            DEBUG_PRINT("read len = %d\n", len);
            if(0 == len) {
                fprintf(stderr, "read length is 0.\n");
	        goto OUT;
            }
        }
        if(NULL == (data = (char*)malloc(len*sizeof(char)))) {
            fprintf(stderr, "read data malloc error!\n");
	    goto OUT;
        }
        read_ch(fd); /* dummy read */
        for(i=0; i<len; i++) {
            data[i] = read_ch(fd);
            DEBUG_PRINT("read data[%d] = 0x%x %c\n", i, data[i] , data[i]);
        }
        pthread_mutex_unlock(&spi_mutex);
        printf("%s",data);
        free(data);
        data = NULL;
    }

OUT:
    pthread_mutex_unlock(&spi_mutex);
    pthread_exit(NULL);
}

static void* read_stdin_handler(void* arg)
{
    int fd = *(int*)arg;
    unsigned char len = 0;
    unsigned char status = SPI_STATUS_OK;
    char* data = NULL;
    int i = 0;

    data = (char*)malloc(SPI_FRAME_MAX_LEN*sizeof(char));
    if(NULL == data) {
        fprintf(stderr, "failed to malloc spi write buffer.\n");
        pthread_exit(NULL);
    }
    while(1) {
        char* data_in = NULL;
        data_in = fgets(data, SPI_FRAME_MAX_LEN, stdin);
        if(NULL != data_in) {
            if(strncmp(data_in, "exit", 4) == 0) {
                pthread_cancel(read_mcu_tidp);
                pthread_exit(NULL);
            }
            do {
            	pthread_mutex_lock(&spi_mutex);
                read_status(fd); /* dummy read */
                status = read_status(fd);
                DEBUG_PRINT("write status = 0x%x\n", status);
                if((status & SPI_STATUS_OK) &&
                  (!(status & SPI_STATUS_7688_WRITE_TO_STM32_F))) {
                    break;
                }
            	pthread_mutex_unlock(&spi_mutex);
                usleep(SPI_MCU_CHECK_STATUS_DELAY_US);
            } while(1);
            len = strlen(data_in);
            if(0 == len) {
                fprintf(stderr, "write length is 0.\n");
		goto OUT;
            }
            put_len(fd, len);
            for(i=0; i<len; i++) {
                put_ch(fd, data_in[i]);
                DEBUG_PRINT("write data[%d] = 0x%x %c\n", i, data_in[i] , data_in[i]);
            }
            pthread_mutex_unlock(&spi_mutex);
        }
        else {
            fprintf(stderr, "fgets error.\n");
        }
        data_in = NULL;
        len = 0;
    }

OUT:
    pthread_mutex_unlock(&spi_mutex);
    pthread_exit(NULL);
}

static void spi_console_exit(int sig)
{
    DEBUG_PRINT("Get SIGINT.\n");
    pthread_cancel(read_mcu_tidp);
    pthread_cancel(read_stdin_tidp);
}

int main(int argc, char* argv[])
{
    int chk_match, size, fd;
    int is_force = 0;
    unsigned char buf;

    if(argc == 1) {
        fd = open("/dev/spiS0", O_RDONLY);
        if (fd <= 0) {
            fprintf(stderr, "Please insmod module spi_drv.o!\n");
            return -1;
        }
        DEBUG_PRINT("Start SPI console.\n");
        signal(SIGINT, spi_console_exit);
        if(pthread_mutex_init(&spi_mutex, NULL) < 0) {
            fprintf(stderr, "Init thread mutex error.\n");
            return -1;
        }
        if(pthread_create(&read_mcu_tidp, NULL, read_mcu_handler, &fd) < 0) {
            fprintf(stderr, "Create read mcu thread failed.\n");
            return -1;
        }
        if(pthread_create(&read_stdin_tidp, NULL, read_stdin_handler, &fd) < 0) {
            fprintf(stderr, "Create write mcu thread failed.\n");
            return -1;
        }

        /* wait SIGINT signal to exit */
        pthread_join(read_mcu_tidp, NULL);
        pthread_join(read_stdin_tidp, NULL);
        printf("SPI console exit.\n");
        close(fd);
        return 0;
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
                    read_status(fd); /* dummy read */
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
                        read_status(fd); /* dummy read */
                        status = read_status(fd);
                        DEBUG_PRINT("read status = 0x%x\n", status);
                        if(status & (SPI_STATUS_OK) &&
                          (!(status & SPI_STATUS_7688_READ_FROM_STM32_E))) {
                            break;
                        }
                        usleep(SPI_MCU_CHECK_STATUS_DELAY_US);
                    }while(1);
                }
                read_len(fd); /* dummy read */
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
                read_ch(fd); /* dummy read */
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
                read_status(fd); /* dummy read */
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
                    read_status(fd); /* dummy read */
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
                        read_status(fd); /* dummy read */
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
                put_len(fd, len+1); //add \n for msh
                for(i=0; i<len; i++) {
                    put_ch(fd, data[i]);
                    DEBUG_PRINT("write data[%d] = 0x%x %c\n", i, data[i] , data[i]);
                }
		put_ch(fd, '\n');
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
