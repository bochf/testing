#ifndef PSHELP_H
#define PSHELP_H

#include <stdlib.h>

/* Temp - Debug - for MAXCOMLEN */
#include <procinfo.h>

/* I am wrapping this so that it will be portable to other platforms */
struct pid_data
{
   pid_t pid;
   uid_t uid;


   char            pi_comm[MAXCOMLEN+1];   /* (truncated) program name */
};

struct filter_on
{
   pid_t pid;
   uid_t uid;


};

/* ========================================================================= */
struct filter_on *CreateFilter(void);

/* ========================================================================= */
int FilterReset(struct filter_on *f);

/* ========================================================================= */
int FilterOnUID(struct filter_on *f, uid_t uid);

/* ========================================================================= */
int FilterOnUName(struct filter_on *f, char *uname);

/* ========================================================================= */
int GetPIDData(struct pid_data *p, struct filter_on *f);


#endif
