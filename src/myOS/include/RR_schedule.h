#ifndef	__RR_schedule_H__
#define __RR_schedule_H__
#include "task.h"
#define TIME_SLICE 100
#define TCBPOOL_SIZE 32
#define READY_QUEUE 1
#define ARRIV_QUEUE 0
#define NULL ((void *)0)
void RRqueue_init(void);
void RRSetTskPara(myTCB *tsk,int priority,int exetime,int arrtime);
void RRTcbPoolInit(int tskNum);
void RRCreateTsk(void (*tskBody)(void),int priority,int exetime,int arrtime);
void RRArrivSchedule(void);
void RRSchedule(void);
void RRTskEnd(void);
void showReadyQueue(void);
void showArrivQueue(void);
myTCB * initial_task;
#endif