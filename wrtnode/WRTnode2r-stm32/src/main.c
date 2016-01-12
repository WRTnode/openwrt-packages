#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

char usage[] =  "WRTnode2r-stm32 <command> [path]\n" \
		" command: \n" \
		"    write filename : upgrade stm32 firmware\n" \
		"    read filename : read stm32 firmware to bin file\n" \
		"    restart : restart stm32\n";

unsigned long GetFileSize(const char *filename)
{
	struct stat buf;
	if(stat(filename,&buf)<0)
	{
		return 0;
	}
	return (unsigned long)buf.st_size;
}

int main(int argc,char* argv[]){
	int len = 0 ;
	int i,fdbin,r_size,ret;
	int a = 0;
	char rbuf[100] = {0};
	char data[100] = {0};
	char cmd = 'w';
	unsigned char chk = 0;
	unsigned long addr;
	static char readcmd[10] = {0};
	static char startcmd[10] = {0};
	unsigned long filesize,scale;
	unsigned long finish_size = 0;
	if(argc < 2 || (strcmp(argv[1],"write") != 0 & strcmp(argv[1],"read") != 0 & strcmp(argv[1],"restart") != 0)){
		fprintf(stderr, "Usage:\n%s\n", usage);
		return -1;
	}
	else if(strcmp(argv[1],"write") == 0 || strcmp(argv[1],"read") == 0){
		fdbin = open(argv[2],O_RDONLY);
		if(fdbin < 0){
			printf("open fail!");
			return -1;
		}
		if(strcmp(argv[1],"write")== 0){
			strcpy(readcmd,"read");
			strcpy(startcmd,"restart");
		}
		else
			strcpy(readcmd,"read");
	}
	else
		strcpy(startcmd,"restart");

	if(strcmp(argv[1],"write") == 0){
		printf("write ...\n");
		cmd = 'w';
		addr = 0x1000;
		lseek( fdbin, 0, SEEK_SET );
		filesize = GetFileSize(argv[2]);
		while(1){
			r_size = read(fdbin,rbuf,64);
			if(r_size < 0){
				printf("read fail!");
				return -1;
			}
			if(r_size == 0){
				printf("write end!\n");
				break;
			}
			data[0] = 0x48;
			data[1] = 0x59;
			data[2] = 0x3c;

			data[3] = (int)(r_size + 2);
			data[4] = cmd;

			for(i=5; i<=7; i++){
				data[i] = (addr >> ((7-i)*8) & 0xff);
			}
			chk = 0;
			memcpy(data + 8,rbuf,r_size);
			for(i=4;i<=7;i++){
				chk ^=data[i];
			}
			i = r_size;
			while(i--) {
				chk ^= (unsigned char)rbuf[i];
			}
			data[r_size + 8] = chk;

			ret = WRTnode2r_spi_write(data, r_size + 9, 0);
			if(ret < 0){
				printf("write fail!\n");
			}
			usleep(100);
			ret = WRTnode2r_spi_read(data, 0);
			if(ret < 0){
				printf("read fail\n");
			}
			memset(rbuf,0,100);
			memset(data,0,100);
			addr += r_size;
			a++;
			if(a%5 == 0)
				printf("#");
			fflush(stdout);
		}
	}

	if(strcmp(readcmd,"read") == 0){
		printf("check ...\n");
		addr = 0x1000;
		cmd = 'r';
		lseek( fdbin, 0, SEEK_SET );
		while(1){
			cmd = 'r';
			data[0] = 0x48;
			data[1] = 0x59;
			data[2] = 0x3c;
			data[3] = 2;
			data[4] = cmd;
			for(i=5; i<=7; i++){
				data[i] = (addr >> ((7-i)*8) & 0xff);
			}
			chk = 0;
			for(i=4;i<=7;i++){
				chk ^=data[i];
			}
			data[8] = chk;
			r_size = read(fdbin,rbuf,64);
			if(r_size < 0){
				printf("read fail!");
				return -1;
			}
			if(r_size == 0){
				printf("check end!\n");
				break;
			}

			ret = WRTnode2r_spi_write(data,9,0);
			ret = WRTnode2r_spi_read(data,0);
			if(ret < 0){
				printf("read fail\n");
			}
			if(strncmp(rbuf,data+8,r_size) == 0){
				//printf("64byte data check ok!\n");
			}
			addr += r_size;
			memset(data,0,100);
			memset(rbuf,0,100);

			a++;
			if(a%5 == 0)
				printf("#");
			fflush(stdout);

		}
	}
	if(strcmp(startcmd,"restart") == 0){
		printf("restart ...\n");
		cmd = 'O';
		data[0] = 0x48;
		data[1] = 0x59;
		data[2] = 0x3c;
		data[3] = 0x0;
		data[4] = 'O';
		data[5] = 'O';
		WRTnode2r_spi_write(data,6,0);
		printf("restart end\n");
	}
	return 0;
}
