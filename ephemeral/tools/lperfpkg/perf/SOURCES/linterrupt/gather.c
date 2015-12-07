
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include "gather.h"
#include "support.h"


int get_optimal_buffer(struct interrupts *intr);
int read_in_file(struct interrupts *intr);
int count_cpus(struct interrupts *intr);
char *get_a_line(struct interrupts *intr, char *last_line);
int build_data_structures(struct interrupts *intr);
int new_interrupt(struct interrupts *intr, char *tag, char *type, char *label);
int fill_data_structures(struct interrupts *intr);
struct interrupt *find_interrupt(struct interrupts *intr, char *tag);
int interrupt_data_swap(struct interrupts *intr);
Interrupt parse_single_value(char *line);

/* ========================================================================= */
struct interrupts *InitIntrData(struct options *o)
{
   struct interrupts *intr;

   if ( NULL == ( intr = (struct interrupts *)malloc(sizeof(struct interrupts)) ) )
   {
      fprintf(stderr, "ERROR: Unable to allocate memory for primary data structure.\n");
      return(NULL);
   }

   /* Initialize values */
   intr->buf = NULL;
   intr->bufsz = 0;
   intr->lrs = 0;
   intr->cpucnt = 0;
   intr->ilist = NULL;
   /* Transfer over the user supplied options */
   intr->bTimestamp = o->bTimestamp;
   intr->bHeader = o->bHeader;
   intr->bCollapse = o->bCollapse;
   intr->bMonochrome = o->bMonochrome;
   intr->bNames = o->bNames;
   intr->bShortNames = o->bShortNames;
   intr->bSkipLOC = o->bSkipLOC;
   intr->bCPUStats = o->bCPUStats;

   if ( o->interval >= 1 )
      intr->interval = o->interval; /* Convert to Interrupt type */
   else
      intr->interval = 1; /* Effectively a silent assert() */

   /* If the file has not been read, then the following calls will read
      it in. So there is no need to call read_in_file() here. */
   
   if ( get_optimal_buffer(intr) )
      return(NULL); /* Handle the errors at point of failure */

   if ( count_cpus(intr) )
      return(NULL); /* Handle the errors at point of failure */

   if ( build_data_structures(intr) )
      return(NULL); /* Handle the errors at point of failure */

   if ( NULL == intr->ilist )
   {
      fprintf(stderr, "ERROR: Internal problem parsing /proc/interrupts.\n");
      return(NULL);
   }

   if ( fill_data_structures(intr) )
      return(NULL); /* Handle the errors at point of failure */

   /* Conditionally allocate memory for derived CPU data */
   if ( intr->bCPUStats )
   {
      if ( NULL == ( intr->cpu_d = (struct cpuderived *)malloc(sizeof(struct cpuderived) * intr->cpucnt) ) )
      {
         return(NULL);
      }
   }

   return(intr);
}

/* ========================================================================= */
int SampleStats(struct interrupts *intr)
{
   if ( read_in_file(intr) )
      return(1);

   if ( fill_data_structures(intr) )
      return(1); /* Handle the errors at point of failure */

   return(0);
}

/* ========================================================================= */
int read_in_file(struct interrupts *intr)
{
   int f;
   ssize_t got;
   ssize_t total;
   int go;
   
   if ( -1 == (f = open("/proc/interrupts", O_RDONLY)) )
      return(1);

   go = 1;
   total = 0;
   while ( go )
   {
      got = read(f, intr->buf + total, intr->bufsz - total);

      if ( got > 0 )
      {
         total += got;
      }
      else
      {
         if ( 0 == got )
         {
            intr->lrs = total;
            go = 0;
         }
         else
         {
            if ( errno != EINTR )
            {
               close(f);
               fprintf(stderr, "ERROR: Failed on a read from /proc/interrupts.\n");
               return(1);
            } /* if ( errno ) */
         } /* if ( got == 0 ) else... */
      } /* if( got > 0 ) else ... */
   } /* while(go) */

   close(f);

   return(0);
}

/* ========================================================================= */
int count_cpus(struct interrupts *intr)
{
   int cpucnt;
   int i;

   /* If you got no data, then read it in */
   if ( 0 == intr->lrs )
   {
      if ( read_in_file(intr) )
         return(1);
   }

   /* This is kind of sub-optimal, but we only call it once */
   cpucnt = 0;
   i = 3;
   while ( i < intr->lrs )
   {
      if ( ( intr->buf[i - 3] == 'C' ) &&
           ( intr->buf[i - 2] == 'P' ) &&
           ( intr->buf[i - 1] == 'U' ) &&
           ( ( intr->buf[i] >= '0' ) && ( intr->buf[i] <= '9' ) ) ) 
         cpucnt++;

      i++;
   }

   if ( cpucnt == 0 )
      return(1);

   intr->cpucnt = cpucnt;

   return(0);
}

