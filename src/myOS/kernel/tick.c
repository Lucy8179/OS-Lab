extern void oneTickUpdateWallClock(void);       //TODO: to be generalized

void (*tick_hook)(void) = 0;

unsigned long tick_number = 0;

#define MAX_FUNC 16

void (*func_list[MAX_FUNC])(void);

int func_num=0;

void append_funclist(void(*func)(void)){
	func_list[func_num++]=func;
}

void clear_funclist(void){
	func_num=0;
}

void tick(void){
	tick_number++;

	oneTickUpdateWallClock();

	if(tick_hook) tick_hook();  //user defined   

	//每次时钟中断会把中断函数表里的函数执行一遍
	//这样可以使得每次tick中断能够运行调度函数
	int i=0;
	for(i=0;i<func_num;i++){
		func_list[i]();
	}

}
