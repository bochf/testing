#ifndef LINBASIC_H
#define LINBASIC_H

int InitBasic(void);
int RefreshBasic(void);

/* The following are gettors for "Basic" data. */
long GetUptimeSecs(char *status, unsigned long size);
long GetProcCount(char *status, unsigned long size);


#endif