/* ========================================================================= */
int get_optimal_buffer(struct interrupts *intr)
{
   int f;
   char buf[1024];
   ssize_t got;
   ssize_t total;
   ssize_t optimal;
   int go;

   if ( -1 == (f = open("/proc/interrupts", O_RDONLY)) )
      return(1);

   total = 0;
   go = 1;
   while ( go ) 
   {
      got = read(f, buf, 1024);

      if ( got > 0 )
      {
         total += got;
      }
      else
      {
         if ( 0 == got )
         {
            /* Crazy small size */
            optimal = 1024;

            /* Get in the ballpark */
            while ( optimal < total )
               optimal += 1024;

            /* Add some breathing room */
            if ( optimal - total < 256 )
               optimal += 1024;

            /* Round to "even" boundary */
            if ( (optimal / 1024) % 2 != 0 )
               optimal += 1024;

            intr->bufsz = optimal;

            if ( NULL == (intr->buf = (char *)malloc(intr->bufsz)) )
            {
               close(f);
               fprintf(stderr, "ERROR: Unable to allocate memory for read buffer.\n");
               return(1);
            }

            go = 0;
         }
         else /* got is negative */
         {
            if ( errno != EINTR )
            {
               close(f);
               fprintf(stderr, "ERROR: Failed on a read from /proc/interrupts.\n");
               return(1);
            }
         } /* if ( got == 0 ) else ... */
      } /* if ( got > 0 ) else ... */
   } /* while go */

   close(f);

   return(0);
}

/* ========================================================================= */
char *get_a_line(struct interrupts *intr, char *last_line)
{
   char *eof;

   if ( NULL == last_line )
      return(intr->buf);

   eof = intr->buf + intr->lrs;

   while ( last_line < eof )
   {
      if ( '\n' == *last_line )
      {
         if ( last_line + 1 < eof )
            return(last_line + 1);
         else
            return(NULL);
      }

      last_line++;
   }

   return(NULL);
}

/* ========================================================================= */
int new_interrupt(struct interrupts *intr, char *tag, char *type, char *label)
{
   struct interrupt *one_intr;

   if ( NULL == ( one_intr = (struct interrupt *)malloc(sizeof(struct interrupt)) ) )
   {
      fprintf(stderr, "ERROR: Unable to allocate memory for interrupt struct.\n");
      return(1);
   }

   /* Save the tag */
   one_intr->tag[0] = tag[0];
   one_intr->tag[1] = tag[1];
   one_intr->tag[2] = tag[2];

   one_intr->type = mkstring(type);
   one_intr->label = mkstring(label);

   one_intr->next = NULL;

   one_intr->this_ic = (Interrupt *)malloc(sizeof(Interrupt) * (intr->cpucnt + 1));
   one_intr->last_ic = (Interrupt *)malloc(sizeof(Interrupt) * (intr->cpucnt + 1));
   one_intr->diff_ic = (Interrupt *)malloc(sizeof(Interrupt) * (intr->cpucnt + 1));

   if (( NULL == one_intr->this_ic ) || ( NULL == one_intr->last_ic ) || ( NULL == one_intr->diff_ic ))
   {
      fprintf(stderr, "ERROR: Unable to allocate memory for interrupt array.\n");
      return(1);
   }

   /* Zero-out all the arrays we just created */
   memset(one_intr->this_ic, 0, sizeof(Interrupt) * (intr->cpucnt + 1));
   memset(one_intr->last_ic, 0, sizeof(Interrupt) * (intr->cpucnt + 1));
   memset(one_intr->diff_ic, 0, sizeof(Interrupt) * (intr->cpucnt + 1));

   /* Into the linked list */
   one_intr->next = intr->ilist;
   intr->ilist = one_intr;

   return(0);
}

/* ========================================================================= */
Interrupt parse_single_value(char *line)
{
   Interrupt value = 0;

   while (( *line != '\n' ) && ( *line != 0 ))
   {
      if (( *line >= '0' ) && ( *line <= '9' ))
      {
         value *= 10;
         value += *line - '0';
      }

      line++;
   }

   return(value);
}

