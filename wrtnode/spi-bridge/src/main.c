#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <linux/spi/spidev.h>

#ifdef DEBUG
#define DEBUG_PRINT   printf
#else
#define DEBUG_PRINT(...)
#endif

#define SPI_DEVICE     "/dev/spidev0.1"
#define SPI_MODE       SPI_MODE_0
#define SPI_WORD_LEN   8
#define SPI_HZ         1000000

/************  SPI Bridge  **************/
#define SPI_BRIDGE_MAX_DATA_LEN         (1024)
#define SPI_BRIDGE_ONE_FRAME_MAX_LEN    (255)
#define SPI_BRIDGE_READ_RETRY_TIMES     (5)
#define SPI_BRIDGE_WRITE_RETRY_TIMES    (5)
#define SPI_BRIDGE_RESP_RETRY_TIMES     (20)

struct spi_bridge {
	int fd;

	/* for console */
	pthread_t read_mcu_tidp;
	pthread_t read_stdin_tidp;
	pthread_mutex_t spi_mutex;

#define SPI_BRIDGE_STATUS_7688_READ_FROM_STM32_E    (1<<0)
#define SPI_BRIDGE_STATUS_7688_READ_FROM_STM32_NE   (0<<0)
#define SPI_BRIDGE_STATUS_7688_WRITE_TO_STM32_F     (1<<1)
#define SPI_BRIDGE_STATUS_7688_WRITE_TO_STM32_NF    (0<<1)
#define SPI_BRIDGE_STATUS_SET_PARAMETER_ERR         (1<<2)
#define SPI_BRIDGE_STATUS_SET_PARAMETER_OK          (0<<2)
#define SPI_BRIDGE_STATUS_CHECK_ERR                 (1<<3)
#define SPI_BRIDGE_STATUS_CHECK_OK                  (0<<3)
#define SPI_BRIDGE_STATUS_OK                        (0x50)
#define SPI_BRIDGE_STATUS_HEAD_MASK                 (0xf0)
#define SPI_BRIDGE_STATUS_ERR_MASK                  (0x0f)
#define SPI_BRIDGE_STATUS_NULL                      (0x00)
	uint8_t status;

#define SPI_BRIDGE_CMD_GET_STATUS                   (1U)
#define SPI_BRIDGE_CMD_7688_READ_FROM_STM32         (10U)
#define SPI_BRIDGE_CMD_7688_WRITE_TO_STM32          (20U)
#define SPI_BRIDGE_CMD_SET_BLOCK_LEN                (30U)
#define SPI_BRIDGE_CMD_NULL                         (0U)
	uint8_t cmd;

	/* block length */
#define SPI_BRIDGE_LEN_8_BYTES                      (8U)
#define SPI_BRIDGE_LEN_16_BYTES                     (16U)
#define SPI_BRIDGE_LEN_32_BYTES                     (32U)
	uint8_t len;
	/* in block length count */
	uint8_t count;
	uint8_t* xfet_buf;
};

#define RT2880_SPI_READ_STR    "read"
#define RT2880_SPI_WRITE_STR   "write"
#define RT2880_SPI_STATUS_STR  "status"

#define RT2880_SPI_READ     (2)
#define RT2880_SPI_STATUS   (2)
#define RT2880_SPI_WRITE    (3)

#define SPI_MCU_READ_DELAY_US          (100000U)
#define SPI_MCU_WRITE_DELAY_US         (200U)
#define SPI_MCU_RESP_DELAY_US          (50000U)
#define SPI_MCU_CHECK_STATUS_DELAY_US  (100000U)

static char usage[] =	"spi-bridge [read/write/status] WRTnode2r stm32 data\n"
						"  spi-bridge read \n"
						"  spi-bridge write [string]\n"
						"  spi-bridge status \n"
						"  NOTE -- read/write/status value are in string\n"
						"spi-bridge\n  Open spiconsole.\n";

static struct spi_bridge spi_bridge;

