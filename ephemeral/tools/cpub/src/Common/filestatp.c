#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <errno.h>

#include "../mainhelp.h"
#include "filestatp.h"


/*
  Notes on this provider:
  - This provider does not keep a different stat structure for each *file*.
    It keeps a different stat structure for each *request*. So, if a user
    were to request five different stat() statistics for a single file,
    then the file would be stat()ed five different times. The "tradeoff"
    was made based on the belief that users will not be inclined to ask
    for more than one stat() statistic per file. This belief was weighed
    against the complexity of writing code capable of limiting this call.
    Additionally, the stat() call was not seen as too expensive in the
    grand scheme of how often this module is likely to be used.
  - stat() returns many very bitness-sensitive typedefed data types. This
    is one of the reasons the framework should be compiled 64 bit. stat()
    is typically sensitive to bitness of the app, but also large file
    support flags. Just staying 64 bit tends to simplify this a bit.
  - Providers that use sioffset to find their way to the struct member
    SHOULD NOT use another struct to hold the result data. This is 
    because if TWO quads are defined for one piece of data then they will
    clobber each other (if, for example, one is a diff and the other is
    not). That is NOT AN ISSUE HERE because each stat call (even if for
    the same file) uses a different stat struct. So this may consume more
    memory, but there is no fear of clobbering some other quads data.
*/

struct file_stat
{
   char *name;
   
   struct stat *prevs;
   struct stat *thiss;
   struct stat *datas; /* See notes above */

   int this_rv;
   int this_errno;
   int prev_rv;
   int prev_errno;
};



/* The items we support */
static const char *diname[] = {"ino",        "size",       "atime",     \
                               "mtime",      "ctime",                   \
                               NULL};
static const int ditype[]  =  {PDT_INT64,    PDT_INT64,    PDT_INT64,   \
                               PDT_INT64,    PDT_INT64                  \
                               };

static struct provider *myself;


/* ========================================================================= */
struct file_stat *new_file_stat(char *filename)
{
   struct file_stat *fs;

   assert ( filename != NULL );


   if ( NULL == (fs = (struct file_stat *)malloc(sizeof(struct file_stat))) )
   {
      /* Error messages *can* be printed at this point */
      fprintf(stderr, "ERROR: Unable to allocate memory in the file.stat provider.\n");
      return(NULL);
   }

   if ( NULL == (fs->name = mkstring(filename)) )
   {
      fprintf(stderr, "ERROR: Unable to allocate memory in the file.stat provider.\n");
      return(NULL);
   }

   if ( NULL == (fs->prevs = (struct stat *)malloc(sizeof(struct stat))) )
   {
      fprintf(stderr, "ERROR: Unable to allocate memory in the file.stat provider.\n");
      return(NULL);
   }

   if ( NULL == (fs->thiss = (struct stat *)malloc(sizeof(struct stat))) )
   {
      fprintf(stderr, "ERROR: Unable to allocate memory in the file.stat provider.\n");
      return(NULL);
   }

   if ( NULL == (fs->datas = (struct stat *)malloc(sizeof(struct stat))) )
   {
      fprintf(stderr, "ERROR: Unable to allocate memory in the file.stat provider.\n");
      return(NULL);
   }

   return(fs);
}

/* ========================================================================= */
int fitem_update(struct pitem *this_pitem, int interval)
{
   struct file_stat *fs;
   struct stat *temps;

   int64_t *lllast;
   int64_t *llthis;
   int64_t *lldata;
   
   /* Get a local, non-casted, copy of the file_stat structure */
   fs = this_pitem->dstruct;

   /* Rotate stat pointers */
   temps = fs->prevs;
   fs->prevs = fs->thiss;
   fs->thiss = temps;

   /* "shift" (rotate) return values */
   fs->prev_rv = fs->this_rv;
   fs->prev_errno = fs->this_errno;

   if ( -1 == (fs->this_rv = stat(fs->name, fs->thiss)) )
   {
      /* If we errored, save the errno, but treat it like regular data! */
      fs->this_errno = errno;

      if ( this_pitem->data_type == PDT_INT64 )
      {
         /* Find our data */
         lllast = (int64_t *)((unsigned long)fs->prevs + (unsigned long)this_pitem->sioffset);
         llthis = (int64_t *)((unsigned long)fs->thiss + (unsigned long)this_pitem->sioffset);
         lldata = this_pitem->data_ptr;

         /* Do something *different* assign the value to "this".
            That is nuts!! Why??
            Because it needs to be rotated and diffed just like normal data. */
         *llthis = (int64_t)fs->this_errno * (int64_t)-1;

         /* Now treat it like normal data */
         if ( this_pitem->munge_flag & MUNGE_DIFF )
            *lldata = *llthis - *lllast;
         else
            *lldata = *llthis;

         if ( this_pitem->munge_flag & MUNGE_PSAVG )
            *lldata = *lldata / (int64_t)interval;
      }
   }
   else
   {
      if ( this_pitem->data_type == PDT_INT64 )
      {
         /* Find our data */
         lllast = (int64_t *)((unsigned long)fs->prevs + (unsigned long)this_pitem->sioffset);
         llthis = (int64_t *)((unsigned long)fs->thiss + (unsigned long)this_pitem->sioffset);
         lldata = this_pitem->data_ptr;

         if ( this_pitem->munge_flag & MUNGE_DIFF )
            *lldata = *llthis - *lllast;
         else
            *lldata = *llthis;

         if ( this_pitem->munge_flag & MUNGE_PSAVG )
            *lldata = *lldata / (int64_t)interval;
      }
   }

   return(0);
}

