/*
    FileName    :uARM_driver.c
    Description :Source code of uARM driver
    Author      :SchumyHao
    Email       :schumy.haojl@gmail.com
    Version     :V10
    Data        :2014.04.07
*/
/* Uncomment next line if you want to debug the code. */
//#define DEBUG

/* Include files */
#include "uARM_driver.h"

/* Functions */
/* Control data transmit function */
int SendData(FILE* const pFp, int const BuffDeep, const char* pBuff)
{
    int i,j;

    for(j=0; j<BuffDeep; j++) {
#ifdef DEBUG
        printf("%4d	",j+1);
#endif

        for(i=0; i<BUFFER_SIZE; i++) {
#ifdef DEBUG
            printf("0x%x ",pBuff[j*BUFFER_SIZE+i]);
#endif

            if(fputc(pBuff[j*BUFFER_SIZE+i], pFp) == -1) {
                break;
            }
        }

        if(i < BUFFER_SIZE) {
            perror("Tramsmit data incomplete.\n");
            return -1;
        }

#ifdef DEBUG
        printf("\n");
#endif
        usleep(FRAME_DELAY_TIME);
    }

    return 0;
}

/* Coordinate initialize function */
int InitCoordinateSystem(t_Coordinate* pCooSys)
{
    pCooSys->CooShiftEn = DISABLE;
    pCooSys->DirectOutputEn = DISABLE;
    pCooSys->X = DEFAULT_X_LOCATION;
    pCooSys->Y = DEFAULT_Y_LOCATION;
    pCooSys->H = DEFAULT_H_LOCATION;
    pCooSys->Angle = DEFAULT_A_DEGREE;
    pCooSys->Radius = DEFAULT_R_LENGTH;
    pCooSys->Dest = DEFAULT_DEST;
    return 0;
}

/* Change the coordinate from rectangular to polar */
int ShiftCoordinate(t_Coordinate* pCooSys)
{
    assert(IS_X_LOCATION(pCooSys->X));
    assert(IS_Y_LOCATION(pCooSys->Y));
    int DistX = pCooSys->X - UARM_X_LOCATION;
    int DistY = pCooSys->Y - UARM_Y_LOCATION;
    pCooSys->Radius = (int)sqrt((double)((DistX*DistX)+(DistY*DistY))) - \
                      (int)sqrt((double)((UARM_X_LOCATION*UARM_X_LOCATION)+(UARM_Y_LOCATION*UARM_Y_LOCATION)));
    pCooSys->Angle = (int)((DistY)?(2*RAD2ANG(atan2((double)DistX,(double)DistY))): \
                           ((DistX>0)?MAX_A_DEGREE:((DistX<0)?MIN_A_DEGREE:DEFAULT_A_DEGREE)));
    pCooSys->CooShiftEn = DISABLE;
    return 0;
}