static inline bool _is_spi_bridge_status_head_ok(uint8_t status)
{
	return (status & SPI_BRIDGE_STATUS_HEAD_MASK) == SPI_BRIDGE_STATUS_OK;
}

static inline bool _can_spi_bridge_status_read(uint8_t status)
{
	return !(status & SPI_BRIDGE_STATUS_7688_READ_FROM_STM32_E);
}

static inline bool _can_spi_bridge_status_write(uint8_t status)
{
	return !(status & SPI_BRIDGE_STATUS_7688_WRITE_TO_STM32_F);
}

static inline bool _is_spi_bridge_status_set_ok(uint8_t status)
{
	return !(status & SPI_BRIDGE_STATUS_SET_PARAMETER_ERR);
}

static inline bool _is_spi_bridge_status_check_ok(uint8_t status)
{
	return !(status & SPI_BRIDGE_STATUS_CHECK_ERR);
}

static inline bool _is_spi_bridge_status_ok(uint8_t status)
{
	return !(status & SPI_BRIDGE_STATUS_ERR_MASK);
}

static inline uint8_t spi_bridge_calulate_check(char* data)
{
	int i;
	uint8_t chk = 0;
	uint8_t* ptr = data;
	for (i = spi_bridge.len; i > 0; i--) {
		chk ^= *ptr++;
	}
	return chk;
}

static uint8_t spi_bridge_send_cmd(const uint8_t cmd, bool retry)
{
	struct spi_ioc_transfer msg[2];
	int i = 1;
	uint8_t resp;

	memset(msg, 0, 2 * sizeof(struct spi_ioc_transfer));

	msg[0].tx_buf = (__u32)&cmd;
	msg[0].len = 1;
	msg[0].delay_usecs = SPI_MCU_RESP_DELAY_US;

	msg[1].rx_buf = (__u32)&resp;
	msg[1].len = 1;
	msg[1].cs_change = 1;

	if (ioctl(spi_bridge.fd, SPI_IOC_MESSAGE(2), msg) < 0) {
		fprintf(stderr, "spi bridge send cmd first send error.\n");
		return SPI_BRIDGE_STATUS_NULL;
	}

	if (_is_spi_bridge_status_head_ok(resp))
		return resp;

	if (!retry) {
		fprintf(stderr, "spi bridge send cmd error.\n");
		return SPI_BRIDGE_STATUS_NULL;
	}

	do {
		DEBUG_PRINT("spi bridge send cmd retry %d.\n", i);
		if (ioctl(spi_bridge.fd, SPI_IOC_MESSAGE(1), &msg[1]) < 0) {
			fprintf(stderr, "spi bridge send cmd retry %d error.\n", i);
			return SPI_BRIDGE_STATUS_NULL;
		}
		if (_is_spi_bridge_status_head_ok(resp))
			return resp;
	} while (i++ <= SPI_BRIDGE_RESP_RETRY_TIMES);

	fprintf(stderr, "spi bridge send cmd error.\n");
	return SPI_BRIDGE_STATUS_NULL;
}

