#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "../support.h"
#include "hwconfig.h"

static int socket_count = 0;
static int core_count   = 0;
static int proc_count   = 0;
static int thr_per_core = 0;
static char bogomips[32];
static char speed[32];

/* Utility prototypes */
int get_colon_value(char *line);
char *get_colon_str(char *line);

/* ========================================================================= */
long GetCPUSocketCount(char *status, unsigned long size)
{
   /* A reasonable limitation on length */
   if ( size < 8 )
      return(0);

   return(sprintf(status, "%d", socket_count));
}

/* ========================================================================= */
long GetCPUCoreCount(char *status, unsigned long size)
{
   /* A reasonable limitation on length */
   if ( size < 8 )
      return(0);

   return(sprintf(status, "%d", core_count));
}

/* ========================================================================= */
long GetCPULogicalCount(char *status, unsigned long size)
{
   /* A reasonable limitation on length */
   if ( size < 8 )
      return(0);

   return(sprintf(status, "%d", proc_count));
}

/* ========================================================================= */
long GetCPUThreadCount(char *status, unsigned long size)
{
   /* A reasonable limitation on length */
   if ( size < 8 )
      return(0);

   return(sprintf(status, "%d", thr_per_core));
}

/* ========================================================================= */
long GetCPUBogomips(char *status, unsigned long size)
{
   /* A reasonable limitation on length */
   if ( size < strlen(bogomips) )
      return(0);

   return(sprintf(status, "%s", bogomips));
}

/* ========================================================================= */
long GetCPUMHz(char *status, unsigned long size)
{
   /* A reasonable limitation on length */
   if ( size < strlen(speed) )
      return(0);

   return(sprintf(status, "%s", speed));
}

/* ========================================================================= */
#define MAX_SOCKET_COUNT 64 /* Set high assuming virtual is socket = core    */
#define MAX_CORE_COUNT   64 /* Also set high to match sockets                */
#define MAX_PROC_COUNT  128 /* Set abnormally high */

int EnumerateCPUs(void)
{
   char line[96];
   FILE *f;
   int socket[MAX_SOCKET_COUNT]; 
   int core[MAX_CORE_COUNT];
   int proc[MAX_PROC_COUNT];
   int value, i;
   char *strval;
   
   /* Null out the arrays / values */
   i = 0;
   while ( i < MAX_SOCKET_COUNT )
      socket[i++] = 0;

   i = 0;
   while ( i < MAX_CORE_COUNT )
      core[i++] = 0;

   i = 0;
   while ( i < MAX_PROC_COUNT )
      proc[i++] = 0;

   bogomips[0] = 0;
   speed[0] = 0;

   /* Open the file for reading */
   if ( NULL == ( f = fopen("/proc/cpuinfo", "r") ) )
   {
      error_msg("ERROR: Unable to open /proc/cpuinfo.");
      return(1);
   }

   if ( NULL == ( f = fopen("/proc/cpuinfo", "r") ) )
   {
      error_msg("ERROR: Unable to open /proc/cpuinfo.");
      return(1);
   }

   /* Walk the file looking for pieces and parts */
   while ( fgets(line, 96, f) )
   {
      /* Enumerate sockets */
      if ( line == strstr(line, "physical id") )
      {
         value = get_colon_value(line);

         if ( value != -1 )
            socket[value]++;
      }

      /* Enumerate cores */
      if ( line == strstr(line, "core id") )
      {
         value = get_colon_value(line);

         if ( value != -1 )
            core[value]++;
      }

      /* Enumerate logical CPUs */
      if ( line == strstr(line, "processor") )
      {
         value = get_colon_value(line);

         if ( value != -1 )
            proc[value]++;
      }

      /* Get bogomips */
      if ( line == strstr(line, "bogomips") )
      {
         if ( bogomips[0] == 0 )
         {
            if ( NULL != (strval = get_colon_str(line)) )
            {
               chomp(strval);
               strcpy(bogomips, strval);
            }
         }
      }

      /* Get speed */
      if ( line == strstr(line, "cpu MHz") )
      {
         if ( speed[0] == 0 )
         {
            if ( NULL != (strval = get_colon_str(line)) )
            {
               chomp(strval);
               strcpy(speed, strval);
            }
         }
      }
   }

   /* Close the file */
   fclose(f);

   /* Count up the entities we found */
   i = 0;
   value = 0;
   while ( i < MAX_CORE_COUNT )
   {
      if ( core[i++] != 0 )
         value++;
   }
   core_count = value;

   i = 0;
   value = 0;
   while ( i < MAX_SOCKET_COUNT )
   {
      if ( socket[i++] != 0 )
         value++;
   }
   socket_count = value;

   i = 0;
   value = 0;
   while ( i < MAX_PROC_COUNT )
   {
      if ( proc[i++] != 0 )
         value++;
   }
   proc_count = value;

   thr_per_core = (proc_count / core_count) / socket_count;

   return(0);
}

/* ========================================================================= */
char *get_colon_str(char *line)
{
   while (( *line != 0 ) && ( *line != ':' ))
      line++;

   if ( *line != ':' )
      return(NULL);
   else
      line++;

   while (( *line != 0 ) && ( *line == ' ' ))
      line++;

   if ( *line == 0 )
      return(NULL);

   return(line);
}

/* ========================================================================= */
int get_colon_value(char *line)
{
   int value;

   while (( *line != 0 ) && ( *line != ':' ))
      line++;

   if ( *line != ':' )
      return(-1);
   else
      line++;

   while (( *line != 0 ) && ( *line == ' ' ))
      line++;

   value = 0;
   while (( *line >= '0' ) && ( *line <= '9' ))
   {
      value *= 10;
      value += *line - '0';

      line++;
   }
   
   return(value);
}




