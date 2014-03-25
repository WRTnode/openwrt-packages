/*
	FileName	：main.c
	Description	：The main function of uARM_driver.
	Author		：SchumyHao
	Version		：V03
	Data		：2013.03.25
*/
/* Uncomment next line if you want to debug the code. */
#define DEBUG

/* Include files */
#include "UART.h"
#include "uARM_driver.h"

/* Define the const */
#define HOST_UART_PORT 	(0)
#define ARGC_MUN	(5)

/* Functions */
int main(int argc, char* argv[]){
	int tmp;
	int BuffDeep = DEFAULT_BUFF_DEEP;
	int Dest = DEFAULT_DEST;	//find one or five?
	t_Coordinate CooSys;
	t_uart UartConfig;
	struct termios OldCfg;
	char* pBuff = NULL;
	/* Initialize the coordinate */
	InitCoordinateSystem(&CooSys);
	/* Initialize the UART */
	InitUartStruct(&UartConfig);
	/* Check the input arguments */
	#ifdef DEBUG
	int i;
	for(i = 0; i < argc; i++){
		printf("argv[%d]: %s\n",i,argv[i]);
	}
	#endif
	if(argc == ARGC_MUN){
		tmp = atoi(argv[1]);
		if(IS_X_LOCATION(tmp)){
			CooSys.X = tmp;
		}
		else{
			perror("1st argument 'x' is wrong!\n");
			return -1;
		}
		tmp = atoi(argv[2]);
		if(IS_Y_LOCATION(tmp)){
			CooSys.Y = tmp;
		}
		else{
			perror("2nd argument 'y' is wrong!\n");
			return -1;
		}
		tmp = atoi(argv[3]);
		if(IS_H_LOCATION(tmp)){
			CooSys.H = tmp;
		}
		else{
			perror("3rd argument 'h' is wrong!\n");
			return -1;
		}
		tmp = atoi(argv[4]);
		if(IS_DESTINATION(tmp)){
			Dest = tmp;
		}
		else{
			perror("4th argument 'destination' is wrong!\n");
			return -1;
		}
	}
	else{
		perror("Arguments number is wrong!\n");
		return -1;
	}

	/* Change the coordinate from rectangular to polar. */
	ShiftCoordinate(&CooSys);
	#ifdef DEBUG
	printf("X location is %d.\n",CooSys.X);
	printf("Y location is %d.\n",CooSys.Y);
	printf("Angle degree is %d.\n",CooSys.Angle);
	printf("Radius length is %d.\n",CooSys.Radius);
	#endif
	/* Generate the motion data */
	pBuff = (char*)malloc(sizeof(char)*BUFFER_SIZE*BUFFER_DEEP);
	if(pBuff == NULL){
		perror("Memory allocated wrong.\n");
		return -1;
	}
	BuffDeep = GenerateMotion(&CooSys, Dest, pBuff);
	if(BuffDeep == DEFAULT_BUFF_DEEP){
		perror("Motion generated wrong.\n");
		return -1;
	}
	#ifdef DEBUG
	printf("BuffDeep is %d.\n",BuffDeep);
	#endif
	/* Configurate the UART */
	UartConfig.Fd = OpenPort(HOST_UART_PORT);
	if(UartConfig.Fd < 0){
		perror("Can not open UART port.\n");	
		return -1;
	}
	UartConfig.pFp=fdopen(UartConfig.Fd,"w");
	if(UartConfig.pFp == NULL){
		perror("Can not open UART stream!\n");
		return -1;
	}
	//Change the buffer type to none.
	setbuf(UartConfig.pFp, NULL);
	#ifdef DEBUG
	printf("UART device fd is %d.\n",UartConfig.Fd);
	#endif
	UartConfig.BaudRate = BAUD_RATE_9600;
	UartConfig.DataBits = DATA_BITS_8BITS;
	UartConfig.Parity = PARITY_NONE;
	UartConfig.StopBits = STOP_BITS_1BIT;		
	if(ConfigUart(&UartConfig, &OldCfg) < 0){
		perror("Config UART error.\n");
		return -1;
	}
	/* Transmit data */
	if(SendData(UartConfig.pFp, BuffDeep, pBuff) < 0){
		perror("Send data error.\n");
		return -1;
	}
	/* Free the buff memory */
	free(pBuff);
	BuffDeep = DEFAULT_BUFF_DEEP;
	/* Restore the UART config parameters */
	if((tcsetattr(UartConfig.Fd, TCSANOW, &OldCfg)) != 0){
		perror("Revert tty error.\n");
		return -1;
	}
	close(UartConfig.Fd);
	fclose(UartConfig.pFp);
	return 0;
}