static uint8_t spi_bridge_read_one_frame(void* data, bool retry)
{
	struct spi_ioc_transfer msg[2];
	char buf[spi_bridge.len + 1];
	int i = 1;
	uint8_t resp;

	memset(msg, 0, 2 * sizeof(struct spi_ioc_transfer));

	msg[0].rx_buf = (__u32)buf;
	msg[0].len = spi_bridge.len + 1;
	msg[0].delay_usecs = SPI_MCU_RESP_DELAY_US;

	msg[1].rx_buf = (__u32)&resp;
	msg[1].len = 1;
	msg[1].cs_change = 1;

	if (ioctl(spi_bridge.fd, SPI_IOC_MESSAGE(2), msg) < 0) {
		fprintf(stderr, "spi bridge read one frame first send error.\n");
		return SPI_BRIDGE_STATUS_NULL;
	}

	if (buf[spi_bridge.len] != spi_bridge_calulate_check(buf)) {
		fprintf(stderr, "spi bridge read one frame data check error.\n");
		return SPI_BRIDGE_STATUS_NULL;
	}

	if (_is_spi_bridge_status_head_ok(resp)) {
		memcpy(data, buf, spi_bridge.len);
		return resp;
	}

	if (!retry) {
		fprintf(stderr, "spi bridge read one frame error.\n");
		return SPI_BRIDGE_STATUS_NULL;
	}

	do {
		DEBUG_PRINT("spi bridge read one frame retry %d.\n", i);
		if (ioctl(spi_bridge.fd, SPI_IOC_MESSAGE(1), &msg[1]) < 0) {
			fprintf(stderr, "spi bridge read one frame retry %d error.\n", i);
			return SPI_BRIDGE_STATUS_NULL;
		}
		if (_is_spi_bridge_status_head_ok(resp)) {
			memcpy(data, buf, spi_bridge.len);
			return resp;
		}
	} while (i++ <= SPI_BRIDGE_RESP_RETRY_TIMES);

	fprintf(stderr, "spi bridge read one frame error.\n");
	return SPI_BRIDGE_STATUS_NULL;
}

static uint8_t spi_bridge_write_one_frame(const void* data, bool retry)
{
	struct spi_ioc_transfer msg[2];
	char buf[spi_bridge.len + 1];
	int i = 1;
	uint8_t resp;

	memset(msg, 0, 2 * sizeof(struct spi_ioc_transfer));
	memcpy(buf, data, spi_bridge.len);
	buf[spi_bridge.len] = spi_bridge_calulate_check(buf);

	msg[0].tx_buf = (__u32)buf;
	msg[0].len = spi_bridge.len + 1;
	msg[0].delay_usecs = SPI_MCU_RESP_DELAY_US;

	msg[1].rx_buf = (__u32)&resp;
	msg[1].len = 1;
	msg[1].cs_change = 1;

	if (ioctl(spi_bridge.fd, SPI_IOC_MESSAGE(2), msg) < 0) {
		fprintf(stderr, "spi bridge write one frame first send error.\n");
		return SPI_BRIDGE_STATUS_NULL;
	}

	if (_is_spi_bridge_status_head_ok(resp))
		return resp;

	if (!retry) {
		fprintf(stderr, "spi bridge write one frame error.\n");
		return SPI_BRIDGE_STATUS_NULL;
	}

	do {
		DEBUG_PRINT("spi bridge write one frame retry %d.\n", i);
		if (ioctl(spi_bridge.fd, SPI_IOC_MESSAGE(1), &msg[1]) < 0) {
			fprintf(stderr, "spi bridge write one frame retry %d error.\n", i);
			return SPI_BRIDGE_STATUS_NULL;
		}
		if (_is_spi_bridge_status_head_ok(resp))
			return resp;
	} while (i++ <= SPI_BRIDGE_RESP_RETRY_TIMES);

	fprintf(stderr, "spi bridge write one frame error.\n");
	return SPI_BRIDGE_STATUS_NULL;
}

static inline uint8_t spi_bridge_read_status(void)
{
	spi_bridge_send_cmd(SPI_BRIDGE_CMD_GET_STATUS, true);
}

