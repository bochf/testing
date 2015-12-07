#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "slurp.h"
#include "filefmt.h"
#include "options.h"

#define MAX_LINE_BUF 96
#define MAX_LINE_LEN 80

/*
  About Input Validation
  
  I wrote this in a weekend... and there are about one thousand directions
  I can go with input validation. But because of the time constraint, my
  thought is: garbage in, garbage out - for now.

  What we check:
    - remove_illegal_chars(): Can it print, convert ',' to ' '.

  What is NOT checked (but we could/should):
    - OS types should match a known list
    - Similar for keys
    - Punctuation and delimiters ':"$%()[]....
    - Shit you should never see *&^%$#
    - Any non-valid path/name character in Location
    - Lots of other things
*/

/* ========================================================================= */
char *strip_whitespace(char *wsstring)
{
   int i;

   if ( NULL == wsstring )
      return(NULL);

   if ( wsstring[0] == 0 )
      return(wsstring); /* To return NULL or just the string??? */

   /*** Handle trailing whitespace ***/
   i = strlen(wsstring) - 1;

   while (( wsstring[i] == ' ' ) || ( wsstring[i] == '\n' ))
   {
      wsstring[i--] = 0;

      if ( i == -1 )
         return(wsstring); /* The string is now zeroed out. so just return */
   }

   /*** Handle leading whitespace */
   i = 0;
   while((wsstring[i] == ' ' ) || (wsstring[i] == 9 )) /* 9 == tab */
   {
      i++;
   }

   return(wsstring + i);
}


/* ========================================================================= */
char *make_short_filename(char *filename)
{
   int lasts;
   int i;

   /* Look for error conditions... but dont report them, they will get reported
      elsewhere */
   if ( NULL == filename )
      return(NULL);

   lasts = -1;
   i = 0;

   while ( filename[i] != 0 )
   {
      if ( filename[i] == '/' )
         lasts = i;

      i++;
   }

   return(filename + lasts + 1);
}



/* ========================================================================= */
/* If we have a pattern match for the expected line, copy the info to location */
int read_single_line(char *into, char *from, char *pattern)
{
   int plen;
   int clen;

   assert(into != NULL);
   assert(from != NULL);
   assert(pattern != NULL);

   if ( from == strstr(from, pattern) )
   {
      plen = strlen(pattern);
      from = strip_whitespace(from + plen);

      strcpy(into, from);

      if (0 < (clen = strlen(into)))
      {
         clen--;
         if ( into[clen] == '\n' )
            into[clen] = 0;
      }

      return(1);
   }

   return(0);
}

/* ========================================================================= */
/* Just like read_single_line() but converts to UC for later compares */
int ucread_single_line(char *into, char *from, char *pattern)
{
   int plen;
   int clen;
   int i;

   assert(into != NULL);
   assert(from != NULL);
   assert(pattern != NULL);

   if ( from == strstr(from, pattern) )
   {
      plen = strlen(pattern);
      from = strip_whitespace(from + plen);

      strcpy(into, from);

      if (0 < (clen = strlen(into)))
      {
         clen--;
         if ( into[clen] == '\n' )
            into[clen] = 0;
      }

      i = 0;
      while ( into[i] != 0 )
      {
         if ( ( into[i] >= 'a' ) && ( into[i] <= 'z' ))
            into[i] -= 32;

         i++;
      }

      return(1);
   }

   return(0);
}

/* ========================================================================= */
/* See notes on input validataion at the top of this file */
#define RIC_FIXED  1  /* Found something bad, fixed (converted) it [','->' '] */
#define RIC_BROKEN 2  /* Found something bad, gave up */
#define RIC_OK     0  /* No issues */
int remove_illegal_chars(char *stringin)
{
   int i;
   int rv;

   rv = RIC_OK;

   i = 0;
   while(stringin[i] != 0)
   {
      /* Can we print it? */
      if ( ! isprint(stringin[i]) )
         return(RIC_BROKEN);

      /* Commas to spaces */
      if (stringin[i] == ',')
      {
         stringin[i] = ' ';
         rv = RIC_FIXED;
      }
      
      i++;
   }

   return(rv);
}

