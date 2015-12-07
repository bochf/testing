#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "gather.h"
#include "munge.h"
#include "scatter.h"

#include "version.h"
#include "options.h"

/* This is just for the IBM compiler - to kill a warning */
int LOOP_FOREVER=1;

void DisplayVersion(void);
void DisplayUsage(void);
void DisplayError(void);

int derive_thread_count(struct cpu_list *c)
{
   int t;
   struct cpu_thread *tlist;

   if (NULL == c->thiscpus->cpu_tree)
      return(0);

   tlist = c->thiscpus->cpu_tree->clist->tlist;
   t = 0;
   while(tlist)
   {
      t++;
      tlist = tlist->next;
   }

   return(t);
}


/* ========================================================================= */
int main (int argc, char *argv[])
{
   struct options *o;
   struct cpu_list *c;

   /*** Read in options passed by command line ***/
  if(NULL == (o = ParseOptions(argc, argv)))
  {
    printf("ERROR: ParseOptions returns NULL.\n");
    return(1);
  }

  /*** Handle the most basic input requests / issues ***/
  if(o->input_error)
  {
    DisplayError();
    return(0);
  }

  if(o->show_help)
  {
    DisplayUsage();
    return(0);
  }

  if(o->show_version)
  {
    DisplayVersion();
    return(0);
  }

  /*** Now begin cpu stats display ***/
  fprintf(stderr, "vcpu version %s starting.\n", VERSION_STRING);

  if(NULL == (c = InitCPUList()))
   {
      fprintf(stderr, "ERROR: Unable to initialize the CPU info.\n");
      return(1);
   }

   if(0 == o->smt_user_set)
   {
      if ( 0 >= ( o->smt_columns = derive_thread_count(c) ) )
      {
         fprintf(stderr, "ERROR: Unable to determine thread value.\n");
         return(1);
      }
   }

   InitMungeData(o, c);

   sleep(1);

   while(LOOP_FOREVER)
   {
      FreshenCPUList(c);
      CalcLatest(o, c);
      DisplayCPU(o, c);
      sleep(o->iteration_time);
   }


#ifdef UNDEFINED


  */

   /* Insure that we can actually display what the user requested. */
   if(NotDisplayable(o, c))
   {
     fprintf(stderr, "NOTE: Unable to display using the column configuration chosen.\n");
     fprintf(stderr, "      Changing to a safe default.\n");
     o->columns = 1;
     o->smt_columns = 1;
   }

#endif

   return(0);
}

/* ========================================================================= */
void DisplayUsage(void)
{
  printf("%s - Alternative CPU display\n", APP_NAME);
  printf("   Usage:\n");
  printf("     -a        Show all columns (from /proc/stat)\n");
  printf("     -b <list> Display CPUs in bold. \"-\" for range, \",\" for multiple members.\n");
  printf("     -B        Display all CPUs in bold.\n");
  printf("     -c X      Use X columns\n");
  printf("     -d        Display three color mode (red,yellow,green)\n");
  printf("     -e        Show extended columns (from /proc/stat)\n");
  printf("     -f        Display in full/five color mode (aka: \"fruity\")\n");
  printf("     -h        Show this help\n");
  printf("     -m        Display output in monochrome\n");
  printf("     -s X      Use X thread columns (Overrides derived value)\n");
  printf("     -t        Show timestamp each iteration.\n");
  /*  printf("     -T        Show \"totals\" for the system.\n"); */
  printf("     -v        Show version info\n");
  printf("   Color (%% busy) mode breakdown\n");
  printf("    -m(onochrome)\n");
  printf("    -d(ull / three)\n");
  printf("       0-32     green\n");
  printf("       33-65    yellow\n");
  printf("       66-100   red\n");
  printf("    -f(ull/ive/ruity)\n");
  printf("       0        cyan (aqua-blue)\n");
  printf("       1-32     green\n");
  printf("       33-65    yellow\n");
  printf("       66-98    red\n");
  printf("       99-100   magenta (pinkish-red)\n");
  printf("    -j(ose's scheme) <--- Default\n");
  printf("       0        grey\n");
  printf("       1-49     green\n");
  printf("       50-74    yellow\n");
  printf("       75-89    light orange\n");
  printf("       90-98    orange\n");
  printf("       99-100   red\n");

}

/* ========================================================================= */
void DisplayVersion(void)
{
  printf("%s - Alternative CPU display\n", APP_NAME);
  printf("   Version: %s\n\n", VERSION_STRING);
  printf("   Bill Hacket <whacket@clerkenwell.bridewell.co.uk>\n");
  printf("   William Favorite <wfavorite2@bloomberg.net>\n");
}

/* ========================================================================= */
void DisplayError(void)
{
  printf("ERROR: Input not understood. Use -h option to see proper usage.\n");
}
