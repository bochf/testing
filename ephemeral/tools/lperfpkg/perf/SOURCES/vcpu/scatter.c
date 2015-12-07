#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>

#include "scatter.h"

#define ANSI_RED          31
#define ANSI_GREEN        32
#define ANSI_YELLOW       33
#define ANSI_BLUE         34
#define ANSI_MAGENTA      35
#define ANSI_CYAN         36
#define ANSI_WHITE        37
#define ANSI_NORMAL        0

/* 256 color mode 
   Used as: 
   Foreground : '<ESC>[38;5;<int>m'
   Background : '<ESC>[48;5;<int>m'
*/
#define ANSI_256_RED     196
#define ANSI_256_ORANGE  202
#define ANSI_256_LORANGE 214
#define ANSI_256_YELLOW  190
#define ANSI_256_GREEN    35
#define ANSI_256_GREY    241

#define ANSI_BOLD          1
#define ANSI_UNDERLINE     4
/* ========================================================================= */
void SetColor(int c)
{
  printf("%c[%dm", 27, c);
}

/* ========================================================================= */
void show_timestamp(void)
{
   time_t t;
   struct tm *ts;

   time(&t);
   ts = localtime(&t);

   /* Apparently asctime includes EOL. Do not include one here. */
   printf("TIME: %s", asctime(ts));
}

/* ========================================================================= */
/* cprintf - printf that takes a ANSI_* define as the first input            */
void cprintf(int mode, int c, const char *format, ...)
{
   va_list ap;

   if(mode == O_COLOR_MANY) 
   {
      if(c != ANSI_UNDERLINE)
         printf("%c[48;5;16m%c[38;5;%dm", 27, 27, c);
      else
         printf("%c[48;5;16m%c[%dm", 27, 27, c);
   }
   else 
      printf("%c[%dm", 27, c);
   
   va_start(ap, format);
   vprintf(format, ap);
   va_end(ap);
   
   if(mode == O_COLOR_MANY)
   {
      if(c != ANSI_UNDERLINE)
         printf("%c[38;5;231m", 27);
      else 
         printf("%c[0m%c[48;5;16m%c[38;5;231m", 27, 27, 27);
   }
   else
      printf("%c[0m", 27);
}

/* ========================================================================= */
char *factor_ull(char *istr, unsigned long long iull)
{
   unsigned long long full = iull;
   char f[5];
   int i = 0;

   f[0] = ' ';
   f[1] = 'K';
   f[2] = 'M';
   f[3] = 'B';
   f[4] = 'T';
   
   if(NULL == istr)
   {
      if(NULL == (istr = (char *)malloc(16)))
      {
         fprintf(stderr, "ERROR: Failed to allocate memory.\n");
         exit(1);
      }
   }

   while(full >= 10000)
   {
      full /= 1000;
      i++;
   }

   if(i > 4)
      istr[0] = 0;
   else
      sprintf(istr, "%llu%c", full, f[i]);

   return(istr);
}

/* ========================================================================= */
int print_cpu_header(struct options *o, struct cpu_list *c)
{
   if(NULL == o)
      return(0);

   if(NULL == c)
      return(0);

   if ( o->column_output == O_COLS_BASIC )
   {
      if(O_COLOR_NONE == o->color_output)
      {
         printf("CPU     idle user  sys wait");
      }
      else
      {
         if(o->color_output == O_COLOR_MANY) 
            printf("%c[0m%c[48;5;16m%c[38;5;231m", 27, 27, 27);

         cprintf(o->color_output, ANSI_UNDERLINE, "CPU     idle user  sys wait");
      }
   }

   if ( o->column_output == O_COLS_EXTENDED )
   {
      if(O_COLOR_NONE == o->color_output)
      {
         /*                 x    x    x    x    x    x    x    x */
         printf("CPU     idle user nice  sys wait  irq sirq stea");
      }
      else
      {
         if(o->color_output == O_COLOR_MANY) 
            printf("%c[0m%c[48;5;16m%c[38;5;231m", 27, 27, 27);

         cprintf(o->color_output, ANSI_UNDERLINE, "CPU     idle user nice  sys wait  irq sirq stea");
      }
   }

   if ( o->column_output == O_COLS_ALL )
   {
      if(O_COLOR_NONE == o->color_output)
      {
         /*                 x    x    x    x    x    x    x    x    x    x */
         printf("CPU     idle user nice  sys wait  irq sirq stea  gst gstn");
      }
      else
      {
         if(o->color_output == O_COLOR_MANY) 
            printf("%c[0m%c[48;5;16m%c[38;5;231m", 27, 27, 27);

         cprintf(o->color_output, ANSI_UNDERLINE, "CPU     idle user nice  sys wait  irq sirq stea  gst gstn");
      }
   }

   return(1);
}