/* ========================================================================= */
int non_numeric_tag(char *tag)
{
   /* Start with the item most likely not a space */
   if ( ( tag[0] >= 'A' ) && ( tag[0] <= 'Z' ) )
      return(1);

   if ( ( tag[1] >= 'A' ) && ( tag[1] <= 'Z' ) )
      return(1);

   if ( ( tag[2] >= 'A' ) && ( tag[2] <= 'Z' ) )
      return(1);

   return(0);
}

/* ========================================================================= */
#define LABEL_SZ 64
#define TYPE_SZ  64
int build_data_structures(struct interrupts *intr)
{
   char *line;
   char tag[3];
   char type[TYPE_SZ];
   char label[LABEL_SZ];
   int i;
   int j, k;
   int insert_interrupt;
   int cpu;

   assert( NULL != intr );

   line = NULL; /* NULL input starts input at beginning of buffer */
   i = 0;
   while ( NULL != (line = get_a_line(intr, line)) )
   {
      /* Some versions have an extra space - yank that */
      if ( ':' == line[4] )
         line++;

      if ( ':' == line[3] )
      {
         /* Copy off the "tag" */
         tag[0] = line[0];
         tag[1] = line[1];
         tag[2] = line[2];

         type[0] = 0;
         label[0] = 0;

         /* Skip through data for now */
         j = 4;
         while (( line[j] == ' ' ) || ((line[j] >= '0') && (line[j] <= '9')))
            j++;
         
         /* Here is the point: The non-standard-interrupt lines *TEND* to 
            have non-standard (free-text) labels. The non-standard interrupt
            lines *TEND* to be in all caps (instead of right justtified 
            numbers. */
         if ( non_numeric_tag(tag) )
         {
            if ( line[j] != '\n' )
            {
               while(line[j] == ' ')
                  j++;

               k = 0;
               while(line[j] != '\n')
               {
                  if ( k < LABEL_SZ )
                     label[k] = line[j];

                  k++;
                  j++;
               }

               if ( k < LABEL_SZ )
                  label[k] = 0;
               else
               {
                  label[LABEL_SZ - 2] = '!';
                  label[LABEL_SZ - 1] = 0;
               }
            }
         }
         else /* It is a numeric */
         {
            if ( line[j] != '\n' )
            {
               k = 0;
               while((line[j] != ' ') && (line[j] != '\n'))
               {
                  if ( k < TYPE_SZ )
                     type[k] = line[j];
                  
                  k++;
                  j++;
               }
               
               if ( k < TYPE_SZ )
                  type[k] = 0;
               else
               {
                  type[TYPE_SZ - 2] = '!';
                  type[TYPE_SZ - 1] = 0;
               }
               
               while(line[j] == ' ')
                  j++;
               

               k = 0;
               while(line[j] != '\n')
               {
                  if ( k < LABEL_SZ )
                     label[k] = line[j];

                  k++;
                  j++;
               }

               if ( k < LABEL_SZ )
                  label[k] = 0;
               else
               {
                  label[LABEL_SZ - 2] = '!';
                  label[LABEL_SZ - 1] = 0;
               }
            } /* if line[j] != \n ---- data left in line */
         } /* if ( non-numeric) else is numeric */

         insert_interrupt = 1;

         if ( ( tag[0] == 'E' ) && ( tag[1] = 'R' ) && ( tag[2] = 'R' ) )
         {
            insert_interrupt = 0;
            intr->ERR = parse_single_value(&line[4]);
         }

         if ( ( tag[0] == 'M' ) && ( tag[1] = 'I' ) && ( tag[2] = 'S' ) )
         {
            insert_interrupt = 0;
            intr->MIS = parse_single_value(&line[4]);
         }

         if ( insert_interrupt )
            if ( new_interrupt(intr, tag, type, label) )
               return(1);
      }

      i++;
   }

   if ( NULL == (intr->totl_ic = (Interrupt *)malloc(sizeof(Interrupt) * (intr->cpucnt + 1))) )
   {
      fprintf(stderr, "ERROR: Unable to allocate memory for interrupt total list.\n");
      return(1);
   }

   /* Set all values to 0 */
   memset(intr->totl_ic, 0, sizeof(Interrupt) * (intr->cpucnt + 1));

   /* Pre-generate all the CPU names (labels) to be used later */
   intr->cpunames = (char *)malloc( CPU_STR_SZ * intr->cpucnt );
   cpu = 0;
   while ( cpu < intr->cpucnt )
   {
      sprintf(&intr->cpunames[CPU_STR_SZ * cpu], "CPU%d", cpu);
      cpu++;
   }

   return(0);
}