/* Motion data Generation function */
int GenerateMotion(t_Coordinate* pCooSys, char* pBuff)
{
    assert(IS_A_DEGREE(pCooSys->Angle));
    assert(IS_R_LENGTH(pCooSys->Radius));
    assert(IS_H_LOCATION(pCooSys->H));
    assert(IS_DESTINATION(pCooSys->Dest));
    t_Move Move;
    int BuffDeep = 0;
    int i;

    if(pCooSys->DirectOutputEn) {
        /* Move to input A&R */
        *pBuff++ = FRAME_HEADER_H;
        *pBuff++ = FRAME_HEADER_L;
        *pBuff++ = HI_BYTE(pCooSys->Angle);
        *pBuff++ = LO_BYTE(pCooSys->Angle);
        *pBuff++ = HI_BYTE(pCooSys->Radius);
        *pBuff++ = LO_BYTE(pCooSys->Radius);
        *pBuff++ = MOTION_NONE;
        BuffDeep++;
        /* Set uArm to input hight */
        Move.DestAngle = pCooSys->Angle;
        Move.DestRadius = pCooSys->Radius;
        Move.DestHight = pCooSys->H;
        Move.CurrAngle = pCooSys->Angle;
        Move.CurrRadius = pCooSys->Radius;
        Move.CurrHight = 0;
        BuffDeep = MoveArm(&Move, pBuff, BuffDeep);
    }
    else {
        /* Go to pick up the coin */
        Move.DestAngle = pCooSys->Angle;
        Move.DestRadius = pCooSys->Radius;
        Move.DestHight = pCooSys->H;
        Move.CurrAngle = DEFAULT_A_DEGREE;
        Move.CurrRadius = DEFAULT_R_LENGTH;
        Move.CurrHight = DEFAULT_H_LOCATION;
        BuffDeep = MoveArm(&Move, (pBuff+(BuffDeep*BUFFER_SIZE)), BuffDeep);

        /* Pick up the coin */
        for(i=PICK_RETRY_TIMES; i>0; i--) {
            BuffDeep = HandleArm(MOTION_PICK, \
                                 (pBuff+(BuffDeep*BUFFER_SIZE)), BuffDeep);
        }

        /* Go to coin collection place */
        switch(pCooSys->Dest) {
            case DEST_ONE:
                Move.DestAngle = DEST_ONE_A;
                Move.DestRadius = DEST_ONE_R;
                Move.DestHight = DEST_ONE_H;
                break;
            case DEST_FIVE:
                Move.DestAngle = DEST_FIVE_A;
                Move.DestRadius = DEST_FIVE_R;
                Move.DestHight = DEST_FIVE_H;
                break;
            case DEST_USER:
                Move.DestAngle = DEST_USER_A;
                Move.DestRadius = DEST_USER_R;
                Move.DestHight = DEST_USER_H;
                break;
            case DEFAULT_DEST:
            default:
                Move.DestAngle = DEFAULT_A_DEGREE;
                Move.DestRadius = DEFAULT_R_LENGTH;
                Move.DestHight = DEFAULT_H_LOCATION;
        }

        Move.CurrAngle = pCooSys->Angle;
        Move.CurrRadius = pCooSys->Radius;
        Move.CurrHight = pCooSys->H;
        BuffDeep = MoveArm(&Move, (pBuff+(BuffDeep*BUFFER_SIZE)), BuffDeep);
        /* Put down the coin */
        BuffDeep = HandleArm(MOTION_RELEASE, \
                             (pBuff+(BuffDeep*BUFFER_SIZE)), BuffDeep);

        /* Go to initial place */
        switch(pCooSys->Dest) {
            case DEST_ONE:
                Move.CurrAngle = DEST_ONE_A;
                Move.CurrRadius = DEST_ONE_R;
                Move.CurrHight = DEST_ONE_H;
                break;
            case DEST_FIVE:
                Move.CurrAngle = DEST_FIVE_A;
                Move.CurrRadius = DEST_FIVE_R;
                Move.CurrHight = DEST_FIVE_H;
                break;
            case DEST_USER:
                Move.CurrAngle = DEST_USER_A;
                Move.CurrRadius = DEST_USER_R;
                Move.CurrHight = DEST_USER_H;
                break;
            case DEFAULT_DEST:
            default:
                Move.CurrAngle = DEFAULT_A_DEGREE;
                Move.CurrRadius = DEFAULT_R_LENGTH;
                Move.CurrHight = DEFAULT_H_LOCATION;
        }

        Move.DestAngle = DEFAULT_A_DEGREE;
        Move.DestRadius = DEFAULT_R_LENGTH;
        Move.DestHight = DEFAULT_H_LOCATION;
        BuffDeep = MoveArm(&Move, (pBuff+(BuffDeep*BUFFER_SIZE)), BuffDeep);
    }

    if(BuffDeep <= BUFFER_DEEP) {
        return BuffDeep;
    }
    else {
        perror("Buffer is overflowed.\n");
        return -1;
    }
}