/* ========================================================================= */
int print_cpu_totals(struct cpu_list *c)
{
   if(NULL == c)
      return(0);

   printf("%-7s %3.0f%% %3.0f%% %3.0f%% %3.0f%%\n",
          "LogTot",
          c->totals->idle,
          c->totals->user,
          c->totals->sys,
          c->totals->wait);

   return(1);
}

/* ========================================================================= */
int print_cpu_line(struct options *o, struct cpu_list *c, int cpui)
{
   int color = ANSI_NORMAL;

   if(NULL == o)
      return(0);

   if(NULL == c)
      return(0);

   /* Print spaces, and exit if we are beyond the end of the array */
   if(cpui > c->count)
   {
      printf("                           ");
      
      /* STUB - if alldata */

      return(0);
   }

   switch(o->color_output)
   {
   case O_COLOR_NONE:
      color = ANSI_NORMAL;
      break;
   case O_COLOR_SIMPLE:
      color = ANSI_GREEN;
      break;
   case O_COLOR_FRUITY:
      color = ANSI_CYAN;
      break;
   case O_COLOR_MANY:
      color = ANSI_256_GREY;
      break;
   }

   if(o->color_output == O_COLOR_SIMPLE || o->color_output == O_COLOR_FRUITY)
   { 
      if(c->derived[cpui].idle < 99)
         color = ANSI_GREEN;

      if(c->derived[cpui].idle < 66)
         color = ANSI_YELLOW;

      if(c->derived[cpui].idle < 33)
         color = ANSI_RED;

      if(o->color_output == O_COLOR_FRUITY)
      {
         if(c->derived[cpui].idle < 1)
            color = ANSI_MAGENTA;
      }
   }
   else if(o->color_output == O_COLOR_MANY)
   {  
      if(c->derived[cpui].idle < 100)
         color = ANSI_256_GREEN;
      if(c->derived[cpui].idle < 51)
         color = ANSI_256_YELLOW;
      if(c->derived[cpui].idle < 26)
         color = ANSI_256_LORANGE;
      if(c->derived[cpui].idle < 11)
         color = ANSI_256_ORANGE;
      if(c->derived[cpui].idle < 2)
         color = ANSI_256_RED;
   }


   /* Set BOLD (or potential other characteristic */
   if(c->ANSI[cpui])
      printf("%c[%dm", 27, c->ANSI[cpui]);

   /* Set BOLD to all CPUs 
      Note we skip 'painting' the CPU if another characteristic has
      been set. This is to avoid overrinding whatever was set, and
      to avoid a duplicate printf of the same character */
   else if(o->bold_on && !c->ANSI[cpui])
      printf("%c[1m", 27);

   /* Print the actual line */
   if ( o->column_output == O_COLS_BASIC )
   {
      if(O_COLOR_NONE == o->color_output)
      {
         printf("cpu%-4d %3.0f%% %3.0f%% %3.0f%% %3.0f%%",
                c->derived[cpui].instance,
                c->derived[cpui].idle,
                c->derived[cpui].user,
                c->derived[cpui].sys,
                c->derived[cpui].wait);
      }
      else
      {
         cprintf(o->color_output, color, "cpu%-4d %3.0f%% %3.0f%% %3.0f%% %3.0f%%",
                 c->derived[cpui].instance,
                 c->derived[cpui].idle,
                 c->derived[cpui].user,
                 c->derived[cpui].sys,
                 c->derived[cpui].wait);
      }
   }

   if ( o->column_output == O_COLS_EXTENDED )
   {
      if(O_COLOR_NONE == o->color_output)
      {
         printf("cpu%-4d %3.0f%% %3.0f%% %3.0f%% %3.0f%% %3.0f%% %3.0f%% %3.0f%% %3.0f%%",
                c->derived[cpui].instance,
                c->derived[cpui].idle,
                c->derived[cpui].user,
                c->derived[cpui].nice,
                c->derived[cpui].sys,
                c->derived[cpui].wait,
                c->derived[cpui].irq,
                c->derived[cpui].softirq,
                c->derived[cpui].steal);
      }
      else
      {
         cprintf(o->color_output, color, "cpu%-4d %3.0f%% %3.0f%% %3.0f%% %3.0f%% %3.0f%% %3.0f%% %3.0f%% %3.0f%%",
                 c->derived[cpui].instance,
                 c->derived[cpui].idle,
                 c->derived[cpui].user,
                 c->derived[cpui].nice,
                 c->derived[cpui].sys,
                 c->derived[cpui].wait,
                 c->derived[cpui].irq,
                 c->derived[cpui].softirq,
                 c->derived[cpui].steal);
      }
   }

   if ( o->column_output == O_COLS_ALL )
   {
      if(O_COLOR_NONE == o->color_output)
      {
         printf("cpu%-4d %3.0f%% %3.0f%% %3.0f%% %3.0f%% %3.0f%% %3.0f%% %3.0f%% %3.0f%% %3.0f%% %3.0f%%",
                c->derived[cpui].instance,
                c->derived[cpui].idle,
                c->derived[cpui].user,
                c->derived[cpui].nice,
                c->derived[cpui].sys,
                c->derived[cpui].wait,
                c->derived[cpui].irq,
                c->derived[cpui].softirq,
                c->derived[cpui].steal,
                c->derived[cpui].guest,
                c->derived[cpui].guest_nice);
      }
      else
      {
         cprintf(o->color_output, color, "cpu%-4d %3.0f%% %3.0f%% %3.0f%% %3.0f%% %3.0f%% %3.0f%% %3.0f%% %3.0f%% %3.0f%% %3.0f%%",
                 c->derived[cpui].instance,
                 c->derived[cpui].idle,
                 c->derived[cpui].user,
                 c->derived[cpui].nice,
                 c->derived[cpui].sys,
                 c->derived[cpui].wait,
                 c->derived[cpui].irq,
                 c->derived[cpui].softirq,
                 c->derived[cpui].steal,
                 c->derived[cpui].guest,
                 c->derived[cpui].guest_nice);
      }
   }
   

   return(1);
}


