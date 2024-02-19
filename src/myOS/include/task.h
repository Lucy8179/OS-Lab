#ifndef __TASK_H__
#define __TASK_H__

#ifndef USER_TASK_NUM
#include "../../userApp/userApp.h"
#endif

#define TASK_NUM (2 + USER_TASK_NUM)   // at least: 0-idle, 1-init

#define initTskBody myMain         // connect initTask with myMain

#define STACK_SIZE 512            // definition of STACK_SIZE

void initTskBody(void);

void CTX_SW(void*prev_stkTop, void*next_stkTop);

#define Priority_Schedule (1)
#define RR_Schedule (2)

typedef struct tskPara{
	unsigned int priority;
	unsigned int exeTime;
	unsigned int arrTime;
	// unsigned int schedPolicy;
} tskPara;


//#error "TODO: 为 myTCB 增补合适的字段"
typedef struct myTCB {
     unsigned long *stkTop;        /* 栈顶指针 */
     unsigned long stack[STACK_SIZE];      /* 开辟了一个大小为STACK_SIZE的栈空间 */  
     unsigned long TSK_State;   /* 进程状态 */
     unsigned long TSK_ID;      /* 进程ID */ 
     void (*task_entrance)(void);  /*进程的入口地址*/
     struct myTCB * nextTCB;           /*下一个TCB*/
	 struct myTCB * arriv_nextTCB;
	 tskPara TSkPara;
} myTCB;

myTCB tcbPool[TASK_NUM];//进程池的大小设置

myTCB * idleTsk;                /* idle 任务 */
myTCB * currentTsk;             /* 当前任务 */
myTCB * firstFreeTsk;           /* 下一个空闲的 TCB */

void TaskManagerInit(void);
void stack_init(unsigned long **stk, void (*task)(void));
unsigned long **prevTSK_StackPtr;
unsigned long *nextTSK_StackPtr;
void context_switch(myTCB *prevTsk, myTCB *nextTsk);
unsigned long BspContextBase[STACK_SIZE];
unsigned long *BspContext;
myTCB * nextFCFSTsk(void);

// typedef struct scheduler {
// 	unsigned long type; // the type of the scheduler
// 	int preemptive_or_not; //if True, the scheduler is preemptive
// 	myTCB* (*nextTsk_func)(void);
// 	void (*enqueueTsk_func)(myTCB *tsk);
// 	void (*dequeueTsk_func)(myTCB *tsk);
// 	void (*schedulerInit_func)(myTCB* idleTsk);
// 	int (*createTsk_hook)(void (*tskBody)(void),tskPara para);
// 	void (*tick_hook)(void); //if set, tick_hook will be called every tick
// }scheduler;

// scheduler Scheduler;


#endif
