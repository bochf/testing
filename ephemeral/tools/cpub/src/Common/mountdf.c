#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <errno.h>

#if defined(PORT_Linux) || defined(PORT_AIX)
#include <mntent.h>
#endif

#if defined(PORT_SunOS)
#include <sys/mnttab.h>
#include <sys/vfstab.h>
#endif

#include <sys/statvfs.h>

#include "../mainhelp.h"
#include "mountdf.h"


/* ========================================================================= */
/*
  Notes on this provider:
  - The code supports the concept of reading from fstab or mntab. Only mntab
    is appropriate here. fstab support may exist, but it is not to be used.
  - This is the first code to support wild cards in the provider argument.
    it supports a single wildcard ("*"). The enablepitem() function may 
    return more than a single pitem struct.
  - The "read the mount list" code is heavily ifdef-ed. AIX has a Linux
    compatibility option that it relies on. SunOS is the real outlier here.
*/

/* ========================================================================= */
/* These are IDs that are put in sioffset */
#define PI_NOT_SET  0
#define PI_MNTNAME  1
#define PI_TOTALSZ  2  /* f_blocks * f_frsize  (in bytes)                    */
#define PI_FREESIZ  3  /* f_bfree * f_frsize   (in bytes)                    */
#define PI_NRFREEB  4  /* f_bavail * f_frsize  (in bytes)                    */
#define PI_PCTFREE  5
#define PI_PCTUSED  6


/* ========================================================================= */
/* The code supports getting the mount list from two locations. We ONLY use
   the mntab source - fstab is not useful in determining what is actually
   mounted. (That is the job of some *other* tool.)
*/
#define LIST_FSTAB 1
#define LIST_MNTAB 2

/* ========================================================================= */
/* This is a single use "file handle" type. (It has an actual file handle
   in it.) The point here is that there is "state" that is maintained between
   calls to the API. That state (and results) are maintained here. The API is:
      struct mlistd *open_mount_list(int src);
      int next_mount_item(struct mlistd *m);
      int close_mount_list(struct mlistd *m);
*/
struct mlistd /* mount list descriptor */
{
   int list_src;
   FILE *mf;
#if defined(PORT_Linux) || defined(PORT_AIX)
   struct mntent *mnt;
#endif
#if defined(PORT_SunOS)
   struct mnttab *mnt;
   struct vfstab *vfs;
#endif

   char *mount_point;
   char nada[4];
};


/* ========================================================================= */
/* Used to create a simple list of ALL mounts. This can be used to:
   1) determine if a supplied mount point is valid
   2) create a list of mounts (if user supplied * wildcard
   This information is kept in "full_mount_list".
*/
/* ========================================================================= */
/* This is the list of mounts that must be stat()ed. The pitems will link to
   the mount that they get data for via the dentry pointer. 
   This information is kept in "mnt_item_ui". */
struct mnt_item
{
   char *mount_point;

   struct statvfs *fs_stat;

   struct mnt_item *ki_next;
   struct mnt_item *ui_next;
};

/* Variables static / global to this file */
static struct mnt_item *full_mount_list;  /* A "raw" list of known mounts   */
static struct mnt_item *mnt_item_ui;      /* The list of mount update items */
static struct provider *myself;


/* ========================================================================= */
/* The items we support */
static const char *diname[] = {"mountpt",    "totalsz",    "privfree",  \
                               "availfree",  "pctfree",    "pctused",   \
                               NULL};
static const int ditype[]  =  {PDT_STRING,   PDT_UINT64,   PDT_UINT64,  \
                               PDT_UINT64,   PDT_UINT16,   PDT_UINT16,  \
                               };

