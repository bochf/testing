#ifndef WMSTDOUT_H
#define WMSTDOUT_H

#include "../providers.h"
#include "../configfile.h"

/* NOTE:
   There are no structs defined here. This data is (effectively) private. The
   framework only sees a void pointer, not the struct data.
*/

void *InitStdoutWM(struct proot *p, struct config *cfg);

int StdoutWMWrite(void *data_handle);

int StdoutWMFinish(void *data_handle);


#endif
