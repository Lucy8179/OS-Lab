#include "../include/task.h"
#include "../include/tick.h"
#include "../include/irq.h"
#include "../include/myPrintk.h"

#define TSK_RDY 0        //表示当前进程已经进入就绪队列中
#define TSK_WAIT -1      //表示当前进程还未进入就绪队列中
#define TSK_RUNING 1     //表示当前进程正在运行
#define TSK_NONE 2       //表示进程池中的TCB为空未进行分配
#define TSK_DONE 3
#define NULL ((void *)0)
#define TRUE 1
#define FALSE 0
#define NO_Free -1

#define TIME_SLICE 6
#define TCBPOOL_SIZE 32
#define READY_QUEUE 1
#define ARRIV_QUEUE 0


typedef struct queue{
	myTCB * head;	
	myTCB * tail;
	int queueType;
}RRqueue ;

RRqueue RR_Ready_queue,RR_Arriv_queue;

void RRqueue_init(void){
	//两个都初始化
	RR_Ready_queue.head=NULL;
	RR_Ready_queue.tail=NULL;
	RR_Ready_queue.queueType=READY_QUEUE;
	RR_Arriv_queue.head=NULL;
	RR_Arriv_queue.tail=NULL;
	RR_Arriv_queue.queueType=ARRIV_QUEUE;
	return;
}

void RRSetTskPara(myTCB *tsk,int priority,int exetime,int arrtime){
	tsk->TSkPara.priority=priority;
	tsk->TSkPara.exeTime=exetime;
	tsk->TSkPara.arrTime=arrtime;
	return;
}

void showReadyQueue(void){
	myTCB *p=RR_Ready_queue.head;
	myPrintk(0x8,"showReadyQueue:\n");
	while(p!=NULL){
		myPrintk(0x8,"ReadyQueue: prio:%d exe:%d arr:%d \n",p->TSkPara.priority,p->TSkPara.exeTime,p->TSkPara.arrTime);
		p=p->nextTCB;
	}
	return;
}

void showArrivQueue(void){
	myTCB *p=RR_Arriv_queue.head;
	myPrintk(0x8,"showArrivQueue:\n");
	while(p!=NULL){
		myPrintk(0x8,"ArrivQueue: prio:%d exe:%d arr:%d \n",p->TSkPara.priority,p->TSkPara.exeTime,p->TSkPara.arrTime);
		p=p->arriv_nextTCB;
	}
	return;
}

int RRqueue_empty(RRqueue *queue){
	if(queue->head==NULL&&queue->tail==NULL)
		return TRUE;
	else
		return FALSE;
}

myTCB * NextRRTsk(RRqueue *queue){
	return queue->head;
}

void RREnqueue(RRqueue *queue,myTCB *newtsk){
	myTCB *p;
	int arrtime;
	if(queue->queueType==READY_QUEUE){
		if(RRqueue_empty(queue)){
			queue->head=newtsk;
			queue->tail=newtsk;
			newtsk->nextTCB=NULL;
			newtsk->TSK_State=TSK_RDY;
			return;
		}
		else{
			queue->tail->nextTCB=newtsk;
			queue->tail=newtsk;
			newtsk->nextTCB=NULL;
			newtsk->TSK_State=TSK_RDY;
			return;
		}
	}
	else{
		//showArrivQueue();
		//按照到达时间顺序入队
		if(RRqueue_empty(queue)){
			queue->head=newtsk;
			queue->tail=newtsk;
			newtsk->arriv_nextTCB=NULL;
			newtsk->TSK_State=TSK_WAIT;
			return;
		}
		else{
			p=queue->head;
			arrtime=newtsk->TSkPara.arrTime;
			//myPrintk(0x7, "arrtime=%d\n", arrtime);
			if(arrtime<=p->TSkPara.arrTime){
				//前插
				//myPrintk(0x7,"forwarding:%d\n",newtsk->TSK_ID);
				newtsk->arriv_nextTCB=p;
				queue->head=newtsk;
				return;
			}
			else if(queue->head==queue->tail){
				//后插
				//myPrintk(0x7,"backing:%d\n",newtsk->TSK_ID);
				p->arriv_nextTCB=newtsk;
				queue->tail=newtsk;
				newtsk->arriv_nextTCB=NULL;
			}
			else{
				while((p!=queue->tail)&&((p->arriv_nextTCB)->TSkPara.arrTime<=arrtime))
					p=p->arriv_nextTCB;
				if(p==queue->tail){
					queue->tail=newtsk;
					newtsk->arriv_nextTCB=NULL;
				}
				newtsk->arriv_nextTCB=p->arriv_nextTCB;
				p->arriv_nextTCB=newtsk;
				return;
			}
		}		
	}
//	myPrintk(0x8,"insert\n");
	// if(RRqueue_empty(queue)){
	// 	queue->head=newtsk;
	// 	queue->tail=newtsk;
	// 	newtsk->arriv_nextTCB=NULL;
	// 	newtsk->TSK_State=TSK_RDY;
	// 	return;
	// }
	// else{
	// 	queue->tail->arriv_nextTCB=newtsk;
	// 	queue->tail=newtsk;
	// 	newtsk->arriv_nextTCB=NULL;
	// 	newtsk->TSK_State=TSK_RDY;
	// 	return;
	// }

}

