#ifndef MNTLIST_H
#define MNTLIST_H

#define LIST_FSTAB 1
#define LIST_MNTAB 2

#include <stdio.h>


#if defined(PORT_Linux) || defined(PORT_AIX)
#include <mntent.h>
#endif

#if defined(PORT_SunOS)
#include <sys/mnttab.h>
#include <sys/vfstab.h>
#endif

#include <sys/statvfs.h>



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
   char *mount_device;
   char *mount_fstype;
   char *mount_options;

   char nada[4];

   struct statvfs *sfs;
};




struct mlistd *open_mount_list(int src);


int next_mount_item(struct mlistd *m);


int close_mount_list(struct mlistd *m);

#endif
