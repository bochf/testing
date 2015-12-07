#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include "mntlist.h"



/* ========================================================================= */
struct mlistd *open_mount_list(int src)
{
   FILE *mf;
   struct mlistd *m;

   mf = NULL;

   if (( LIST_FSTAB != src ) && ( LIST_MNTAB != src ))
      return(NULL);
 
   /* fprintf(stderr, "DEBUG: src = %d\n", src); */

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
   {
      perror("mpmount");
      return(NULL);
   }

   if ( NULL == (m = (struct mlistd *)malloc(sizeof(struct mlistd))) )
      return(NULL);

   if ( NULL == (m->sfs = (struct statvfs *)malloc(sizeof(struct statvfs))))
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
   m->mount_device = NULL;
   m->mount_fstype = NULL;
   m->mount_options = NULL;

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

   if(statvfs(m->mnt->mnt_dir, m->sfs))
      return(1);

   m->mount_point = m->mnt->mnt_dir;
   m->mount_device = m->mnt->mnt_fsname;
   m->mount_fstype = m->mnt->mnt_type;
   m->mount_options = m->mnt->mnt_opts;
#endif

#if defined(PORT_SunOS)
   if ( LIST_FSTAB == m->list_src )
   {
      if ( getvfsent(m->mf, m->vfs) )
         return(1);

      m->mount_point = m->vfs->vfs_mountp;

      if ( m->vfs->vfs_special )
         m->mount_device = m->vfs->vfs_special;
      else
         m->mount_device = m->nada;

      if ( m->vfs->vfs_fstype )
         m->mount_fstype = m->vfs->vfs_fstype;
      else
         m->mount_fstype = m->nada;

      if ( m->vfs->vfs_mntopts )
         m->mount_options = m->vfs->vfs_mntopts;
      else
         m->mount_options = m->nada;
   }

   if ( LIST_MNTAB == m->list_src )
   {
      if ( getmntent(m->mf, m->mnt) )
         return(1);

      m->mount_point = m->mnt->mnt_mountp;
      m->mount_device = m->mnt->mnt_special;
      m->mount_fstype = m->mnt->mnt_fstype;
      m->mount_options = m->mnt->mnt_mntopts;
   }
#endif

   if ( NULL == m->mount_point )
   {
      m->mount_point = m->nada;

      bzero(m->sfs, sizeof(struct statvfs));

      return(0);
   }

   if(statvfs(m->mount_point, m->sfs))
      return(1);

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

   free(m->sfs);
   free(m);

   return(0);
}

