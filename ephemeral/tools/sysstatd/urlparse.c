#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "support.h"
#include "urlparse.h"

/* ========================================================================= */
/* NOTES:
    1. The parsing method here is simply "sufficient" (and possibly not even
       that). It looks for patterns in the URL that it knows... rather than
       trying to parse every item. The appropriate solution would be to 
       pull off item by item and match against expected strings. Non-matches
       would become errors. ---> UP_ERROR_UNK (unknown item encountered)
    2. The specific error encountered is returned as the function return value
       and is or-ed onto the q->parse_failure variable.
    3. Perhaps q->parse_failure should be q->parse_error;
*/
int parse_query(struct query *q, char *qstring)
{
   assert(NULL != q);

   if ( NULL == qstring )
   {
      q->tree = TREE_DEFAULT;
      q->format = FORMAT_DEFAULT;
      return(UP_ERROR_NONE);
   }

   if ( qstring[0] == 0 )
   {
      q->tree = TREE_DEFAULT;
      q->format = FORMAT_DEFAULT;
      return(UP_ERROR_NONE);
   }

   /* Set the "unset" values */
   q->tree = TREE_NONE;
   q->format = FORMAT_NONE;

   /*** Parse the potential formats ***/
   if ( StrStr(qstring, "/json") )
   {
      if ( q->format != FORMAT_NONE )
      {
         q->parse_failure |= UP_ERROR_FMT;
         return(UP_ERROR_FMT);
      }

      q->format = FORMAT_JSON;
   }

   if ( StrStr(qstring, "/xml") )
   {
      if ( q->format != FORMAT_NONE )
      {
         q->parse_failure |= UP_ERROR_FMT;
         return(UP_ERROR_FMT);
      }

      q->format = FORMAT_XML;
   }

   if ( StrStr(qstring, "/csv") )
   {
      if ( q->format != FORMAT_NONE )
      {
         q->parse_failure |= UP_ERROR_FMT;
         return(UP_ERROR_FMT);
      }

      q->format = FORMAT_CSV;
   }

   if ( StrStr(qstring, "/yaml") )
   {
      if ( q->format != FORMAT_NONE )
      {
         q->parse_failure |= UP_ERROR_FMT;
         return(UP_ERROR_FMT);
      }

      q->format = FORMAT_YAML;
   }

   /*** Parse the (known) trees ***/
   if ( StrStr(qstring, "/root") )
   {
      if ( q->tree != TREE_NONE )
      {
         q->parse_failure |= UP_ERROR_TREE;
         return(UP_ERROR_TREE);
      }

      q->tree = TREE_ROOT;
   }

   if ( StrStr(qstring, "/default") ) /* Alt name for /root */
   {
      if ( q->tree != TREE_NONE )
      {
         q->parse_failure |= UP_ERROR_TREE;
         return(UP_ERROR_TREE);
      }

      q->tree = TREE_ROOT;
   }

   if ( StrStr(qstring, "/core") ) /* Alt name for /root */
   {
      if ( q->tree != TREE_NONE )
      {
         q->parse_failure |= UP_ERROR_TREE;
         return(UP_ERROR_TREE);
      }

      q->tree = TREE_ROOT;
   }

   if ( StrStr(qstring, "/perf") )
   {
      if ( q->tree != TREE_NONE )
      {
         q->parse_failure |= UP_ERROR_TREE;
         return(UP_ERROR_TREE);
      }

      q->tree = TREE_PERF;
   }

   if ( StrStr(qstring, "/qc_conr") )
   {
      if ( q->tree != TREE_NONE )
      {
         q->parse_failure |= UP_ERROR_TREE;
         return(UP_ERROR_TREE);
      }

      q->tree = TREE_QCCR;
   }

   if ( StrStr(qstring, "/hardware") )
   {
      if ( q->tree != TREE_NONE )
      {
         q->parse_failure |= UP_ERROR_TREE;
         return(UP_ERROR_TREE);
      }

      q->tree = TREE_HRDW;
   }

   if ( StrStr(qstring, "/bbenv") )
   {
      if ( q->tree != TREE_NONE )
      {
         q->parse_failure |= UP_ERROR_TREE;
         return(UP_ERROR_TREE);
      }

      q->tree = TREE_BBNV;
   }

   if ( StrStr(qstring, "/process") )
   {
      if ( q->tree != TREE_NONE )
      {
         q->parse_failure |= UP_ERROR_TREE;
         return(UP_ERROR_TREE);
      }

      q->tree = TREE_PROC;
   }

   /* Set defaults if nothing was parsed (for those values) */
   if ( q->format == FORMAT_NONE )
      q->format = FORMAT_DEFAULT;

   if ( q->tree == TREE_NONE )
      q->tree = TREE_DEFAULT;

   return(UP_ERROR_NONE);
}