static ssize_t spi_bridge_write(const void* data, size_t len)
{
	int writen = len;
	int i = 0;
	char buf[spi_bridge.len];
	const char* ptr = data;
	uint8_t status;

	status = spi_bridge_send_cmd(SPI_BRIDGE_CMD_7688_WRITE_TO_STM32, true);
	if (status == SPI_BRIDGE_STATUS_NULL) {
		fprintf(stderr, "spi bridge write send cmd error.\n");
		return 0;
	}

	while (writen && (i < SPI_BRIDGE_WRITE_RETRY_TIMES)) {
		if (!_can_spi_bridge_status_write(status)) {
			fprintf(stderr, "spi bridge write stm32 full.\n");
			return len - writen;
		}

		if (writen < spi_bridge.len) {
			memset(buf, 0, spi_bridge.len);
			memcpy(buf, data, writen);
			ptr = buf;
		}

		status = spi_bridge_write_one_frame(ptr, false);
		if (status == SPI_BRIDGE_STATUS_NULL) {
			fprintf(stderr, "spi bridge write write one frame error.\n");
			return len - writen;
		}
		if (!_is_spi_bridge_status_check_ok(status)) {
			i++;
			DEBUG_PRINT("spi bridge write retry %d.\n", i);
			continue;
		}
		writen -= (writen < spi_bridge.len) ? writen : spi_bridge.len;
		ptr += (writen < spi_bridge.len) ? 0 : spi_bridge.len;
		i = 0;
	}

	if (SPI_BRIDGE_WRITE_RETRY_TIMES == i)
		fprintf(stderr, "spi bridge write error.\n");

	return len - writen;
}

static ssize_t spi_bridge_read(void* data, size_t len)
{
	int readn = len;
	int i = 0;
	char buf[spi_bridge.len];
	char* ptr = data;
	uint8_t status;

	status = spi_bridge_send_cmd(SPI_BRIDGE_CMD_7688_READ_FROM_STM32, true);
	if (status == SPI_BRIDGE_STATUS_NULL) {
		fprintf(stderr, "spi bridge read send cmd error.\n");
		return 0;
	}

	while (readn && (i < SPI_BRIDGE_READ_RETRY_TIMES)) {
		if (!_can_spi_bridge_status_read(status)) {
			fprintf(stderr, "spi bridge read stm32 empty.\n");
			return len - readn;
		}

		status = spi_bridge_read_one_frame(buf, false);
		if (status == SPI_BRIDGE_STATUS_NULL) {
			fprintf(stderr, "spi bridge read read one frame error.\n");
			return len - readn;
		}
		if (!_is_spi_bridge_status_check_ok(status)) {
			i++;
			DEBUG_PRINT("spi bridge read retry %d.\n", i);
			continue;
		}

		memcpy(ptr, buf, (readn < spi_bridge.len) ? readn : spi_bridge.len);
		readn -= (readn < spi_bridge.len) ? readn : spi_bridge.len;
		ptr += (readn < spi_bridge.len) ? 0 : spi_bridge.len;
		i = 0;
	}

	if (SPI_BRIDGE_READ_RETRY_TIMES == i)
		fprintf(stderr, "spi bridge read error.\n");

	return len - readn;
}

static void* read_mcu_handler(void* arg)
{
	char* buf;

	buf = (char*)malloc(SPI_BRIDGE_MAX_DATA_LEN * sizeof(char));
	if (NULL == buf) {
		fprintf(stderr, "spi bridge failed to malloc spi read buffer.\n");
		pthread_exit(NULL);
	}

	while (1) {
		char* data_in;
		uint8_t status;
		ssize_t len;

		data_in = buf;
		do {
			pthread_mutex_lock(&spi_bridge.spi_mutex);
			status = spi_bridge_read_status();
			DEBUG_PRINT("read_mcu_handler read status = 0x%x\n", status);
			if (_is_spi_bridge_status_head_ok(status)
				&& _can_spi_bridge_status_read(status)) {
				break;
			}
			pthread_mutex_unlock(&spi_bridge.spi_mutex);
			usleep(SPI_MCU_CHECK_STATUS_DELAY_US);
		} while (1);

		len = spi_bridge_read(data_in, SPI_BRIDGE_MAX_DATA_LEN);
		pthread_mutex_unlock(&spi_bridge.spi_mutex);
		if (len < 0) {
			fprintf(stderr, "spi bridge read failed.\n");
			break;
		}
		if (SPI_BRIDGE_MAX_DATA_LEN != len)
			data_in[len] = '\0';
		fprintf(stdout, "%s", data_in);
	}

	free(buf);
	pthread_cancel(spi_bridge.read_stdin_tidp);
	pthread_exit(NULL);
}

