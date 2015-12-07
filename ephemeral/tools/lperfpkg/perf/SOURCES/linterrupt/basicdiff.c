#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "basicdiff.h"

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
inline void set_color(int color)
{
   if ( color != ANSI_NORMAL )
      printf("%c[38;5;%dm", 27, color);
}

/* ========================================================================= */
inline void clear_color(int color)
{
   if ( color != ANSI_NORMAL )
      printf("%c[%dm", 27, ANSI_NORMAL);
}

/* ========================================================================= */
inline int assign_cpu_color(float idle, int bNoColor)
{
   if ( bNoColor )
      return(ANSI_NORMAL);

   if ( idle < 2 )
      return(ANSI_256_RED);

   if ( idle < 11 )
      return(ANSI_256_ORANGE);

   if ( idle < 26 )
      return(ANSI_256_LORANGE);

   if ( idle < 51 )
      return(ANSI_256_YELLOW);

   if ( idle < 100 )
      return(ANSI_256_GREEN);

   return(ANSI_256_GREY);
}

/* ========================================================================= */
/* Set the color of the interrupt label & tag */
inline int assign_intrtag_color(struct interrupts *intr, Interrupt i)
{
   if ( intr->bMonochrome == 1 )
      return(ANSI_NORMAL);

   if ( i >= intr->intr_rzone )
      return(ANSI_256_RED);

   if ( i >= intr->intr_ozone )
      return(ANSI_256_ORANGE);

   if ( i >= intr->intr_yzone )
      return(ANSI_256_YELLOW);

   if ( i > 0 )
      return(ANSI_256_GREEN);

   return(ANSI_256_GREY);
}

/* ========================================================================= */
/* Set the color of the interrupt (individual) cell */
inline int assign_intrcell_color(struct interrupts *intr, Interrupt i)
{
   if ( intr->bMonochrome == 1 )
      return(ANSI_NORMAL);

   if ( i == 0 )
      return(ANSI_256_GREY);

   return(ANSI_NORMAL);
}

/* ========================================================================= */
/* Set the color of the CPU label & total */
inline int assign_cputotal_color(struct interrupts *intr, Interrupt i)
{
   if ( intr->bMonochrome == 1 )
      return(ANSI_NORMAL);

   if ( i >= intr->cpu_rzone )
      return(ANSI_256_RED);

   if ( i >= intr->cpu_ozone )
      return(ANSI_256_ORANGE);

   if ( i >= intr->cpu_yzone )
      return(ANSI_256_YELLOW);

   if ( i > 0 )
      return(ANSI_256_GREEN);

   return(ANSI_256_GREY);
}            

