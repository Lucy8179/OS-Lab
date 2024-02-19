#ifndef __TICK_H__
#define __TICK_H__

unsigned long tick_number;
void tick(void);
void append_funclist(void(*func)(void));
void clear_funclist(void);
#endif