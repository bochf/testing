#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "bbenv.h"



/* ========================================================================= */
char *get_hostname(void)
{
   char *hn;

   hn = malloc(32);

   /* gethostname() does all the work here... It will tell me if the pointer
      is bad, if the string is too short, any error. I just return NULL, and
      we can deal with it elsewhere - without freeing the memory. (This does
      not constitute a real leak) */
   if ( gethostname(hn, 32) )
      return(NULL);

   return(hn);
}

/* ========================================================================= */
int line_has_dev(char *line)
{
   int linelen;
   int i;

   linelen = strlen(line);

   i = 2; /* Start into the string */
   while ( i + 4 < linelen )
   {
      if (line[i] == ' ')
      {
         if (line[i + 1] == 'd')
         {
            if (line[i + 2] == 'e')
            {
               if (line[i + 3] == 'v')
               {
                  if (line[i + 4] == ' ')
                  {
                     return(1);
                  }
               }
            }
         }
      }

      i++;
   }

   return(0);
}

/* ========================================================================= */
#define BBCPU_MAX_LINE_SZ 1024

int IsBBDevSystem(void)
{
   char *hn;
   FILE *bbcpu;
   char line[BBCPU_MAX_LINE_SZ + 1];

   /* Get hostname, if not, then not dev */
   if(NULL == (hn = get_hostname()))
      return(0);

   /* Check access, if not, then not dev */
   if(access("/bb/bin/bbcpu.lst", R_OK))
      return(0);

   /* Open the file, if not, then not dev */
   if (NULL == (bbcpu = fopen("/bb/bin/bbcpu.lst", "r")))
      return(0);
   
   while(fgets(line, BBCPU_MAX_LINE_SZ, bbcpu))
   {
      /* Weak pattern match, backed by secondary condition on first match */
      if((line == strstr(line, hn)) && (' ' == line[strlen(hn)]))
      {
         if ( line_has_dev(line) )
         {
            fclose(bbcpu);
            return(1);
         }
      }
   }
   
   fclose(bbcpu);

   return(0);
}