/* ========================================================================= */
int DisplayBasicDiff(struct interrupts *intr)
{
   struct interrupt *one_intr;
   int cpu;
   int c;   /* Used to hold the color */
   int ic;  /* Used to temporarily hold the color (see notes below) */
   time_t t;

   assert(NULL != intr);

   if ( intr->bTimestamp )
   {
      time(&t);
      printf("%s", ctime(&t));
   }

   /* --- Header --- */
   if ( intr->bHeader )
   {
      printf("     ");

      cpu = 0;
      while ( cpu < intr->cpucnt )
      {
         c = assign_cputotal_color(intr, intr->totl_ic[cpu]);
         
         set_color(c);

         printf(" %7s", &intr->cpunames[CPU_STR_SZ * cpu]);

         /* Turn us back to normal ANSI output */
         clear_color(c);
         
         cpu++;
      }

      /* Print total interrupts of this type */
      printf ("   TOTAL\n");
   }      

   /* --- CPU usage stats --- */
   if ( intr->bCPUStats )
   {
      printf("%%IDLE");
      cpu = 0;
      while ( cpu < intr->cpucnt )
      {
         intr->cpu_d[cpu].color = assign_cpu_color(intr->cpu_d[cpu].idle, intr->bMonochrome);
         set_color(intr->cpu_d[cpu].color);
         printf ("     %3.0f", intr->cpu_d[cpu].idle);
         clear_color(intr->cpu_d[cpu].color);
         cpu++;
      }
      printf("\n");

      printf("%%NICE");
      cpu = 0;
      while ( cpu < intr->cpucnt )
      {
         set_color(intr->cpu_d[cpu].color);
         printf ("     %3.0f", intr->cpu_d[cpu].nice);
         clear_color(intr->cpu_d[cpu].color);
         cpu++;
      }
      printf("\n");

      printf("%%USER");
      cpu = 0;
      while ( cpu < intr->cpucnt )
      {
         set_color(intr->cpu_d[cpu].color);
         printf ("     %3.0f", intr->cpu_d[cpu].user);
         clear_color(intr->cpu_d[cpu].color);
         cpu++;
      }
      printf("\n");

      printf("%% SYS");
      cpu = 0;
      while ( cpu < intr->cpucnt )
      {
         set_color(intr->cpu_d[cpu].color);
         printf ("     %3.0f", intr->cpu_d[cpu].sys);
         clear_color(intr->cpu_d[cpu].color);
         cpu++;
      }
      printf("\n");

      printf("%%WAIT");
      cpu = 0;
      while ( cpu < intr->cpucnt )
      {
         set_color(intr->cpu_d[cpu].color);
         printf ("     %3.0f", intr->cpu_d[cpu].wait);
         clear_color(intr->cpu_d[cpu].color);
         cpu++;
      }
      printf("\n");

      printf("%%SIRQ");
      cpu = 0;
      while ( cpu < intr->cpucnt )
      {
         set_color(intr->cpu_d[cpu].color);
         printf ("     %3.0f", intr->cpu_d[cpu].softirq);
         clear_color(intr->cpu_d[cpu].color);
         cpu++;
      }
      printf("\n");

      printf("%% IRQ");
      cpu = 0;
      while ( cpu < intr->cpucnt )
      {
         set_color(intr->cpu_d[cpu].color);
         printf ("     %3.0f", intr->cpu_d[cpu].irq);
         clear_color(intr->cpu_d[cpu].color);
         cpu++;
      }
      printf("\n");

      printf("%%OTHR");
      cpu = 0;
      while ( cpu < intr->cpucnt )
      {
         set_color(intr->cpu_d[cpu].color);
         printf ("     %3.0f", intr->cpu_d[cpu].steal + intr->cpu_d[cpu].guest + intr->cpu_d[cpu].guest_nice);
         clear_color(intr->cpu_d[cpu].color);
         cpu++;
      }
      printf("\n");
   }

   /* --- Interrupt Table --- */
   one_intr = intr->ilist;
   while ( one_intr )
   {
      /* Skip if LOC data should not be here */
      if (( intr->bSkipLOC ) && ( 0 == is_not_loc_intr(one_intr) ))
      {
         one_intr = one_intr->next; /* Move to the next interrupt */
         continue;                  /* Start again at top of loop */
      }

      /* Collapse this line if user wants, and the total interrupts are 0 */
      if (( 0 == intr->bCollapse ) || ( 0 != one_intr->diff_ic[intr->cpucnt] ))
      {
         c = assign_intrtag_color(intr, one_intr->diff_ic[intr->cpucnt]);
         set_color(c);

         /* print [TAG] */
         printf("[%c%c%c]",
                one_intr->tag[0],
                one_intr->tag[1],
                one_intr->tag[2]);

         /* Turn us back to normal ANSI output */
         clear_color(c);

         /* Print data for this interrupt across all CPUs */
         cpu = 0;
         while ( cpu < intr->cpucnt )
         {
            /* Here we use "ic" instead of "c". This is because the color
               coding used in c shows up in the first and last part of the
               line. We may use different colors in-between. So, instead
               of re-calculating c, we use a temporary/localized version
               of c called ic (interrupt color) that is for the individual
               interrupt-to-CPU "cell" in the table". */
            ic = assign_intrcell_color(intr, one_intr->diff_ic[cpu]);
            set_color(ic);

            printf (" %7lld", one_intr->diff_ic[cpu]);

            clear_color(ic);

            cpu++;
         }

         /* Turn color BACK ON using the saved value for c */
         set_color(c);

         /* Print total interrupts of this type */
         printf (" %7lld", one_intr->diff_ic[cpu]);
         
         if ( intr->bNames )
         {
            if ( intr->bShortNames )
            {
               if ( one_intr->label )
                  printf("   %s", one_intr->label);
            }
            else
            {
               printf("   ");

               if ( one_intr->type )
                  printf("%s ", one_intr->type);

               if ( one_intr->label)
                  printf("%s", one_intr->label);
            }
         }

         /* Turn us back to normal ANSI output */
         clear_color(c);

         printf("\n");
      }

      one_intr = one_intr->next;
   }

   /* --- Print totals --- */
   printf("TOTAL");

   cpu = 0;
   while ( cpu < intr->cpucnt )
   {
      c = assign_cputotal_color(intr, intr->totl_ic[cpu]);
            
      set_color(c);

      printf (" %7lld", intr->totl_ic[cpu]);

      clear_color(c);
         
      cpu++;
   }
   printf(" %7lld\n", intr->totl_ic[cpu]); /* Total of totals */

   printf("\n"); /* Extra LF between output blocks */
   fflush(stdout);

   return(0);
}
