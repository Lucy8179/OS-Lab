#include "../myOS/userInterface.h"
#include "shell.h"
#include "../myOS/include/RR_schedule.h"
#include "../myOS/include/tick.h"
#include "../myOS/include/task.h"
#include "../myOS/include/irq.h"
//#include <cstddef>
#define NULL ((void *)0)
#define TSK_DONE 3
void RRTsk1(void){
	unsigned long i,j,k;
	for(i=0;i<50;i++){
		myPrintf(0x5," tsk1:%d",i);
		for(j=0;j<500000;j++){
			k+=j*j;
		}//delay
	}
	RRTskEnd();
}
void RRTsk2(void){
	unsigned long i,j,k;
	for(i=0;i<26;i++){
		myPrintf(0x7," tsk2:%c",'a'+i);
		for(j=0;j<500000;j++){
			k+=j*j;
		}//delay
	}
	RRTskEnd();
}
void RRTsk3(void){
	unsigned long i,j,k;
	for(i=0;i<26;i++){
		myPrintf(0x8," tsk3:%c",'A'+i);
		for(j=0;j<500000;j++){
			k+=j*j;
		}//delay
	}
	RRTskEnd();
}
int RRScheduleTest(int argc, unsigned char **argv){
	disable_interrupt();
	RRqueue_init();
	RRTcbPoolInit(5);
	RRCreateTsk(RRTsk1,0,1,1);
	RRCreateTsk(RRTsk2,0,2,2);
	RRCreateTsk(RRTsk3,0,3,3);
	append_funclist(RRArrivSchedule);
	append_funclist(RRSchedule);
	tick_number=0;
	showArrivQueue();
	initial_task=nextFCFSTsk();
	currentTsk->TSK_State=TSK_DONE;
	enable_interrupt();
	return 0;
}
void RR_ScheduleTestInit(void){
	addNewCmd("RRTest",RRScheduleTest,NULL,"To have RR schedule Test");
}
