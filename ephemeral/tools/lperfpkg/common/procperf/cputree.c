#include <stdio.h>
#include "procperf.h"

int extra_crap = 0;

void dump_threads(struct cpu_thread *t)
{
   struct cpu_thread *thist;

   thist = t;
   while(thist)
   {
      printf("    cpu%d", thist->id);
      
      if ( extra_crap )
      {
         if ( NULL == thist->cs )
            printf(" -> NULL\n");
         else
            printf(" -> %d\n", thist->cs->instance);
      }
      else
         printf("\n");

      thist = thist->next;
   }
}

void dump_cores(struct cpu_core *c)
{
   struct cpu_core *thisc;

   thisc = c;
   while(thisc)
   {
      printf("  core%d\n", thisc->id);
      
      dump_threads(thisc->tlist);

      thisc = thisc->next;
   }
}

void dump_sockets(struct cpu_socket *s)
{
   struct cpu_socket *thiss;

   thiss = s;
   while(thiss)
   {
      printf("socket%d\n", thiss->id);

      dump_cores ( thiss->clist );

      thiss = thiss->next;
   }
}


int main(int argc, char *argv[])
{
   struct cpustats *cpus;

   if ( argc > 1 )
      extra_crap = 1;

   if ( NULL == (cpus = GetCPUStats(NULL)) )
   {
      fprintf(stderr, "ERROR: Unable to parse CPU stats.\n");
      return(1);
   }


   if(BuildCPUTree(cpus))
   {
      fprintf(stderr, "ERROR: Issues building CPU tree.\n");
      return(1);
   }

   dump_sockets(cpus->cpu_tree);


   return(0);
}
