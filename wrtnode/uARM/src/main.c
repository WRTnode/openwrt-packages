/*
	FileName	：main.c
	Description	：The main function of uARM_driver.
	Author		：SchumyHao
	Version		：V04
	Data		：2013.03.28
*/
/* Uncomment next line if you want to debug the code. */
#define DEBUG

/* Include files */
#include "UART.h"
#include "uARM_driver.h"

/* Define the const */
#define HOST_UART_PORT       (0)
#define ARGV_FLAG_o          (0x01)
#define ARGV_FLAG_O          (0x02)
#define ARGV_FLAG_r          (0x04)
#define ARGV_FLAG_p          (0x08)
#define IS_ARGV_FLAG(FLAG)   (((FLAG) == ARGV_FLAG_o) || \
                             ((FLAG) == ARGV_FLAG_O) || \
                             ((FLAG) == ARGV_FLAG_r) || \
                             ((FLAG) == ARGV_FLAG_p))
/* Functions Declaration*/
int ProcessArguments(int argc, char** argv, t_Coordinate* pCooSys);

/* Functions */
int main(int argc, char* argv[]){
	int BuffDeep = DEFAULT_BUFF_DEEP;
	t_Coordinate CooSys;
	t_uart UartConfig;
	struct termios OldCfg;
	char* pBuff = NULL;
	/* Initialize the coordinate */
	InitCoordinateSystem(&CooSys);
	/* Initialize the UART */
	InitUartStruct(&UartConfig);
	/* Arguments process */
	if(ProcessArguments(argc, argv, &CooSys) < 0){
		perror("Input arguments are wrong.\n");
		return -1;
	}
	if(CooSys.CooShiftEn){
		/* Change the coordinate from rectangular to polar. */
		ShiftCoordinate(&CooSys);
		#ifdef DEBUG
		printf("X location is %d.\n",CooSys.X);
		printf("Y location is %d.\n",CooSys.Y);
		printf("Angle degree is %d.\n",CooSys.Angle);
		printf("Radius length is %d.\n",CooSys.Radius);
		#endif
	}
	/* Generate the motion data */
	pBuff = (char*)malloc(sizeof(char)*BUFFER_SIZE*BUFFER_DEEP);
	if(pBuff == NULL){
		perror("Memory allocated wrong.\n");
		return -2;
	}
	BuffDeep = GenerateMotion(&CooSys, pBuff);
	if(BuffDeep < 0){
		perror("Motion generated wrong.\n");
		return -3;
	}
	#ifdef DEBUG
	printf("BuffDeep is %d.\n",BuffDeep);
	#endif
	/* Configurate the UART */
	UartConfig.Fd = OpenPort(HOST_UART_PORT);
	if(UartConfig.Fd < 0){
		perror("Can not open UART port.\n");	
		return -4;
	}
	UartConfig.pFp=fdopen(UartConfig.Fd,"w");
	if(UartConfig.pFp == NULL){
		perror("Can not open UART stream!\n");
		return -5;
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
		return -6;
	}
	/* Transmit data */
	if(SendData(UartConfig.pFp, BuffDeep, pBuff) < 0){
		perror("Send data error.\n");
		return -7;
	}
	/* Free the buff memory */
	free(pBuff);
	BuffDeep = DEFAULT_BUFF_DEEP;
	/* Restore the UART config parameters */
	if((tcsetattr(UartConfig.Fd, TCSANOW, &OldCfg)) != 0){
		perror("Revert tty error.\n");
		return -8;
	}
	close(UartConfig.Fd);
	fclose(UartConfig.pFp);
	return 0;
}