myTCB* RRDequeue(RRqueue *queue){
	myTCB *old_head;
	if(queue->queueType==READY_QUEUE){
		if(RRqueue_empty(queue))
			return NULL;
		else{
			old_head=queue->head;
			if(old_head==queue->tail){
				queue->head=NULL;
				queue->tail=NULL;
			}
			else{
				queue->head=old_head->nextTCB;
			}
			return old_head;
		}
	}
	else{
		if(RRqueue_empty(queue))
			return NULL;
		else{
			old_head=queue->head;
			if(old_head==queue->tail){
				queue->head=NULL;
				queue->tail=NULL;
			}
			else{
				queue->head=old_head->arriv_nextTCB;
			}
			return old_head;
		}
	}
}

myTCB RRTcbPool[TCBPOOL_SIZE];
myTCB * RRfirstFreeTsk=NULL;
myTCB * initial_task;

void RRTcbPoolInit(int tskNum){
	// 初始化进程池（所有的进程状态都是TSK_NONE）
    int i;
    myTCB * thisTCB;
    for(i=0;i<tskNum;i++){//对进程池tcbPool中的进程进行初始化处理
          thisTCB = &RRTcbPool[i];
          thisTCB->TSK_ID = i;
          thisTCB->stkTop = thisTCB->stack+STACK_SIZE-1;//将栈顶指针复位
          thisTCB->TSK_State = TSK_NONE;//表示该进程池未分配，可用
          thisTCB->task_entrance = NULL;
          if(i==tskNum-1){
               thisTCB->nextTCB = (void *)0;
			   thisTCB->arriv_nextTCB=(void *)0;
          }
          else{
               thisTCB->nextTCB = &tcbPool[i+1];
			   thisTCB->arriv_nextTCB = &tcbPool[i+1];
			   
          }
     }
	 RRfirstFreeTsk=RRTcbPool;
}

void RRCreateTsk(void (*tskBody)(void),int priority,int exetime,int arrtime){
	//RR_Arriv_queue.queueType=0;
	if(RRfirstFreeTsk==NULL){
		//任务池中没有空闲TCB
		//myPrintk(0x8,"fail");
		return;
	}
	else{
		myTCB *new_task=RRfirstFreeTsk;
		RRfirstFreeTsk=RRTcbPool+new_task->TSK_ID+1;
		//myPrintk(0x7,"ID:%d",RRfirstFreeTsk->TSK_ID);
		new_task->task_entrance=tskBody;
		stack_init(&new_task->stkTop,tskBody);
		new_task->TSK_State=TSK_RDY;
		//myPrintk(0x8,"win\n");
		RRSetTskPara(new_task,priority,exetime,arrtime);
		RREnqueue(&RR_Arriv_queue,new_task);//先加入arriv queue
		//showArrivQueue();
		return;
	}
}

void RRArrivSchedule(void){
	//需加进func_list
	disable_interrupt();
	myTCB * arrivTsk;
	arrivTsk=NextRRTsk(&RR_Arriv_queue);
	if(arrivTsk==NULL){
		enable_interrupt();
		return;
	}
	else{
		while((arrivTsk!=NULL)&&(arrivTsk->TSkPara.arrTime<=tick_number)){
			arrivTsk=RRDequeue(&RR_Arriv_queue);
			RREnqueue(&RR_Ready_queue, arrivTsk);
			arrivTsk=arrivTsk->arriv_nextTCB;
		}
	}
	enable_interrupt();
}


void RRSchedule(void){
	//需加进func_list
	disable_interrupt();
	myTCB* nextTsk;
	//myPrintk(0x7, "succeed\n");
	if(tick_number%TIME_SLICE==0){
		//时间片已到
		//showReadyQueue();
		tick_number++;
		//myPrintk(0x7, "succeed\n");
		if(currentTsk->TSK_State==TSK_DONE){
			//如果已经完成现在的任务
			//则无需将其入队
			nextTsk = RRDequeue(&RR_Ready_queue);
			if(nextTsk==NULL){
				enable_interrupt();
				return;
			}
			else{
				context_switch(currentTsk,nextTsk);
				enable_interrupt();
				return;
			}
		}
		else{
			//没有完成任务
			currentTsk->TSK_State=TSK_WAIT;
			RREnqueue(&RR_Ready_queue,currentTsk);
			nextTsk = NextRRTsk(&RR_Ready_queue);
			//myPrintk(0x9,"yes\n");
			if(nextTsk==NULL){
				enable_interrupt();
				return;
			}
			else{
				//myPrintk(0x9,"yes\n");
				//showReadyQueue();
				RRDequeue(&RR_Ready_queue);
				if(currentTsk==nextTsk){
					enable_interrupt();
					return;
				}
				else
					context_switch(currentTsk,nextTsk);
				return;
			}
		}
	}
	else if(currentTsk->TSK_State==TSK_DONE){
		//showReadyQueue();
		//myPrintk(0x7,"\nFinish!\n");
		nextTsk = RRDequeue(&RR_Ready_queue);
		if(nextTsk==NULL){
			//myPrintk(0x7,"\nWin!\n");
			if(RRqueue_empty(&RR_Arriv_queue)){
			 	clear_funclist();			
				context_switch(currentTsk,initial_task);
			}
			enable_interrupt();
			return;
		}
		else{
			//myPrintk(0x7,"\nFail!\n");
			context_switch(currentTsk,nextTsk);
			enable_interrupt();
			return;
		}
	}
}

void RRTskEnd(void){
	currentTsk->TSK_State=TSK_DONE;
	RRSchedule();
}