static void* read_stdin_handler(void* arg)
{
	char* buf;

	buf = (char*)malloc(SPI_BRIDGE_MAX_DATA_LEN * sizeof(char));
	if (NULL == buf) {
		fprintf(stderr, "spi bridge failed to malloc spi write buffer.\n");
		pthread_exit(NULL);
	}

	while (1) {
		char* data_in;
		uint8_t status;

		data_in = buf;
		data_in = fgets(data_in, SPI_BRIDGE_MAX_DATA_LEN, stdin);
		if (NULL != data_in) {
			if (strncmp(data_in, "exit", 4) == 0)
				break;
			do {
				pthread_mutex_lock(&spi_bridge.spi_mutex);
				status = spi_bridge_read_status();
				DEBUG_PRINT("read_stdin_handler read status = 0x%x\n", status);
				if (_is_spi_bridge_status_head_ok(status)
					&& _can_spi_bridge_status_write(status)) {
					break;
				}
				pthread_mutex_unlock(&spi_bridge.spi_mutex);
				usleep(SPI_MCU_CHECK_STATUS_DELAY_US);
			} while (1);
			if (spi_bridge_write(data_in, strlen(data_in)) < 0) {
				fprintf(stderr, "spi bridge write failed.\n");
				pthread_mutex_unlock(&spi_bridge.spi_mutex);
				break;
			}
			DEBUG_PRINT("spi bridge write %s\n", data_in);
			pthread_mutex_unlock(&spi_bridge.spi_mutex);
		} else {
			fprintf(stderr, "fgets error.\n");
			break;
		}
	}

	free(buf);
	pthread_cancel(spi_bridge.read_mcu_tidp);
	pthread_exit(NULL);
}

static void spi_console_exit(int sig)
{
	DEBUG_PRINT("Get SIGINT.\n");
	pthread_cancel(spi_bridge.read_mcu_tidp);
	pthread_cancel(spi_bridge.read_stdin_tidp);
}

static int open_spi_device(const char* dev)
{
	int fd;
	int mode = SPI_MODE;
	int word_len = SPI_WORD_LEN;
	int hz = SPI_HZ;

	fd = open(dev, O_RDWR);
	if (fd <= 0) {
		fprintf(stderr, "Can not open spi device(%s).\n", dev);
		return -1;
	}
	if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0) {
		fprintf(stderr, "Can not set spidev mode to %d.\n", mode);
		goto err_out;
	}
	if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &word_len) < 0) {
		fprintf(stderr, "Can not set spidev word len to %d.\n", word_len);
		goto err_out;
	}
	if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &hz) < 0) {
		fprintf(stderr, "Can not set spidev speed to %d.\n", hz);
		goto err_out;
	}
	return fd;

err_out:
	close(fd);
	return -1;
}

static void spi_bridge_init(void)
{
	memset(&spi_bridge, 0, sizeof(spi_bridge));
	spi_bridge.len = SPI_BRIDGE_LEN_16_BYTES;
}

