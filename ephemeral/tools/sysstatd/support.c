#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <syslog.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>

#include "support.h"

extern int errorto; /* From main.c - used for printing error messages */

/* ========================================================================= */
char *mkstring(char *cpin)
{
   char *cpout;

   if (NULL == cpin)
      return(NULL);

   if ( 0 == cpin[0] )
      return(NULL);

   if ( NULL == ( cpout = (char *)malloc(strlen(cpin) + 1) ) )
   {
      error_msg("ERROR: Failed to allocate memory for a simple string.");
      return(NULL);
   }

   strcpy(cpout, cpin);

   return(cpout);
}

/* ========================================================================= */
int dotdotdot_string(char *cpset, char *cpsrc, int size)
{
   int i, j;

   assert( NULL != cpset );
   assert( NULL != cpsrc );
   assert( size > 5 );

   if ( (size - 1) < (j = strlen(cpsrc)) )
   {
      i = size - 1;
      j -= 1;
      cpset[i--] = 0;
            
      while ( i > 2 )
         cpset[i--] = cpsrc[j--];
            
      cpset[i--] = '.';
      cpset[i--] = '.';
      cpset[i] = '.';
   }
   else
      strcpy(cpset, cpsrc);

   return(0);
}

/* ========================================================================= */
int string_dotdotdot(char *cpset, char *cpsrc, int size)
{
   int i;

   assert( NULL != cpset );
   assert( NULL != cpsrc );
   assert( size > 5 );

   i = 0;
   while ( cpsrc[i] != 0 )
   {
      cpset[i] = cpsrc[i];

      i++;

      if ( i >= (size - 1) )
      {
         cpset[size - 1] = 0;
         cpset[size - 2] = '.';
         cpset[size - 3] = '.';
         cpset[size - 4] = '.';

         return(0);
      }
   }

   cpset[i] = 0;

   return(0);
}

/* ========================================================================= */
int wstrim(char *line)
{
   if ( NULL == line )
      return(1);

   if ( line[0] == 0 )
      return(0);

   while (( line[strlen(line) - 1] == ' ' ) || ( line[strlen(line) - 1] == '\t' ))
      line[strlen(line) - 1] = 0;
   
   return(0);
}

/* ========================================================================= */
int chomp(char *line)
{
   if ( NULL == line )
      return(1);

   if ( line[strlen(line) - 1] == '\n' )
      line[strlen(line) - 1] = 0;
   
   return(0);
}

char *Chomp(char *line)
{
   if ( NULL == line )
      return(NULL);

   if ( line[strlen(line) - 1] == '\n' )
      line[strlen(line) - 1] = 0;
   
   return(line);
}

/* ========================================================================= */
/* strip off comments */
int clamp(char *line)
{
   int i = 0;
   int inq = 0;

   if ( NULL == line )
      return(1);

   while(line[i] != 0)
   {
      if (( line[i] == '"' ) || ( line[i] == '\'' ))
         inq = 1 - inq;

      if (( line[i] == '#' ) && ( 0 == inq ))
      {
         line[i] = 0;
         return(0);
      }

      i++;
   }

   return(0);
}

/* ========================================================================= */
/* Scrape off the lead(ing) white space */
char *unlead(char *line)
{
   char *unleaded;

   if ( NULL == line )
      return(NULL);

   if ( *line == 0 )
      return(line);

   unleaded = line;

   while ((*unleaded == ' ') || (*unleaded == '\t'))
      unleaded++;

   return(unleaded);
}

/* ========================================================================= */
#define ERROR_STRING_MAX 100

int error_msg(const char *format, ...)
{
   va_list ap;

   char outstring[ERROR_STRING_MAX + 1];

   /* Clean up the var-args business - This way clean strings can be passed
      to whatever handler(s) we end up using. */
   va_start(ap, format);
   vsnprintf(outstring, ERROR_STRING_MAX, format, ap);
   va_end(ap);

   if ( errorto & ERROR_MSG_STDERR )
   {
      fprintf(stderr, "%s\n", outstring);
   }

   if ( errorto & ERROR_MSG_SYSLOG )
   {
      syslog(LOG_ERR|LOG_USER, "%s\n", outstring);
   }

   /* STUB: act.log writer call goes here */

   /* STUB: syslog writer call goes here */

   /* STUB: direct error long file goes here */

   return(0);
}

