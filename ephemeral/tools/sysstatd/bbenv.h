#ifndef BBENV_H
#define BBENV_H

#include "coredata.h"

long Get_bb(char *status, unsigned long size);
long Get_bb_bin(char *status, unsigned long size);
long Get_bb_sys(char *status, unsigned long size);
long Get_bb_data(char *status, unsigned long size);
long Get_bb_pm(char *status, unsigned long size);
long Get_bb_bigmem(char *status, unsigned long size);
long Get_bb_cores(char *status, unsigned long size);
long Get_bb_util(char *status, unsigned long size);
long Get_bb_admin(char *status, unsigned long size);


long Get_ethdv_dta(char *status, unsigned long size);
long Get_comdbg64(char *status, unsigned long size);
long Get_timserv(char *status, unsigned long size);
long Get_bbpy(char *status, unsigned long size);
long Get_varldb(char *status, unsigned long size);
long Get_rpml(char *status, unsigned long size);
long Get_console_fifo(char *status, unsigned long size);
long Get_bypass_file(char *status, unsigned long size);


/* This is a "fake" data item that binds to the top of the list. It
   *gathers* data, it does not print data. */
long EnumerateMatches(char *status, unsigned long size);

/* Lump all registrations into a single function. These registrations are
   not as simple as others (they are multiple step registrations). */
struct dataitem *RegisterBBProcItems(void);

#endif