int ProcessArguments(int argc, char** argv, t_Coordinate* pCooSys){
	int i;
	int tmp;
	unsigned char Flag = 0;
	#ifdef DEBUG
	
	for(i = 0; i < argc; i++){
		printf("argv[%d]: %s\n",i,argv[i]);
	}
	#endif
	for(i = 1; i < argc; i++){
		if(*argv[i] == '-'){
			switch(*(++argv[i])){
			case 'o':
				if(Flag & ARGV_FLAG_o){
					printf("WARNING:  Got '-o' more than onec in arguments\n");
				}
				else{
					tmp = atoi(argv[++i]);
					if(IS_A_DEGREE(tmp)){
						pCooSys->Angle = tmp;
					}
					else{
						perror("Angle argument is wrong!\n");
						return -1;
					}
					tmp = atoi(argv[++i]);
					if(IS_R_LENGTH(tmp)){
						pCooSys->Radius = tmp;
					}
					else{
						perror("Radius argument is wrong!\n");
						return -1;
					}
					tmp = atoi(argv[++i]);
					if(IS_H_LOCATION(tmp)){
						pCooSys->H = tmp;
					}
					else{
						perror("Hight argument is wrong!\n");
						return -1;
					}
					pCooSys->CooShiftEn = DISABLE;
					pCooSys->DirectOutputEn = ENABLE;
					Flag |= ARGV_FLAG_o;
				}
				break;
			case 'O':
				if(Flag & ARGV_FLAG_O){
					printf("WARNING:  Got '-O' more than onec in arguments\n");
				}
				else{
					tmp = atoi(argv[++i]);
					if(IS_X_LOCATION(tmp)){
						pCooSys->X = tmp;
					}
					else{
						perror("X argument is wrong!\n");
						return -1;
					}
					tmp = atoi(argv[++i]);
					if(IS_Y_LOCATION(tmp)){
						pCooSys->Y = tmp;
					}
					else{
						perror("Y argument is wrong!\n");
						return -1;
					}
					tmp = atoi(argv[++i]);
					if(IS_H_LOCATION(tmp)){
						pCooSys->H = tmp;
					}
					else{
						perror("Hight argument is wrong!\n");
						return -1;
					}
					pCooSys->CooShiftEn = ENABLE;
					pCooSys->DirectOutputEn = ENABLE;
					Flag |= ARGV_FLAG_O;
				}
				break;
			case 'r':
				if(Flag & ARGV_FLAG_r){
					printf("WARNING:  Got '-r' more than onec in arguments\n");
				}
				else{
					tmp = atoi(argv[++i]);
					if(IS_X_LOCATION(tmp)){
						pCooSys->X = tmp;
					}
					else{
						perror("X argument is wrong!\n");
						return -1;
					}
					tmp = atoi(argv[++i]);
					if(IS_Y_LOCATION(tmp)){
						pCooSys->Y = tmp;
					}
					else{
						perror("Y argument is wrong!\n");
						return -1;
					}
					tmp = atoi(argv[++i]);
					if(IS_H_LOCATION(tmp)){
						pCooSys->H = tmp;
					}
					else{
						perror("Hight argument is wrong!\n");
						return -1;
					}
					tmp = atoi(argv[++i]);
					if(IS_DESTINATION(tmp)){
						pCooSys->Dest = tmp;
					}
					else{
						perror("Destination argument is wrong!\n");
						return -1;
					}
					pCooSys->CooShiftEn = ENABLE;
					pCooSys->DirectOutputEn = DISABLE;
					Flag |= ARGV_FLAG_r;
				}
				break;
			case 'p':
				if(Flag & ARGV_FLAG_p){
					printf("WARNING:  Got '-p' more than onec in arguments\n");
				}
				else{
					tmp = atoi(argv[++i]);
					if(IS_A_DEGREE(tmp)){
						pCooSys->Angle = tmp;
					}
					else{
						perror("Angle argument is wrong!\n");
						return -1;
					}
					tmp = atoi(argv[++i]);
					if(IS_R_LENGTH(tmp)){
						pCooSys->Radius = tmp;
					}
					else{
						perror("Radius argument is wrong!\n");
						return -1;
					}
					tmp = atoi(argv[++i]);
					if(IS_H_LOCATION(tmp)){
						pCooSys->H = tmp;
					}
					else{
						perror("Hight argument is wrong!\n");
						return -1;
					}
					tmp = atoi(argv[++i]);
					if(IS_DESTINATION(tmp)){
						pCooSys->Dest = tmp;
					}
					else{
						perror("Destination argument is wrong!\n");
						return -1;
					}
					pCooSys->CooShiftEn = DISABLE;
					pCooSys->DirectOutputEn = DISABLE;
					Flag |= ARGV_FLAG_p;
				}
				break;
			default:
				printf("WARNING:  Got redundant arguments\n");
			}
		}
	}
	if(IS_ARGV_FLAG(Flag)){
		return 0;
	}
	else{
		perror("Arguments are wrong!\n");
		return -1;
	}
}
