#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc_info.h>
#include <errno.h>
#include <assert.h>
#include <pwd.h>

#include "options.h"
#include "distrogph.h"
#include "version.h"

/*
  The URL of the API:
    http://publib.boulder.ibm.com/infocenter/aix/v7r1/topic/com.ibm.aix.basetechref/doc/basetrf1/get_ipc_info.htm

  Notes on the API (not *initially clear* in the page):
    - The last paramater (int *size) really *is* the size. It is NOT the
      count. It is just a bit misleading that size is captured in a signed
      int.
    - Call the API with NULL for the buffer pointer and then see the 
      resulting size value returned to do the malloc() for the buffer.
      The count can be determined by dividing the size by the sizeof() the
      desired struct.
    - ENOSPC is a *return value* not an errno value. (It is really "both".)
      The point is that <errno.h> is required, but not explicitly specified
      in the InfoCenter page. I am not sure what the documentation convention
      for errnos in the return value are, but i think it should be noted
      as a required header as you NEED to know this value when deriving 
      the size of the buffer.

  Notes on the API (of potential interest):
    - There is an option to query Posix shared memory segments. That might
      be of interest. See: GET_IPCINFO_RTSHM

  Notes on the application:
    - This is very brute force at this time, no command line options parsed.
    - Some additional items that could/should be derived:
      > possibly the number of pinned segments
      > max/min size or, better yet, a size distribution graph
      > count by user
    - The app should be suid root or at least check euid of root. Improper
      permissions will (potentially) give undesirable results. (I am not sure
      this is a *real* requirement... I ran this successfully (in the original
      creation) as a non-priv user.
    - The original creation had a loop (trying to retrieve the results until
      it achieved proper size and success. The current version does not do
      this. Multiple tries *should* be attempted (coded for) in the event 
      that an additional segment is added between the inital (sizing) request
      and the actual data retrieval request.
      
*/

/* ========================================================================= */
struct per_user
{
   uid_t		uid;
   char *uname;
   size64_t	total_size;
   unsigned long count;
   unsigned long no_attach_count;

   struct per_user *next;
};

/* ========================================================================= */
char *factor_bytes(char *facts, unsigned long long bytes);
int about(void);
int help(void);
struct per_user *collect_per_user(struct per_user *ulist,
                                  ipcinfo_shm_t *shmbuf);
struct per_user *dump_ulist(struct per_user *ulist);

/* ========================================================================= */
int main(int argc, char *argv[])
{
   struct options *o;

   /* API data */
   ipcinfo_shm_t *shmbuf = NULL;
   int size;
   char fact_string[32];

   /* Result data */
   long count = 0;
   int no_attach_cnt = 0;
   size64_t total_size = 0;
   size64_t min_size = 0;
   size64_t max_size = 0;
   struct per_user *ulist = NULL;
   struct barray *ba = NULL;

   /* General usage */
   int i;
   int keep_trying;



   if ( NULL == (o = ReadOptions(argc, argv)) )
      return(1);

   if ( o->bError )
      return(1);

   if ( o->bAbout )
      return(about());

   if ( o->bHelp )
      return(help());

   keep_trying = 1;
   while ( keep_trying )
   {
      if ( shmbuf )
         free(shmbuf);

      /*** Determine size of required buffer ***/
      if ( ENOSPC != get_ipc_info(0,
                                  GET_IPCINFO_SHM_ALL,
                                  IPCINFO_SHM_VERSION_1,
                                  NULL,
                                  &size))
      {
         fprintf(stderr, "ERROR: Unable to determine size/count of data.\n");
         return(1);
      }

      /*** Allocate memory for the buffer/array */
      if ( NULL == (shmbuf = (ipcinfo_shm_t *)malloc(size)) )
      {
         fprintf(stderr, "ERROR: Failed to allocate memory.\n");
         return(1);
      }
      
      /* Copy in the data to the allocated array ***/
      switch (  get_ipc_info(0,
                         GET_IPCINFO_SHM_ALL,
                         IPCINFO_SHM_VERSION_1,
                         (char *)shmbuf,
                         &size) )
      {
      case 0:
         keep_trying = 0;
         break;
      case ENOSPC:
         break;
      default:
         fprintf(stderr, "ERROR: Problems retrieving IPC data.\n");
         return(1);
         /* Unreachable */
         break;
      }
   }

   /* Determine count from size */
   count = size / (int)(sizeof(ipcinfo_shm_t));

   if ( o->bSizeGph )
   {
      ba = InitBuckets(BA_BASE10);
   }


   /*** Walk through the array deriving results as we go ***/
   i = 0;
   while ( i < count )
   {
      if ( shmbuf[i].shm_nattch = 0 )
         no_attach_cnt++;

      total_size += shmbuf[i].shm_segsz;

      if ( i == 0 )
      {
         min_size = shmbuf[i].shm_segsz;
         min_size = shmbuf[i].shm_segsz;
      }

      if ( min_size > shmbuf[i].shm_segsz )
          min_size = shmbuf[i].shm_segsz;

      if ( max_size < shmbuf[i].shm_segsz )
          max_size = shmbuf[i].shm_segsz;


      if ( o->bSizeGph )
         BucketValue(ba, shmbuf[i].shm_segsz);

      if ( o->bPerUser )
         ulist = collect_per_user(ulist, &shmbuf[i]);

      i++;
   }

   /*** Display results ***/
   printf("Number of shm segments               : %d\n", count);
   printf("Number of no-attach segments         : %d\n", no_attach_cnt);
   factor_bytes(fact_string, total_size);
   printf("Total size of shm segments (human)   : %s\n", fact_string);
   printf("Total size of shm segments (bytes)   : %llu\n", total_size);
   factor_bytes(fact_string, max_size);
   printf("Maximum sized shm segment            : %s\n", fact_string);
   factor_bytes(fact_string, min_size);
   printf("Minimum sized shm segment            : %s\n", fact_string);


   if ( ulist )
      dump_ulist(ulist);

   if ( ba )
   {
      int max_count;

      max_count = GetMaxCount(ba);

      printf("Size distribution data:\n");
      DumpBucketArray(ba, max_count);
   }
   fflush(stdout);

   return(0);
}