/* ========================================================================= */
struct mlistd *open_mount_list(int src);
int next_mount_item(struct mlistd *m);
int close_mount_list(struct mlistd *m);
int is_not_in_ui_list(struct mnt_item *matchmi);
int build_mount_list(void);
int is_a_valid_mount(char *mount_point);
struct mnt_item *get_mount_item(char *mount_point);
unsigned long parse_mpdi(char *iname);
int get_data_type(unsigned long sioffset);
void *make_data_type(unsigned long sioffset, char *mount_point);
int mdf_pitem_update(struct pitem *thispi);
/* --- The "three" functions --- */
int mdf_list(int dflag);
int mdf_update(int interval);
struct pitem *mdf_enablepitem(struct qparts *qp);


/* ========================================================================= */
int MountDFRegister(struct proot *p)
{
   /* Set the lists to un-initialized */
   full_mount_list = NULL;
   mnt_item_ui = NULL;

   /* No data types are checked - we only export *specific* data types. */

   if ( NULL == (myself = RegisterProvider(p, "mount.df",
                                           mdf_update,
                                           mdf_list,
                                           mdf_enablepitem)) )
   {
      /* All about readability */
      return(1);
   }

   return(0);

}

/* ========================================================================= */
int build_mount_list(void)
{
   struct mlistd *m;
   struct mnt_item *thism;

   if ( NULL != full_mount_list )
      return(0);

   if (NULL == (m = open_mount_list(LIST_MNTAB)))
   {
      error_msg("ERROR: Unable to read the list of current mounts.");
      return(1);
   }


   while ( 0 == next_mount_item(m) )
   {
      /* Malloc the data structure */
      if (NULL == (thism = (struct mnt_item *)malloc(sizeof(struct mnt_item))))
         return(1);

      /* Malloc the string (for the mount point) */
      if (NULL == (thism->mount_point = (char *)malloc(strlen(m->mount_point) + 1)))
         return(1);

      /* Copy in the string */
      strcpy(thism->mount_point, m->mount_point);

      /* Set all (immediately) unused items to NULL */
      thism->fs_stat = NULL;
      /* sm->ki_next = NULL;  <---- Will be set in following lines */
      thism->ui_next = NULL;

      /* Insert into linked list */
      thism->ki_next = full_mount_list;
      full_mount_list = thism;
   }

   close_mount_list(m);

   return(0);
}

/* ========================================================================= */
int is_not_in_ui_list(struct mnt_item *matchmi)
{
   struct mnt_item *thismi;

   thismi = mnt_item_ui;
   while ( thismi )
   {
      if ( thismi == matchmi )
         return(0); /* Found it, return false */

      thismi = thismi->ui_next;
   }

   /* Not found, return true */
   return(1);
}

/* ========================================================================= */
struct mnt_item *get_mount_item(char *mount_point)
{
   struct mnt_item *thismi;

   /* Go ahead and create this if it does not exist */
   if ( NULL == full_mount_list )
   {
      if ( build_mount_list() )
         return(NULL);
   }

   /* Look for it in an existing list */
   thismi = full_mount_list;
   while ( thismi )
   {
      if ( 0 == strcmp(thismi->mount_point, mount_point) )
      {
         /* An existing mount item was found. Return it. */

         /* If the statvfs was never allocated, then allocate it */
         if ( NULL == thismi->fs_stat )
         {
            if (NULL == (thismi->fs_stat = (struct statvfs *)malloc(sizeof(struct statvfs))))
               return(NULL);
         }

         /* Put it into the update list - once */
         if ( is_not_in_ui_list(thismi) )
         {
            thismi->ui_next = mnt_item_ui;
            mnt_item_ui = thismi;
         }

         return(thismi);
      }

      thismi = thismi->ki_next;
   }

   return(NULL);
}

/* ========================================================================= */
unsigned long parse_mpdi(char *iname)
{
   if ( 0 == strcmp(iname, "mountpt") )
      return(PI_MNTNAME);

   if ( 0 == strcmp(iname, "totalsz") )
      return(PI_TOTALSZ);

   if ( 0 == strcmp(iname, "privfree") )
      return(PI_FREESIZ);

   if ( 0 == strcmp(iname, "availfree") )
      return(PI_NRFREEB);

   if ( 0 == strcmp(iname, "pctfree") )
      return(PI_PCTFREE);

   if ( 0 == strcmp(iname, "pctused") )
      return(PI_PCTUSED);

   return(PI_NOT_SET);
}

