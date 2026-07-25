#ifndef TIMER_H
#define TIMER_H

void init_timer(unsigned int frequency);
void timer_handler(void);

extern unsigned int system_ticks;

#endif