int main(int argc, char* argv[])
{
	spi_bridge_init();

	if (argc == 1) {
		spi_bridge.fd = open_spi_device(SPI_DEVICE);
		if (spi_bridge.fd <= 0) {
			fprintf(stderr, "Can not open spi device!\n");
			return -1;
		}
		DEBUG_PRINT("Start SPI console.\n");
		signal(SIGINT, spi_console_exit);
		if (pthread_mutex_init(&spi_bridge.spi_mutex, NULL) < 0) {
			fprintf(stderr, "Init thread mutex error.\n");
			return -1;
		}
		if (pthread_create(&spi_bridge.read_mcu_tidp, NULL, read_mcu_handler, NULL) < 0) {
			fprintf(stderr, "Create read mcu thread failed.\n");
			return -1;
		}
		if (pthread_create(&spi_bridge.read_stdin_tidp, NULL, read_stdin_handler, NULL) < 0) {
			fprintf(stderr, "Create write mcu thread failed.\n");
			return -1;
		}

		/* wait SIGINT signal to exit */
		pthread_join(spi_bridge.read_mcu_tidp, NULL);
		pthread_join(spi_bridge.read_stdin_tidp, NULL);
		printf("SPI console exit.\n");
		close(spi_bridge.fd);
		return 0;
	}

	switch (argc) {
	//case RT2880_SPI_STATUS:
	case RT2880_SPI_READ:
		if (0 == strcmp(argv[1], RT2880_SPI_READ_STR)) {
			char* data;
			int len;

			spi_bridge.fd = open_spi_device(SPI_DEVICE);
			if (spi_bridge.fd <= 0) {
				fprintf(stderr, "Can not open spi device!\n");
				return -1;
			}

			do {
				spi_bridge.status = spi_bridge_read_status();
				DEBUG_PRINT("spi bridge status = 0x%x\n", spi_bridge.status);
				if (_is_spi_bridge_status_head_ok(spi_bridge.status)
					&& _can_spi_bridge_status_read(spi_bridge.status)) {
					break;
				}
				usleep(SPI_MCU_CHECK_STATUS_DELAY_US);
			} while (1);

			if (NULL == (data = (char*)malloc(SPI_BRIDGE_MAX_DATA_LEN * sizeof(char)))) {
				fprintf(stderr, "read data malloc error!\n");
				close(spi_bridge.fd);
				return -1;
			}

			len = spi_bridge_read(data, SPI_BRIDGE_MAX_DATA_LEN);
			fprintf(stdout, "%s", data);
			free(data);
			close(spi_bridge.fd);
		} else if (0 == strcmp(argv[1], RT2880_SPI_STATUS_STR)) {
			spi_bridge.fd = open_spi_device(SPI_DEVICE);
			if (spi_bridge.fd <= 0) {
				fprintf(stderr, "Can not open spi device!\n");
				return -1;
			}
			spi_bridge.status = spi_bridge_read_status();
			DEBUG_PRINT("spi bridge status = 0x%x\n", spi_bridge.status);
			if (!_is_spi_bridge_status_head_ok(spi_bridge.status)) {
				printf("spi bridge read status error.\n");
				close(spi_bridge.fd);
				return -1;
			}
			if (!_can_spi_bridge_status_read(spi_bridge.status))
				printf("Can not read. STM32 read buf empty.\n");
			if (!_can_spi_bridge_status_write(spi_bridge.status))
				printf("Can not write. STM32 write buf full.\n");
			if (!_is_spi_bridge_status_set_ok(spi_bridge.status))
				printf("Set stm32 parameter error.\n");
			if (!_is_spi_bridge_status_check_ok(spi_bridge.status))
				printf("Transfer data error.\n");
			if (_is_spi_bridge_status_ok(spi_bridge.status))
				printf("OK\n");
			close(spi_bridge.fd);
		} else {
			fprintf(stderr, "Usage:\n%s\n", usage);
			return -1;
		}
		break;
	case RT2880_SPI_WRITE:
		if (0 == strcmp(argv[1], RT2880_SPI_WRITE_STR)) {
			char* data = argv[2];

			spi_bridge.fd = open_spi_device(SPI_DEVICE);
			if (spi_bridge.fd <= 0) {
				fprintf(stderr, "Can not open spi device!\n");
				return -1;
			}

			do {
				spi_bridge.status = spi_bridge_read_status();
				DEBUG_PRINT("spi bridge status = 0x%x\n", spi_bridge.status);
				if (_is_spi_bridge_status_head_ok(spi_bridge.status)
					&& _can_spi_bridge_status_write(spi_bridge.status)) {
					break;
				}
				usleep(SPI_MCU_CHECK_STATUS_DELAY_US);
			} while (1);

			spi_bridge_write(data, strlen(data));
			DEBUG_PRINT("spi bridge write %s\n", data);
			close(spi_bridge.fd);
		} else {
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