/*  Arm move function */
int MoveArm(t_Move* pMotion, char* pBuff, int BuffDeep)
{
    assert(IS_A_DEGREE(pMotion->DestAngle));
    assert(IS_R_LENGTH(pMotion->DestRadius));
    assert(IS_H_LOCATION(pMotion->DestHight));
    assert(IS_A_DEGREE(pMotion->CurrAngle));
    assert(IS_R_LENGTH(pMotion->CurrRadius));
    assert(IS_H_LOCATION(pMotion->CurrHight));
#ifdef GO_WITH_LINE
    int TempA,TempR,TempH;
    int MaxLoop;
    int StepA,StepR,StepH;
    int i;
    //Caculate Angle、Radius、Hight distance
    TempA = pMotion->DestAngle - pMotion->CurrAngle;
    TempR = pMotion->DestRadius - pMotion->CurrRadius;
    TempH = ((pMotion->CurrHight - pMotion->DestHight) > COIN_THICKNESS)? \
            (pMotion->DestHight - pMotion->CurrHight + COIN_THICKNESS): \
            (pMotion->DestHight - pMotion->CurrHight);
    //Caculate max loop cycle number
    MaxLoop = MAX2(abs(TempA),(MAX2(abs(TempR),abs(TempH))));
    //Caculate step
    StepA = abs((TempA)?(MaxLoop/TempA):MaxLoop);
    StepR = abs((TempR)?(MaxLoop/TempR):MaxLoop);
    StepH = abs((TempH)?(MaxLoop/TempH):MaxLoop);
#ifdef DEBUG
    printf("Current Angle is %d.\n",pMotion->CurrAngle);
    printf("Current Radius is %d.\n",pMotion->CurrRadius);
    printf("Current Hight is %d.\n",pMotion->CurrHight);
    printf("Destination Angle is %d.\n",pMotion->DestAngle);
    printf("Destination Radius is %d.\n",pMotion->DestRadius);
    printf("Destination Hight is %d.\n",pMotion->DestHight);
    printf("TempA is DestA-CurrA = %d.\n",TempA);
    printf("TempR is DestR-CurrR = %d.\n",TempR);
    printf("TempH is DestH-CurrH = %d.\n",TempH);
    printf("MaxLoop is %d.\n",MaxLoop);
    printf("StepA is %d.\n",StepA);
    printf("StepR is %d.\n",StepR);
    printf("StepH is %d.\n",StepH);
#endif

    for(i=MaxLoop; i>0; i--) {
        pMotion->CurrAngle += ((i%StepA) || (!TempA))? \
                              0:(SINGNAL(TempA));
        pMotion->CurrRadius += ((i%StepR) || (!TempR))? \
                               0:(SINGNAL(TempR));
        pMotion->CurrHight += ((i%StepH) || (!TempH))? \
                              0:(SINGNAL(TempH));
        *pBuff++ = FRAME_HEADER_H;
        *pBuff++ = FRAME_HEADER_L;
        *pBuff++ = HI_BYTE(pMotion->CurrAngle);
        *pBuff++ = LO_BYTE(pMotion->CurrAngle);
        *pBuff++ = HI_BYTE(pMotion->CurrRadius);
        *pBuff++ = LO_BYTE(pMotion->CurrRadius);
        *pBuff++ = (TempH<0)?MOTION_H_DOWN: \
                   ((TempH>0)?MOTION_H_UP:MOTION_NONE);
        TempA -= ((i%StepA)||(!TempA))?0:(SINGNAL(TempA));
        TempR -= ((i%StepR)||(!TempR))?0:(SINGNAL(TempR));
        TempH -= ((i%StepH)||(!TempH))?0:(SINGNAL(TempH));
        BuffDeep++;
    }

    if((pMotion->CurrHight - pMotion->DestHight) == COIN_THICKNESS) {
        for(i=COIN_THICKNESS; i>0; i--) {
            *pBuff++ = FRAME_HEADER_H;
            *pBuff++ = FRAME_HEADER_L;
            *pBuff++ = HI_BYTE(pMotion->CurrAngle);
            *pBuff++ = LO_BYTE(pMotion->CurrAngle);
            *pBuff++ = HI_BYTE(pMotion->CurrRadius);
            *pBuff++ = LO_BYTE(pMotion->CurrRadius);
            *pBuff++ = MOTION_H_DOWN;
            BuffDeep++;
        }
    }

#endif
#ifdef GO_WITH_PARABOLA
    //TODO:go twith parabola.
#endif
    return BuffDeep;
}

/* Arm handle function */
int HandleArm(const unsigned char Motion, char* pBuff, int BuffDeep)
{
#ifdef DEBUG
    printf("Current motion is %d.\n",Motion);
#endif

    if(pBuff-BUFFER_SIZE > 0) {
        *pBuff++ = *(pBuff-BUFFER_SIZE);
        *pBuff++ = *(pBuff-BUFFER_SIZE);
        *pBuff++ = *(pBuff-BUFFER_SIZE);
        *pBuff++ = *(pBuff-BUFFER_SIZE);
        *pBuff++ = *(pBuff-BUFFER_SIZE);
        *pBuff++ = *(pBuff-BUFFER_SIZE);
        *pBuff++ = Motion;
        BuffDeep++;
    }
    else {
        perror("Please move the ARM first.\n");
    }

    return BuffDeep;
}