/* ========================================================================= */
int DisplayCPU(struct options *o, struct cpu_list *c)
{
   int cc;           /* CPU counter */
   int thrdcols;     /* This is the "natural" columns of the output as
                        determined by the "threads" of the CPU arch. This is
                        set at the top, and referenced after. */
   int tcol = 0;     /* This is the counter for the thread columns */
   int colmax;
   int col;
   int row;
   int col_offset;

   cc = 0;

   if(NULL == c)
      return(1);

   if(NULL == o)
      return(1);

   /*
   printf("CPU     idle  user sys wait");
   printf("   CPU     idle  user sys wait   CPU     idle  user sys wait\n");
   */

   printf("\n");

   if(o->show_timestamp)
      show_timestamp();

   /*** This displays the header ***/

   colmax = o->columns;
   thrdcols = o->smt_columns;

   col = 0;
   while(col < colmax)
   {
     if(col > 0)  
       printf("%c[0m    ", 27);

     tcol = 0;
     while(tcol < thrdcols)
     {
        if(tcol > 0)
           printf("   ");

        print_cpu_header(o, c);
        tcol++;
     }
     col++;
   }
   printf("%c[0m\n", 27);

   /*** This displays the data ***/
   
   /* Derive the offset for the beginning of each column */
   col_offset = c->count / o->columns;
   while(col_offset % o->smt_columns != 0)
      col_offset++;

   cc = 0;
   row = 0;
   while(cc < c->count)
   {
      col = 0;
      while(col < colmax)
      {
         if(col > 0) 
          printf("%c[0m    ", 27);

         tcol = 0;
         while(tcol < thrdcols)
         {
            if (tcol > 0)
               printf(" | ");

            print_cpu_line(o, c, row + (col * col_offset) + tcol);
            cc++;
            tcol++;
         }
         col++;
      }
      row += tcol;

      printf("%c[0m\n", 27);
   }

   if(o->show_totals)
   {
      print_cpu_totals(c);
   }


   /* Make sure it gets out */
   fflush(stdout);


     /* How this works:
        Work backwards through the colors percentages... using idle time as the
        key to the color. The breakdown is as follows:

         Color (%% busy) mode breakdown\n");
          -m(onochrome)\n");
          -d(ull / three)\n");
             0-32     green\n");
             33-65    yellow\n");
             66-100   red\n");
          -f(ull/ive/ruity)\n");
             0        cyan (aqua-blue)\n");
             1-32     green\n");
             33-65    yellow\n");
             66-98    red\n");
             99-100   magenta (pinkish-red)\n");
          -j(ose's scheme) <--- Default\n");
             0        grey\n");
             1-49     green\n");
             50-74    yellow\n");
             75-89    light orange\n");
             90-98    orange\n");
             99-100   red\n");
     */
 

   return(0);

}

/* ========================================================================= */
/* Determines if the system has sufficient CPUs to fufill the command line
   requests of the user */
int NotDisplayable(struct options *o, struct cpu_list *c)
{
 if(NULL == o)
    return(1);
  
  if(NULL == c)
    return(1);

  if(c->count < o->columns * o->smt_columns)
    return(1);
  else
    return(0);
}



