#ifndef WMBCD_H
#define WMBCD_H

#include "../providers.h"
#include "../configfile.h"

/* NOTE:
   There are no structs defined here. This data is (effectively) private. The
   framework only sees a void pointer, not the struct data.
*/

void *InitBCDWM(struct proot *p, struct config *cfg);

int WriteBCDWM(void *data_handle);

int FinishBCDWM(void *data_handle);


#endif