/* ========================================================================= */
int get_data_type(unsigned long sioffset)
{
   switch(sioffset)
   {
   case PI_MNTNAME:
      return(PDT_STRING);
      break; /* its all */
   case PI_TOTALSZ:
   case PI_FREESIZ:
   case PI_NRFREEB:
      return(PDT_UINT64);
      break; /* about */
   case PI_PCTFREE:
   case PI_PCTUSED:
      return(PDT_INT16);
      break; /* the compiler */
   }

   return(PDT_UNKNOWN);
}

/* ========================================================================= */
void *make_data_type(unsigned long sioffset, char *mount_point)
{
   switch(sioffset)
   {
   case PI_MNTNAME:
      return((void *)mkstring(mount_point));
      break; /* its all */
   case PI_TOTALSZ:
   case PI_FREESIZ:
   case PI_NRFREEB:
      return(malloc(sizeof(uint64_t)));
      break; /* about */
   case PI_PCTFREE:
   case PI_PCTUSED:
      return(malloc(sizeof(int16_t)));
      break; /* the compiler */
   }

   return(NULL);
}


/* ========================================================================= */
struct pitem *mdf_mkpitem(struct qparts *qp, struct mnt_item *the_mount)
{
   struct pitem *this_pitem;
   unsigned long sioffset;
   int data_type;
   void *data_ptr;

   /* This is slightly cleaner than an assert */
   if ((NULL == qp) || (NULL == the_mount))
      return(NULL);

   /* Determine the data item wanted */
   if ( PI_NOT_SET == (sioffset = parse_mpdi(qp->iname)) )
   {
      error_msg("ERROR: Data item \"%s\" not understood.", qp->iname);
      return(NULL);
   }

   /* sioffset is known good at this point. The following two must be good as well */
   /* If not (for some reason) then NewPItem() will catch the unsupported value */
   data_type = get_data_type(sioffset);
   data_ptr = make_data_type(sioffset, qp->pargs);

   /* If all good, then create a pitem */
   if ( NULL == (this_pitem = NewPItem(qp, data_type, data_ptr)) )
      return(NULL);

   /* Add the additional data items */
   this_pitem->dstruct = (void *)the_mount;
   this_pitem->sioffset = sioffset;  /* The specific data item requested.
                                        NOTE: sioffset is NOT used as an offset, but
                                        as a flag using one of the TS_* defines above */

   /* Shove this into the ui_list */
   this_pitem->next_ui = myself->ui_list;
   myself->ui_list = this_pitem;

   return(this_pitem);
}

/* ========================================================================= */
struct pitem *mdf_enablepitem(struct qparts *qp)
{
   struct pitem *pitemlist;
   struct pitem *this_pitem;
   struct mnt_item *thismi;
   struct mnt_item *gotmi;

   pitemlist = NULL;

   /* Make sure we were called with the right quad (provider) name */
   if ( 0 != strcmp(qp->pname, "mount.df") )
      return(NULL);

   /* Parse iargs */
   if ( ShouldDiff(qp) )
   {
      error_msg("ERROR: DIFF is not supported on file.perfops items.");
      return(NULL);
   }

   if ( ShouldPSAvg(qp) )
   {
      error_msg("ERROR: PSAVG is not supported on file.perfops items.");
      return(NULL);
   }


   if ( qp->pargs[0] == 0 )
   {
      error_msg("ERROR: No mount specified in mount.df arg quad.");
      return(NULL);
   }