/* ========================================================================= */
int is_numeric(char *cpin)
{
   int i;

   if (NULL == cpin)
      return(0);

   if (cpin[0] == 0)
      return(0);

   /* Allow to be negative */
   if ( cpin[0] == '-' )
      i = 1;
   else
      i = 0;

   while(cpin[i] != 0)
   {
      if((cpin[i] < '0') || (cpin[i] > '9'))
         return(0);

      i++;
   }

   return(1);
}

/* ========================================================================= */
int is_pnumeric(char *cpin)
{
   int i;

   if (NULL == cpin)
      return(0);

   if (cpin[0] == 0)
      return(0);

   i = 0;

   while(cpin[i] != 0)
   {
      if((cpin[i] < '0') || (cpin[i] > '9'))
         return(0);

      i++;
   }

   return(1);
}

/* ========================================================================= */
int file_exists(char *filename)
{
   struct stat sb;

   /* Bad things that must stop us from continuing */
   assert( NULL != filename );
   assert( filename[0] != 0 );

   /* See header file for notes */
   if ( 0 == stat(filename, &sb) )
   {
      return(1);
   }
   else
   {
      if ( errno == ENOENT )
         return(0);
   }

   return(1);
}

/* ========================================================================= */
char *StrStr(char *haystack, char *needle)
{
   char *n;
   char *h;

   /* From what I read:
        strstr("", "")  ----> returns string one (haystack)
        strstr("abc", "") --> returns string one (haystack)

        This does neither - there is no match!
        This is a non-compliant implementation.
   */
        
   if ( NULL == haystack )
      return(NULL);

   if ( NULL == needle )
      return(NULL);

   while ( *haystack != 0 )
   {
      h = haystack;
      n = needle;
      while ( tolower(*h) == tolower(*n) )
      {
         if ( *(n + 1) == 0 )
            return(haystack);

         n++;
         h++;
      }

      haystack++;
   }

   return(NULL);
}

#ifdef DEPRECATED_CODE
/* =========================================================================
 * Name: StrStr (The "first attempt" solution)
 * Description: A non-case-sensitive version of strstr() 
 * Paramaters: haystack and needle (like strstr)
 * Returns: Pointer to the *original* data!
 * Side Effects: None - SEE NOTES!
 * Notes: This is a very inefficient variant... It is not a clean/
 *        new implementation. Instead, it allocates a string, converts
 *        it to all upper-case then sends to strstr(). It DOES free the
 *        memory when it is done (no side-effects). The intent here is to 
 *        use this very infrequently (such as when parsing a few items from
 *        an input file).
 * 
 *        Also note: It does pointer math and gives you a pointer to the
 *        original/unmodified data.
 *
 *        If you have to do lots of case insensitive compares... DO NOT USE
 *        THIS. Save a case-specific version of your pattern and then convert
 *        the changing/dynamic to that case for the compare.
 */
char *StrStr(char *haystack, char *needle)
{
   char *HAYSTACK;
   char *NEEDLE;
   char *ptr;
   int i;

   /* Let's malloc here. It is expensive, but relatively safe. This is not
      called with high frequency so performance and fears of leaks are not
      a big deal. I just exit on malloc() failures. If you can't malloc
      then you should be dead soon anyways. */
   if (NULL == (HAYSTACK = (char *)malloc(strlen(haystack) + 1)))
      return(NULL);

   if (NULL == (NEEDLE = (char *)malloc(strlen(needle) + 1)))
      return(NULL);

   i = 0;
   while(haystack[i] != 0)
   {
      if ((haystack[i] >= 'a') && (haystack[i] <= 'z'))
         HAYSTACK[i] = haystack[i] - 32;
      else
         HAYSTACK[i] = haystack[i];

      i++;
   }
   HAYSTACK[i] = 0;

   i = 0;
   while(needle[i] != 0)
   {
      if ((needle[i] >= 'a') && (needle[i] <= 'z'))
         NEEDLE[i] = needle[i] - 32;
      else
         NEEDLE[i] = needle[i];

      i++;
   }
   NEEDLE[i] = 0;

   if ( NULL != (ptr = strstr(HAYSTACK, NEEDLE)) )
   {
      /* If it is a match, then the pointer needs to be "transfered"
         back to the original reference */
      ptr = haystack + ((unsigned long)ptr - (unsigned long)HAYSTACK);
   }

   free(NEEDLE);
   free(HAYSTACK);

   return(ptr);
}
#endif