/* ========================================================================= */
struct per_user *dump_ulist(struct per_user *ulist)
{
   char fact_string[32];
   struct per_user *this_user;

   printf("Per-user data:\n");
   printf("  %-10s %7s %7s %15s %15s\n", "User", "Count", "!Attach", "Size (bytes)", "Size (human)");

   this_user = ulist;
   while(this_user)
   {
      factor_bytes(fact_string, this_user->total_size);

      printf("  %-10s %7lu %7lu %15llu %15s\n",
             this_user->uname,
             this_user->count,
             this_user->no_attach_count,
             (unsigned long long)this_user->total_size,
             fact_string);

      this_user = this_user->next;
   }

   return(NULL);
}


/* ========================================================================= */
struct per_user *find_user(struct per_user *ulist, uid_t uid)
{
   struct per_user *this_user;

   this_user = ulist;
   while(this_user)
   {
      if ( this_user->uid == uid )
         return(this_user);

      this_user = this_user->next;
   }

   return(NULL);
}

/* ========================================================================= */
char *get_uname(uid_t uid)
{
   struct passwd *pwe;
   char *uname;

   if ( NULL == (uname = (char *)malloc(32)) )
      return(NULL); /* unlikely, but required */

   if( NULL == (pwe = getpwuid(uid)) )
   {
      sprintf(uname, "%lu", (unsigned long)uid);
   }
   else
   {
      strcpy(uname, pwe->pw_name);
   }

   return(uname);
}

/* ========================================================================= */
struct per_user *collect_per_user(struct per_user *ulist, ipcinfo_shm_t *shmbuf)
{
   struct per_user *the_user;

   if ( NULL == shmbuf )
      return(ulist);

   if ( NULL == (the_user = find_user(ulist, shmbuf->shm_perm.uid)) )
   {
      if (NULL == (the_user = (struct per_user *)malloc(sizeof(struct per_user))))
      {
         /* This gives bad results - but is highly unlikely */
         return(ulist);
      }

      the_user->uid = shmbuf->shm_perm.uid;
      the_user->uname = get_uname(shmbuf->shm_perm.uid);
      the_user->total_size = shmbuf->shm_segsz;
      the_user->count = 1;
      if ( shmbuf->shm_nattch = 0 )
         the_user->no_attach_count = 1;
      else
         the_user->no_attach_count = 0;
      the_user->next = ulist;

      return(the_user);
   }
   else
   {
      the_user->total_size += shmbuf->shm_segsz;
      the_user->count++;
      if ( shmbuf->shm_nattch = 0 )
         the_user->no_attach_count++;
      return(ulist);
   }

   /* unreachable */
   return(NULL);
}

/* ========================================================================= */
int about(void)
{
   printf("shmstat - Display summary statistics on shmat() segments.\n");
   printf("   Version: %s\n", VERSION_STRING);
   printf("     Udham Singh <amritsar@pentonville.gov.uk>\n");
   printf("     William Favorite <wfavorite2@bloomberg.net>\n");

   return(0);
}

/* ========================================================================= */
int help(void)
{
   printf("shmstat - Display summary statistics on shmat() segments.\n");
   printf("   Usage: shmstat [options]\n");
   printf("   Options:\n");
   printf("     -a   Show \"about\" information (and exit)\n");
   printf("     -h   Show usage information (and exit)\n");
   printf("     -u   Show summary of per-user information\n");
   printf("     -s   Show size distribuiton information\n");

   return(0);
}



/* ========================================================================= */
char *factor_bytes(char *facts, unsigned long long bytes)
{
   int f;
   char FACTOR[] = "BKMGTEPXXXXX";

   assert(NULL != facts);

   f = 0;
   while ( bytes > 10000 )
   {
      f++;
      bytes /= 1024;
   }

   sprintf(facts, "%llu %c", bytes, FACTOR[f]);

   return(facts);
}


