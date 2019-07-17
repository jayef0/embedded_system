#ifndef MOD_COMMS_H
#define MOD_COMMS_H

#include <stdint.h>

void module_comms_start_received(void);
void module_comms_heartbeat(void);
void module_comms_start(void);
void module_comms_stop(void);

//added for new thread
void module_comms_init(void); //added change (to initialise thread)
void module_comms_task(void *argument);
void module_comms_update(void);

#endif