/* ========================================================================= */
int fstat_update(int interval)
{
   struct pitem *this_pitem;
   int trv;

   trv = 0;
   this_pitem = myself->ui_list;
   while ( this_pitem )
   {
      trv += fitem_update(this_pitem, interval);

      this_pitem = this_pitem->next_ui;
   }

   return(trv);
}


/* ========================================================================= */
int fstat_list(int dflag)
{
   int icount = 0;

   while ( diname[icount] )
   {
      DumpQuadData(dflag, ditype[icount]);
      printf("file.stat::%s\n", diname[icount]);

      icount++;
   }

   return(icount);
}

/* ========================================================================= */
#define REG_INT64(DITEM); if ( 0 == strcmp(qp->iname, #DITEM) )                                                                      \
                          {                                                                                                          \
                             if ( NULL == ( this_pitem = NewPItem(qp, PDT_INT64, &this_fs->datas->st_##DITEM) ) )                    \
                                return(NULL);                                                                                        \
                                                                                                                                     \
                             this_pitem->sioffset = (unsigned long)&this_fs->thiss->st_##DITEM - (unsigned long)this_fs->thiss;      \
                          }


struct pitem *fstat_enablepitem(struct qparts *qp)
{
   struct pitem *this_pitem;
   struct file_stat *this_fs;

   /* Make sure we were called with the right quad (provider) name */
   if ( 0 != strcmp(qp->pname, "file.stat") )
      return(NULL);

   if ( qp->pargs[0] == 0 )
   {
      fprintf(stderr, "ERROR: No filename specified in file.stat quad.\n");
      return(NULL);
   }

   /* It looks like the input is good */
   /* NOTE: stat() could be called here to insure that permissions are
      ok. But a failed stat() is not considered a failure condition. */


   /* Flag update for the provider */
   myself->update_required = 1;

   /* mallocate memory for the the stat buffers */
   if ( NULL == (this_fs = new_file_stat(qp->pargs)) )
      return(NULL);

   this_pitem = NULL;

   REG_INT64(ino);
   REG_INT64(size);
   REG_INT64(atime);
   REG_INT64(mtime);
   REG_INT64(ctime);

   if ( NULL == this_pitem )
   {
      /* No match for this item */
      return(NULL);
   }

   this_pitem->dstruct = this_fs;

   /* Parse iargs */
   if ( ShouldDiff(qp) )
      this_pitem->munge_flag |= MUNGE_DIFF;

   if ( ShouldPSAvg(qp) )
      this_pitem->munge_flag |= MUNGE_DIFF;

   this_fs->this_rv = stat(this_fs->name, this_fs->thiss);

   /* Shove this into the ui_list */
   this_pitem->next_ui = myself->ui_list;
   myself->ui_list = this_pitem;

   return(this_pitem);
}

/* ========================================================================= */
int FileStatRegister(struct proot *p)
{
   struct stat assert_stat; /* Just used to reference the data types */

   /* Check our data types */
   assert(sizeof(assert_stat.st_ino) == sizeof(int64_t));
   assert(sizeof(assert_stat.st_size) == sizeof(int64_t));
   assert(sizeof(assert_stat.st_atime) == sizeof(int64_t)); /* atime == mtime == ctime */

   if ( NULL == (myself = RegisterProvider(p, "file.stat",
                                           fstat_update,
                                           fstat_list,
                                           fstat_enablepitem)) )
   {
      /* All about readability */
      return(1);
   }

   return(0);

}
