#ifndef __TIMEBASE_H
#define __TIMEBASE_H

#include <stdint.h>


uint32_t get_tick();
void timebase_init();
void delay(uint32_t delay);
#endif
