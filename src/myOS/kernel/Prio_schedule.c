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

#define TIME_SLICE 10
#define TCBPOOL_SIZE 32
#define READY_QUEUE 1
#define ARRIV_QUEUE 0


typedef struct queue{
	myTCB * head;	
	myTCB * tail;
	int queueType;
}Prioqueue ;

Prioqueue Prio_Ready_queue,Prio_Arriv_queue;
myTCB *initTsk;

void Prioqueue_init(void){
	//两个都初始化
	Prio_Ready_queue.head=NULL;
	Prio_Ready_queue.tail=NULL;
	Prio_Ready_queue.queueType=READY_QUEUE;
	Prio_Arriv_queue.head=NULL;
	Prio_Arriv_queue.tail=NULL;
	Prio_Arriv_queue.queueType=ARRIV_QUEUE;
	return;
}

void PrioSetTskPara(myTCB *tsk,int priority,int exetime,int arrtime){
	tsk->TSkPara.priority=priority;//most important
	tsk->TSkPara.exeTime=exetime;
	tsk->TSkPara.arrTime=arrtime;
	return;
}

void PrioshowReadyQueue(void){
	myTCB *p=Prio_Ready_queue.head;
	myPrintk(0x8,"showReadyQueue:\n");
	while(p!=NULL){
		myPrintk(0x8,"ReadyQueue: prio:%d exe:%d arr:%d \n",p->TSkPara.priority,p->TSkPara.exeTime,p->TSkPara.arrTime);
		p=p->nextTCB;
	}
	return;
}

void PrioshowArrivQueue(void){
	myTCB *p=Prio_Arriv_queue.head;
	myPrintk(0x8,"showArrivQueue:\n");
	while(p!=NULL){
		myPrintk(0x8,"ArrivQueue: prio:%d exe:%d arr:%d \n",p->TSkPara.priority,p->TSkPara.exeTime,p->TSkPara.arrTime);
		p=p->arriv_nextTCB;
	}
	return;
}

int Prioqueue_empty(Prioqueue *queue){
	if(queue->head==NULL&&queue->tail==NULL)
		return TRUE;
	else
		return FALSE;
}

myTCB * NextPrioTsk(Prioqueue *queue){
	return queue->head;
}

void PrioEnqueue(Prioqueue *queue,myTCB *newtsk){
	myTCB *p;
	int priority,arrtime;
	if(queue->queueType==READY_QUEUE){
		//showArrivQueue();
		//按照优先级顺序入队
		if(Prioqueue_empty(queue)){
			queue->head=newtsk;
			queue->tail=newtsk;
			newtsk->nextTCB=NULL;
			newtsk->TSK_State=TSK_WAIT;
			return;
		}
		else{
			p=queue->head;
			priority=newtsk->TSkPara.priority;
			//myPrintk(0x7, "arrtime=%d\n", arrtime);
			if(priority<=p->TSkPara.priority){
				//前插
				//myPrintk(0x7,"forwarding:%d\n",newtsk->TSK_ID);
				newtsk->nextTCB=p;
				queue->head=newtsk;
				return;
			}
			else if(queue->head==queue->tail){
				//后插
				//myPrintk(0x7,"backing:%d\n",newtsk->TSK_ID);
				p->nextTCB=newtsk;
				queue->tail=newtsk;
				newtsk->nextTCB=NULL;
			}
			else{
				while((p!=queue->tail)&&((p->nextTCB)->TSkPara.priority<=priority))
					p=p->nextTCB;
				if(p==queue->tail){
					queue->tail=newtsk;
					newtsk->nextTCB=NULL;
				}
				newtsk->nextTCB=p->nextTCB;
				p->nextTCB=newtsk;
				return;
			}
		}		
	}
	else{
		//showArrivQueue();
		//按照到达时间顺序入队
		if(Prioqueue_empty(queue)){
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
}

myTCB* PrioDequeue(Prioqueue *queue){
	myTCB *old_head;
	if(queue->queueType==READY_QUEUE){
		if(Prioqueue_empty(queue))
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
		if(Prioqueue_empty(queue))
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

myTCB PrioTcbPool[TCBPOOL_SIZE];
myTCB * PriofirstFreeTsk=NULL;

void PrioTcbPoolInit(int tskNum){
	// 初始化进程池（所有的进程状态都是TSK_NONE）
    int i;
    myTCB * thisTCB;
    for(i=0;i<tskNum;i++){//对进程池tcbPool中的进程进行初始化处理
          thisTCB = &PrioTcbPool[i];
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
	 PriofirstFreeTsk=PrioTcbPool;
}

void PrioCreateTsk(void (*tskBody)(void),int priority,int exetime,int arrtime){
	//Prio_Arriv_queue.queueType=0;
	if(PriofirstFreeTsk==NULL){
		//任务池中没有空闲TCB
		//myPrintk(0x8,"fail");
		return;
	}
	else{
		myTCB *new_task=PriofirstFreeTsk;
		PriofirstFreeTsk=PrioTcbPool+new_task->TSK_ID+1;
		//myPrintk(0x7,"ID:%d",PriofirstFreeTsk->TSK_ID);
		new_task->task_entrance=tskBody;
		stack_init(&new_task->stkTop,tskBody);
		new_task->TSK_State=TSK_RDY;
		//myPrintk(0x8,"win\n");
		PrioSetTskPara(new_task,priority,exetime,arrtime);
		PrioEnqueue(&Prio_Arriv_queue,new_task);//先加入arriv queue
		//showArrivQueue();
		return;
	}
}

void PrioSchedule(){
	//非抢占式调度
	//如果arriv queue无内容则返回shell
	disable_interrupt();
	myTCB *nextTsk;
	if(currentTsk->TSK_State==TSK_DONE){
		//任务完成才调度，非抢占
		nextTsk=PrioDequeue(&Prio_Ready_queue);
		if(nextTsk==NULL){
			if(Prioqueue_empty(&Prio_Arriv_queue)){
				//myPrintk(0x7,"yes");
				clear_funclist();
				context_switch(currentTsk,initTsk);
			}
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
		enable_interrupt();
		return;
	}
}

void PrioArrivSchedule(void){
	//需加进func_list
	disable_interrupt();
	myTCB * arrivTsk;
	arrivTsk=NextPrioTsk(&Prio_Arriv_queue);
	if(arrivTsk==NULL){
		enable_interrupt();
		return;
	}
	else{
		while((arrivTsk!=NULL)&&(arrivTsk->TSkPara.arrTime<=tick_number)){
			arrivTsk=PrioDequeue(&Prio_Arriv_queue);
			PrioEnqueue(&Prio_Ready_queue, arrivTsk);
			arrivTsk=arrivTsk->arriv_nextTCB;
		}
	}
	enable_interrupt();
}



void PrioTskEnd(void){
	currentTsk->TSK_State=TSK_DONE;
	PrioSchedule();
}