   if ( qp->pargs[0] == '*' )
   {
      if ( NULL == full_mount_list )
      {
         if ( build_mount_list() )
            return(NULL);
      }

      thismi = full_mount_list;
      while ( thismi )
      {
         if ( (0 == strcmp(thismi->mount_point, "/proc")) || (0 == strcmp(thismi->mount_point, "/sys")) )
         {
            thismi = thismi->ki_next;
            continue;
         }

         /* Q: Why on earth would you call get_mount_item() as you are walking
               the list? The results will be the same as what you queried!
            A: Because get_mount_item() has side effects. It knows that you
               are querying the list because you will be using it. So it does
               the extra allocation and list maintenance for you. I call it
               so that there is a single instance of this code to maintain.
         */
         gotmi = get_mount_item(thismi->mount_point);


         /* But wait!!! qp will be the same for multiple entries! How do we
            know what is what? When it returns, we modify the header. See
            the code below... */
         if ( NULL == (this_pitem = mdf_mkpitem(qp, gotmi)) )
            return(NULL);

         /* Here it is - we modify the header to insure uniqueness (as much
            as MAX_QPART_LEN will allow us to). */
         strncpy(this_pitem->header, thismi->mount_point, MAX_QPART_LEN);

         /* Now put this into the "list". Order does not matter as all
            items here are wild-carded and "equate" to the same thing. 
            NOTE: This is the opi list! The providers should never manage
            this list.... except when they are returning multiple items.
         */
         this_pitem->next_opi = pitemlist;
         pitemlist = this_pitem;


         thismi = thismi->ki_next;
      }

      /* Flag update for the provider */
      myself->update_required = 1;
   }
   else
   {
      /* No relative pathing */
      if ( qp->pargs[0] != '/' )
      {
         error_msg("ERROR: mount.df mounts must be fully pathed.");
         return(NULL);
      }

      /* Get mount point */
      if ( NULL == (thismi = get_mount_item(qp->pargs)) )
      {
         error_msg("ERROR: \"%s\" is not a recognized mount.", qp->pargs);
         return(NULL);
      }

      if ( NULL == (this_pitem = mdf_mkpitem(qp, thismi)) )
         return(NULL);

      /* Create a list of one */
      pitemlist = this_pitem;

      /* Flag update for the provider */
      myself->update_required = 1;
   }

   return(pitemlist);
}

/* ========================================================================= */
int mdf_list(int dflag)
{
   int icount;

   icount = 0;
   while ( diname[icount] )
   {
      DumpQuadData(dflag, ditype[icount]);
      printf("mount.df:*:%s\n", diname[icount]);

      icount++;
   }

   return(icount);
}

/* ========================================================================= */
int mdf_update(int interval)
{
   struct mnt_item *mithis;
   struct pitem *this_pitem;
   int rv;

   rv = 0;
   mithis = mnt_item_ui;

   while(mithis)
   {
      rv += statvfs(mithis->mount_point, mithis->fs_stat);

      mithis = mithis->ui_next;
   }

   /* Now update all the data items (converting timestamps to diffs) */
   this_pitem = myself->ui_list;
   while ( this_pitem )
   {
      rv += mdf_pitem_update(this_pitem);

      this_pitem = this_pitem->next_ui;
   }

   return(rv);
}

/* ========================================================================= */
int mdf_pitem_update(struct pitem *thispi)
{
   struct mnt_item *mi; /* Less than optimal effort to beat casting issues */
   uint64_t btotal;
   uint64_t bfree;
   uint64_t pctfree;
   
   /* If the string, then nothing to do */
   if ( PI_MNTNAME == thispi->sioffset )
      return(0);

   mi = thispi->dstruct;

   if ( PI_TOTALSZ == thispi->sioffset )
   {
      *((uint64_t *)thispi->data_ptr) = mi->fs_stat->f_blocks * mi->fs_stat->f_frsize;
      return(0);
   }

   if ( PI_FREESIZ == thispi->sioffset )
   {
      *((uint64_t *)thispi->data_ptr) = mi->fs_stat->f_bfree * mi->fs_stat->f_frsize;
      return(0);

   }

   if ( PI_NRFREEB == thispi->sioffset )
   {
      *((uint64_t *)thispi->data_ptr) = mi->fs_stat->f_bavail * mi->fs_stat->f_frsize;
      return(0);
   }

   /* The remaining items require us to calculate percentages */
   btotal = mi->fs_stat->f_blocks;
   bfree = mi->fs_stat->f_bavail * 100;
   
   if ( 0 == btotal )
      pctfree = 0;
   else
      pctfree = bfree / btotal;

   if ( PI_PCTFREE == thispi->sioffset )
   {
      *((uint16_t *)thispi->data_ptr) = (uint16_t)pctfree;
      return(0);
   }

   if ( PI_PCTUSED == thispi->sioffset )
   {
      *((uint16_t *)thispi->data_ptr) = 100 - (uint16_t)pctfree;
      return(0);
   }

   return(1);
}

