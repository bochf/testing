#ifndef WMALARM_H
#define WMALARM_H

#include "../providers.h"
#include "../configfile.h"

void *InitAlarmWM(struct proot *p, struct config *cfg);

int WriteAlarmWM(void *data_handle);

int FinishAlarmWM(void *data_handle);


#endif