/* ========================================================================= */
int ReadCmdItemFile(struct cmditem *ci, char *filename, int verbose)
{
   FILE *f;
   char line[MAX_LINE_BUF];
   char status[32];
   char impact[96];
   char *short_filename;
   int lastitemsread;

   assert(NULL != filename);
   assert(0 != filename[0]);

   short_filename = make_short_filename(filename);

   if ( verbose )
      printf("Reading %s...", short_filename);

   ci->name[0] = 0;
   ci->desc[0] = 0;
   ci->keys[0] = 0;
   ci->locn[0] = 0;
   ci->pfrm[0] = 0;
   ci->ownr[0] = 0;
   ci->file[0] = 0;
   impact[0] = 0;
   ci->itemsread = 0;
   ci->fileok = 1;
   lastitemsread = 0;

   /* Copy in the filename */
   strcpy(ci->file, short_filename);

   if ( NULL == ( f = fopen(filename, "r") ) )
      return(1);

   while(fgets(line, MAX_LINE_BUF, f))
   {
      line[MAX_LINE_LEN] = 0;
      if ( strlen(line) >= MAX_LINE_LEN )
      {
         if ( verbose )
         {
            printf("Failed.\n");
            printf("WARNING: A line in %s exceeds %d chars. Ignoring file.\n", short_filename, MAX_LINE_LEN);
            fflush(stdout);
         }

         ci->fileok = 0;
         return(1);
      }
      else
      {
         ci->itemsread += read_single_line(ci->name, line, NAME_LINE_KEY);
         ci->itemsread += read_single_line(ci->desc, line, DESC_LINE_KEY);
         ci->itemsread += ucread_single_line(ci->keys, line, KEYS_LINE_KEY);
         ci->itemsread += read_single_line(ci->locn, line, LOCN_LINE_KEY);
         ci->itemsread += ucread_single_line(ci->pfrm, line, PFRM_LINE_KEY);
         ci->itemsread += read_single_line(ci->ownr, line, OWNR_LINE_KEY);
         ci->itemsread += ucread_single_line(impact, line, IMPT_LINE_KEY);

         /* It is possible that there are entries in the file that are
            not matched here. As a result, we may parse multiple lines
            without a match. This is ok. The file could have comments
            in it. */

         if ( (ci->fileok) && (verbose) )
         {
            if ( ci->itemsread != lastitemsread )
            {
               printf(".");
               fflush(stdout);
            }

            lastitemsread = ci->itemsread;
         }

      }

   }
   fclose(f);

   /*** Check what we got ***/
   strcpy(status, "Fixed"); /* Assume good, change to otherwise */
   
   if ( (ci->fileok) && (verbose) )
   {
      printf(".");
      fflush(stdout);
   }


   if ( IMPACT_FAIL == ( ci->impact = ParseImpact(impact) ) )
   {
      if (verbose)
      {
         printf("Failed.\nWARNING: Unable to parse the \"Impact\" field in %s.\n", short_filename);
         fflush(stdout);
      }
      ci->fileok = 0;
      return(1);
   }

   /* Check that we got all the exected keys (lines) */
   if ( ci->itemsread == REQUIRED_KEYS )
   {
      if ( (ci->fileok) && (verbose) )
      {
         printf(".");
         fflush(stdout);
      }
   }
   else
   {
      if ( verbose )
      {
         printf("Short.\nWARNING: Expected %d entries in %s, got %d.\n", REQUIRED_KEYS, short_filename, ci->itemsread);
         fflush(stdout);
      }
      ci->fileok = 0;
      return(1);
   }

   /* This will only report on the last error! */
   switch( remove_illegal_chars(ci->keys) )
   {
   case RIC_FIXED:
      strcpy(status, "Fixed");
      break;
   case RIC_BROKEN:
      ci->fileok = 0;
      break;
   case RIC_OK:
   default:
      break;
   }


   if ( ci->fileok )
   {
      if ( verbose )
      {
         printf(".");
         fflush(stdout);
      }
   }
   else
   {
      if ( verbose )
      {
         printf("Failed.\nWARNING: Keylist entry in %s has illegal characters. Skipping file.\n", short_filename);
         fflush(stdout);
      }
      return(1);
   }


   switch( remove_illegal_chars(ci->pfrm) )
   {
   case RIC_FIXED:
      strcpy(status, "Fixed");
      break;
   case RIC_BROKEN:
      ci->fileok = 0;
      break;
   case RIC_OK:
   default:
      break;
   }

   if ( ci->fileok )
   {
      if ( verbose )
      {
         printf(".");
         fflush(stdout);
      }
   }
   else
   {
      if ( verbose )
      {
         printf("Failed.\nWARNING: Platform entry in %s has illegal characters. Skipping file.\n", short_filename);
         fflush(stdout);
      }
      return(1);
   }

   if ( verbose )
   {
      printf("%s.\n", status);
      fflush(stdout);
   }

   /* The file is probably good */
   return(0);
}
