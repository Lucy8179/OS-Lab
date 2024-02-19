#ifndef __PRIO_SCHEDULE_H__
#define __PRIO_SCHEDULE_H__
#include "task.h"
typedef struct queue{
	myTCB * head;	
	myTCB * tail;
	int queueType;
}Prioqueue ;
Prioqueue Prio_Ready_queue,Prio_Arriv_queue;
myTCB *initTsk;
void Prioqueue_init(void);
void PrioshowReadyQueue(void);
void PrioshowArrivQueue(void);
void PrioTcbPoolInit(int tskNum);
void PrioCreateTsk(void (*tskBody)(void),int priority,int exetime,int arrtime);
void PrioSchedule();
void PrioArrivSchedule(void);
void PrioTskEnd(void);
#endif