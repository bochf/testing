#include <stdio.h>
#include <stdlib.h>

#include "mntlist.h"

int main(int argc, char *argv[])
{
   struct mlistd *m;
   int src = LIST_MNTAB;

   if ( argc == 2 )
   {
      //fprintf(stderr, "INFO: Setting src to \"%s\".\n", argv[1]);
      src = atoi(argv[1]);
   }

   switch(src)
   {
   case LIST_MNTAB:
   case LIST_FSTAB:
      break;
   default:
      src = LIST_MNTAB;
      break;
   }

   if (NULL == (m = open_mount_list(src)))
   {
      fprintf(stderr, "ERROR: Unable to open mount list.\n");
      return(1);
   }

   while(0 == next_mount_item(m))
   {
      fprintf(stdout, "---------------------------------------------\n");
      fprintf(stdout, "Mount Point : %s\n", m->mount_point);
      fprintf(stdout, "Mount Device: %s\n", m->mount_device);
      fprintf(stdout, "FS Type     : %s\n", m->mount_fstype);
      fprintf(stdout, "Mount Opts  : %s\n", m->mount_options);
      fprintf(stdout, "frsize      : %llu\n", (unsigned long long)m->sfs->f_frsize);
      fprintf(stdout, "bfree       : %llu\n", (unsigned long long)m->sfs->f_bfree);
   }


   close_mount_list(m);

   return(0);
}
