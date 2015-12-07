#ifndef WMFILE_H
#define WMFILE_H

#include "../providers.h"
#include "../configfile.h"

/* NOTE:
   There are no structs defined here. This data is (effectively) private. The
   framework only sees a void pointer, not the struct data.
*/

void *InitFileWM(struct proot *p, struct config *cfg);

int WriteFileWM(void *data_handle);

int FinishFileWM(void *data_handle);


#endif
