#include "../myOS/userInterface.h"
#include "shell.h"
#include "../myOS/include/Prio_schedule.h"
#include "../myOS/include/tick.h"
#include "../myOS/include/task.h"
#include "../myOS/include/irq.h"
//#include <cstddef>
#define NULL ((void *)0)
#define TSK_DONE 3
void PrioTsk1(void){
	unsigned long i,j,k;
	for(i=0;i<50;i++){
		myPrintf(0x5," tsk1:%d",i);
		for(j=0;j<500000;j++){
			k+=j*j;
		}//delay
	}
	PrioTskEnd();
}
void PrioTsk2(void){
	unsigned long i,j,k;
	for(i=0;i<26;i++){
		myPrintf(0x7," tsk2:%c",'a'+i);
		for(j=0;j<500000;j++){
			k+=j*j;
			k-=j*j;
		}//delay
	}
	PrioTskEnd();
}
void PrioTsk3(void){
	unsigned long i,j,k;
	for(i=0;i<26;i++){
		myPrintf(0x8," tsk3:%c",'A'+i);
		for(j=0;j<500000;j++){
			k+=j*j;
			k-=j*j;
		}//delay
	}
	PrioTskEnd();
}
void PrioTsk4(void){
	unsigned long i,j,k;
	for(i=50;i<100;i++){
		myPrintf(0x4," tsk4:%d",i);
		for(j=0;j<500000;j++){
			k+=j*j;
		}//delay
	}
	PrioTskEnd();
}
int PrioScheduleTest(int argc, unsigned char **argv){
	disable_interrupt();
	Prioqueue_init();
	PrioTcbPoolInit(5);
	PrioCreateTsk(PrioTsk1,3,3,0);
	PrioCreateTsk(PrioTsk2,2,3,9);
	PrioCreateTsk(PrioTsk3,1,1,9);
	PrioCreateTsk(PrioTsk4,0,1,9);
	append_funclist(PrioArrivSchedule);
	append_funclist(PrioSchedule);
	tick_number=1;
	PrioshowArrivQueue();
	initTsk=nextFCFSTsk();
	currentTsk->TSK_State=TSK_DONE;
	enable_interrupt();
	//PrioshowReadyQueue();
	//PrioSchedule();
	return 0;
}
void Prio_ScheduleTestInit(void){
	addNewCmd("PrioTest",PrioScheduleTest,NULL,"To have Prio schedule Test");
}
