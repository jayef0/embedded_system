#ifndef MOD_INPUTS_H
#define MOD_INPUTS_H

#include <stdint.h>

void module_inputs_init(void);
void module_inputs_update(void);
void module_inputs_task(void *argument);
void module_inputs_configure_hardware(void);
void module_inputs_start(void);
void module_inputs_stop(void);

#endif