/* ========================================================================= */
int parse_labels(struct query *q, char *qstring)
{
   int i;

   /* Erase any reference to a previous query */
   q->label_list[0] = 0;

   if ( NULL == qstring )
      return(0);

   i = 0;
   while ( qstring[i] != 0 )
   {
      /* Bounds checking - return indication that things did not go well */
      if ( i == LABEL_LIST_SIZE )
      {
         q->label_list[i] = 0;
         return(1);
      }

      q->label_list[i] = qstring[i];
      i++;
   }

   q->label_list[i] = 0;

   return(0);
}

/* ========================================================================= */
int GetLabelValue(char *value, unsigned int vlen, struct query *q, char *name)
{
   char *positive;
   int bad_match;
   int i;

   assert(value != NULL);
   assert(q != NULL);

   if ( NULL == name )
      return(0);

   if ( q->label_list[0] == 0 )
      return(0);

   /* Look for a simple (case sensitive) pattern */
   if ( NULL == (positive = strstr(q->label_list, name)) )
      return(0);

   bad_match = 1;
   if ( positive == q->label_list )
   {
      bad_match = 0;
   }
   else
   {
      if ( *(positive - 1) == '&' )
         bad_match = 0;
   }

   if ( bad_match )
   {
      /* We matched a value, not a label */
      return(0);
   }

   while ( *positive != '=' )
   {
      /* Check for prematurely terminated strings */
      if ( *positive == 0 )
         return(0);

      positive++;
   }

   if ( *positive == '=' )
      positive++;
   else
      return(0);

   i = 0;
   while (( positive[i] != '&' ) && ( positive[i] != 0 ))
   {
      /* Not enough room */
      if ( i == vlen )
         return(0);

      value[i] = positive[i];
      i++;
   }
   value[i] = 0; /* Terminate */

   /* We did it! */
   return(1);
}

/* ========================================================================= */
struct query *ParseURLString(struct query *q, char *urlstring)
{
   char *qstring = NULL;
   char *paramlist = NULL;

   /* Let's start with sanity checks. */
   if ( NULL == q )
   {
      if ( NULL == (q = (struct query *)malloc(sizeof(struct query))) )
      {
         error_msg("ERROR: Unable to malloc for query processing.");
         return(NULL);
      }
   }

   /* Addume no parse errors to start */
   q->parse_failure = UP_ERROR_NONE;
   /* Set other values to NONE - they will be overwritten */
   q->tree = TREE_NONE;
   q->format = FORMAT_NONE;

   /* Handle the NULL URI case */
   /* This means that we only want to insure that the q is allocated and
      do not care what the actual "query" is. (Likely the user asked for
      OPTIONS) */
   if ( NULL == urlstring )
      return(q);

   /* Handle the empty URL case */
   if ( (urlstring[0] == 0) ||
        ((urlstring[0] == '/') && (urlstring[1] == 0)) )
   {
      q->parse_failure = UP_ERROR_NONE;
      q->tree = TREE_DEFAULT;
      q->format = FORMAT_DEFAULT;
      q->label_list[0] = 0;
      return(q);
   }

   /* If not empty, then it must be 4 chars - at a minimum */
   /* /root, /xml, /json, /cvs, /perf/json.... */
   if ( strlen(urlstring) < 4 )
   {
      q->parse_failure |= UP_ERROR_SHORT;
      return(q);
   }

   /* Is this an absoluteURI or not? */
   if ( *urlstring != '/' )
   {
      /* We are not dealing with the "traditional" URL string. This is
         likely a absoluteURI (see RFC2616) string. */
      
      /* Bite off expected 'chunks' of the URI */
      if (( urlstring[0] == 'h' ) &&
          ( urlstring[1] == 't' ) &&
          ( urlstring[2] == 't' ) &&
          ( urlstring[3] == 'p' ))
      {
         urlstring += 4;
      }

      /* [http]://[server_addr...] */
      if ((urlstring[0] == ':' ) &&
          (urlstring[1] == '/' ) &&
          (urlstring[2] == '/' ))
      {
         urlstring += 3;
      }

      /* Walk up to the "traditional" URI string */
      while (( *urlstring != 0 ) && ( *urlstring != '/' ))
         urlstring++;

      if ( *urlstring != '/' )
      {
         q->parse_failure |= UP_ERROR_MAL;
         return(q);
      }
   }

   while ( *urlstring != 0 )
   {
      if ( *urlstring == '/' )
      {
         if ( NULL == qstring )
            qstring = urlstring;
      }

      if ( *urlstring == '?' )
      {
         *urlstring = 0;
         urlstring++;

         /* String should not end with a '?' */
         if ( *urlstring == 0 )
         {
            q->parse_failure |= UP_ERROR_MAL;
            return(q);
         }

         if ( NULL == paramlist )
            paramlist = urlstring;
      }

      urlstring++;
   }

   if(parse_query(q, qstring))
      return(q);

   if(parse_labels(q, paramlist))
      return(q);

   return(q);
}


