#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <procinfo.h>
#include <sys/types.h>
#include <errno.h>

#include "pshelp.h"
#include "pwhelp.h"

/* ========================================================================= */
struct filter_on *CreateFilter(void)
{
   struct filter_on *f;

   if ( NULL == (f = (struct filter_on *)malloc(sizeof(struct filter_on))) )
      return(NULL);

   f->pid = 0;
   f->uid = 0; /* 0 is root's ID. You cannot filter on root. */

   return(f);
}

/* ========================================================================= */
int FilterOnUID(struct filter_on *f, uid_t uid)
{
   if ( f == NULL )
      return(1);

   f->uid = uid;

   /* Cannot filter on root */
   if (0 == f->uid)
      return(1);

   return(0);
}

/* ========================================================================= */
int FilterReset(struct filter_on *f)
{
   if ( f == NULL )
      return(1);

   f->pid = 0;

   return(0);
}

/* ========================================================================= */
int FilterOnUName(struct filter_on *f, char *uname)
{
   if(-1 == (f->uid = GetUIDbyName(uname)))
      return(1);

   /* Cannot filter on root */
   if (0 == f->uid)
      return(1);

   return(0);
}

/* ========================================================================= */
int copy_in_pd(struct pid_data *p, struct procentry64 *pe)
{
   p->pid = pe->pi_pid;
   p->uid = pe->pi_uid;


   /* DEBUG */
   strcpy(p->pi_comm, pe->pi_comm);
   

   return(0);
}


/* ========================================================================= */
int GetPIDData(struct pid_data *p, struct filter_on *f)
{
   struct procentry64 pe;
   int gpgot;

   while (1 == (gpgot = getprocs64(&pe, sizeof(struct procentry64), NULL, 0, &f->pid, 1)))
   {
      /* error - stop trying */
      if ( gpgot == -1 )
      {
         /* perror("getprocs64"); */
         return(0);
      }

      if ( pe.pi_pid == 0 ) /* Skip PID 0 - this is a bug */
         continue;

      if ( pe.pi_pid == 1 ) /* Don't bother looking at init */
         continue;

      if ( f->uid > 0 ) /* You cannot filter on root! */
      {
         if ( pe.pi_uid == f->uid )
         {
            copy_in_pd(p, &pe);
            return(1);
         }
      }
      else
      {
         copy_in_pd(p, &pe);
         return(1);
      }
   }

   return(0);
}