/* *************************************************************************
   *************************************************************************
   ************************************************************************* */
/* All the platform-specific coding is below this line */


/* ========================================================================= */
struct mlistd *open_mount_list(int src)
{
   FILE *mf;
   struct mlistd *m;

   mf = NULL;

   if (( LIST_FSTAB != src ) && ( LIST_MNTAB != src ))
      return(NULL);
 
#if defined(PORT_Linux) || defined(PORT_AIX)
   if ( src == LIST_FSTAB )
      mf = setmntent("/etc/fstab", "r");

   if ( src == LIST_MNTAB )
      mf = setmntent("/etc/mtab", "r");
#endif

#if defined(PORT_SunOS)
   if ( src == LIST_FSTAB )
      mf = fopen("/etc/vfstab", "r");

   if ( src == LIST_MNTAB )
      mf = fopen("/etc/mnttab", "r");
#endif

   /* If NULL, then failure */
   if ( NULL == mf )
      return(NULL);

   if ( NULL == (m = (struct mlistd *)malloc(sizeof(struct mlistd))) )
      return(NULL);

#if defined(PORT_SunOS)
   if ( src == LIST_FSTAB )
   {
      m->mnt = NULL;

      if ( NULL == (m->vfs = (struct vfstab *)malloc(sizeof(struct vfstab))))
         return(NULL);
   }

   if ( src == LIST_MNTAB )
   {
      m->vfs = NULL;

      if ( NULL == (m->mnt = (struct mnttab *)malloc(sizeof(struct mnttab))))
         return(NULL);
   }
#endif

   /* Assign appropriate values */
   m->mf = mf;
   m->list_src = src;
   strcpy(m->nada, "-");

   m->mount_point = NULL;

   return(m);
}

/* ========================================================================= */
int next_mount_item(struct mlistd *m)
{
   if ( NULL == m )
      return(-1);

   if ( NULL == m->mf )
      return(-1);

   if (( LIST_FSTAB != m->list_src ) && ( LIST_MNTAB != m->list_src ))
      return(-1);

#if defined(PORT_Linux) || defined(PORT_AIX)
   if ( NULL == (m->mnt = getmntent(m->mf)) )
      return(1);

   m->mount_point = m->mnt->mnt_dir;
#endif

#if defined(PORT_SunOS)
   if ( LIST_FSTAB == m->list_src )
   {
      if ( getvfsent(m->mf, m->vfs) )
         return(1);

      m->mount_point = m->vfs->vfs_mountp;
   }

   if ( LIST_MNTAB == m->list_src )
   {
      if ( getmntent(m->mf, m->mnt) )
         return(1);

      m->mount_point = m->mnt->mnt_mountp;
   }
#endif

   if ( NULL == m->mount_point )
      m->mount_point = m->nada;

   return(0);
}

/* ========================================================================= */
int close_mount_list(struct mlistd *m)
{
#if defined(PORT_Linux) || defined(PORT_AIX)
   endmntent(m->mf);

   /* Linux and AIX use a statically allocated struct - so, no free() */
#endif

#if defined(PORT_SunOS)
   fclose(m->mf);

   free(m->mnt);
   free(m->vfs);
#endif

   free(m);

   return(0);
}