/* ========================================================================= */
struct interrupt *find_interrupt(struct interrupts *intr, char *tag)
{
   struct interrupt *this_interrupt;

   this_interrupt = NULL;

   assert(NULL != intr);
   assert(NULL != tag);

   this_interrupt = intr->ilist;

   while ( this_interrupt )
   {
      if ( ( this_interrupt->tag[0] == tag[0] ) &&
           ( this_interrupt->tag[1] == tag[1] ) &&
           ( this_interrupt->tag[2] == tag[2] ) )
      {
         return(this_interrupt);
      }

      this_interrupt = this_interrupt->next;
   }

   return(NULL);
}

/* ========================================================================= */
int interrupt_data_swap(struct interrupts *intr)
{
   struct interrupt *this_int;
   Interrupt *temp_ic;

   assert(NULL != intr);

   this_int = intr->ilist;

   while ( this_int )
   {
      /* assert() in a loop kind of sucks, but it will optimize out */
      assert( NULL != this_int->this_ic );
      assert( NULL != this_int->last_ic );

      temp_ic = this_int->this_ic;
      this_int->this_ic = this_int->last_ic;
      this_int->last_ic = temp_ic;
      
      this_int = this_int->next;
   }

   return(0);
}

/* ========================================================================= */
int lay_in_interrupts(struct interrupt *thisi, char *line)
{
   int cpu;
   Interrupt icount;

   while (( *line != ':' ) && ( ( *line != 0 ) || ( *line != '\n') ))
      line++;

   /* We should *not* be sitting on EOL */
   if ( *line == 0 )
      return(0);

   line++; /* move off the ':' character */

   cpu = 0;
   while ( ( *line == ' ' ) || ( ( *line >= '0' ) && ( *line <= '9' ) ) )
   {
      while ( *line == ' ' )
         line++;

      icount = 0;
      while ( ( *line >= '0' ) && ( *line <= '9' ) )
      {
         icount *= 10;
         icount += ( *line - '0' );

         line++;
      }

      thisi->this_ic[cpu++] = icount;

      /* Chomp up any white space prior to next loop */
      while ( *line == ' ' )
         line++;
   }

   return(cpu);
}

/* ========================================================================= */
int fill_data_structures(struct interrupts *intr)
{
   char *line;
   char tag[3];
   struct interrupt *this_int;
   int cpucnt;
   int insert_interrupt;

   assert(NULL != intr);
   assert(NULL != intr->ilist);

   /* Swap the arrays make current data the previous */
   if ( interrupt_data_swap(intr) )
      return(1);

   line = NULL;

   while ( NULL != (line = get_a_line(intr, line)) )
   {
      if ( ':' == line[4] ) /* Some releases have an extra space - remove that */
         line++;

      if ( ':' == line[3] )
      {
         /* Copy off the "tag" */
         tag[0] = line[0];
         tag[1] = line[1];
         tag[2] = line[2];

         insert_interrupt = 1;

         if ( ( tag[0] == 'E' ) && ( tag[1] = 'R' ) && ( tag[2] = 'R' ) )
         {
            insert_interrupt = 0;
            intr->ERR = parse_single_value(&line[4]);
         }
         
         if ( ( tag[0] == 'M' ) && ( tag[1] = 'I' ) && ( tag[2] = 'S' ) )
         {
            insert_interrupt = 0;
            intr->MIS = parse_single_value(&line[4]);
         }


         if ( insert_interrupt )
         {
            if ( NULL == ( this_int = find_interrupt(intr, tag) ) )
            {
               fprintf(stderr, "ERROR: New interrupt found. Unable to process \'%c%c%c\'.\n", tag[0], tag[1], tag[2]);
               /* This is kind of stub-ish. If you find a new interrupt 
                  why not just add it? */
               
               /* Also... if you just made tag 4 characters and terminated
                  the last, life would be much easier. */
               
               return(1);
            }
            
            if ( intr->cpucnt != (cpucnt = lay_in_interrupts(this_int, line)) )
            {
               fprintf(stderr, "ERROR: Unable to account for %d CPUs for %c%c%c.\n",  intr->cpucnt - cpucnt, tag[0], tag[1], tag[2]);
               fprintf(stderr, "       This system has %d CPUs, I found %d.\n", intr->cpucnt, cpucnt);
               return(1);
            }
         }

      } /* if ":" -- a tag line */
   } /* while ( get_a_line() ) */

   return(0);
}

