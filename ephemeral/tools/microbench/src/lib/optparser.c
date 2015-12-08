
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "optparser.h"
#include "verbosity.h"

/* ========================================================================= */
struct optbase *optbase_init(void)
{
   struct optbase *ob = NULL;

   if ( NULL == ( ob = (struct optbase *)malloc(sizeof(struct optbase)) ) )
   {
      ErrorMessage("ERROR: Failed to allocate memory.\n");
      return(NULL);
   }

   ob->opstart = NULL;
   ob->action = PARSE_NONE;
   ob->parse_error = 0;
   ob->parsed_opts = 0;

   return(ob);
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

/* ========================================================================= */
char *mkstring(char *cpin)
{
   char *cpout;

   if (NULL == cpin)
      return(NULL);

   if ( NULL == ( cpout = (char *)malloc(strlen(cpin) + 1) ) )
   {
      ErrorMessage("ERROR: Failed to allocate memory for a simple string.");
      return(NULL);
   }

   strcpy(cpout, cpin);

   return(cpout);
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
char *MKSTRING(char *cpin)
{
   char *cpout;
   int i;

   if (NULL == cpin)
      return(NULL);

   if ( NULL == ( cpout = (char *)malloc(strlen(cpin) + 1) ) )
   {
      ErrorMessage("ERROR: Failed to allocate memory for a simple string.");
      return(NULL);
   }

   i = 0;
   while( cpin[i] != 0 )
   {
      if ( ( cpin[i] >= 'a' ) && ( cpin[i] <= 'z' ) )
         cpout[i] = cpin[i] - 32;
      else
         cpout[i] = cpin[i];

      i++;
   }

   cpout[i] = 0;

   return(cpout);
}

/* ========================================================================= */
struct optpair *optpair_init(char *label, char *value)
{
   struct optpair *op;

   if ( NULL == ( op = (struct optpair *)malloc(sizeof(struct optpair)) ) )
   {
      ErrorMessage("ERROR: Unable to allocate memory.\n");
      return(NULL);
   }

   op->name = MKSTRING(label);
   op->cpvalue = mkstring(value);

   return(op);
}

/* ========================================================================= */
int copy_uc_string(char *tostr, char *fromstr)
{
   while ( *fromstr != 0 )
   {
      if ( ( *fromstr >= 'a' ) && ( *fromstr <= 'z' ) )
         *tostr = *fromstr - 32;
      else
         *tostr = *fromstr;

      fromstr++;
      tostr++;
   }

   *tostr = 0;

   return(0);
}

/* ========================================================================= */
char *get_str_value(struct optbase *ob, char *pair_name)
{
   struct optpair *op;
   char LABEL[MAX_LABEL_SIZE];

   assert(NULL != pair_name);
   assert(0 != pair_name[0]);

   copy_uc_string(LABEL, pair_name);

   /* Does it exist (at all) */
   op = ob->opstart;
   while ( op )
   {
      if ( 0 == strcmp(op->name, LABEL) )
      {
         return(op->cpvalue);
      }

      op = op->next;
   }

   return(NULL);
}

/* ========================================================================= */
struct optpair *parse_set_line(char *line)
{
   int i, j;
   char label[MAX_LABEL_SIZE];
   char value[MAX_VALUE_SIZE];

   /* Start with no assumptions about content */
   if ( ( line[0] == 's' ) && ( line[1] == 'e' ) && ( line[2] == 't' ) )
      i = 3;
   else
      return(NULL);

   /* Skip through white space */
   while( ( line[i] == ' ' ) || ( line[i] == '\t' ) )
      i++;

   j = 0;
   while ( ( line[i] != ' ' ) && ( line[i] != '\t' ) && ( line[i] != 0 ) && ( line[i] != '=' ) )
   {
      label[j++] = line[i++];

      if (( i >= MAX_LABEL_SIZE ) || ( j >= MAX_LABEL_SIZE ))
         return(NULL);
   }

   label[j] = 0;

   /* Skip through white space */
   while( ( line[i] == ' ' ) || ( line[i] == '\t' ) )
      i++;
   
   /* Skip through equality symbol */
   if( line[i] == '=' )
      i++;
   else
      return(NULL);

   /* Skip through white space */
   while( ( line[i] == ' ' ) || ( line[i] == '\t' ) )
      i++;

   if ( line[i] == '"' )
   {
      i++;

      j = 0;
      while ( line[i] != '"' )
      {

         if ( line[i] == 0 )
         {
            return(NULL);
         }

         value[j++] = line[i++];
         
         if ( j >= MAX_VALUE_SIZE )
            return(NULL);
      }
      
      value[j] = 0;
   }
   else
   {
      j = 0;
      while ( ( line[i] != ' ' ) && ( line[i] != '\t' ) && ( line[i] != 0 ) && ( line[i] != '\n' ) )
      {
         value[j++] = line[i++];

         if ( j >= MAX_VALUE_SIZE )
            return(NULL);
      }

      value[j] = 0;
   }


   return(optpair_init(label, value));
}

/* ========================================================================= */
struct optbase *file_parse(FILE *fh)
{
   char line[128];
   struct optbase *ob = NULL;
   struct optpair *op;
   int lineno;

   /* Set up the optbase struct */
   if ( NULL == ( ob = optbase_init() ) )
      return(NULL);

   /* Start parsing the file - line by line */
   lineno = 0;
   while ( fgets(line, 128, fh) )
   {
      lineno++;
      
      chomp(line);  /* Remove line feed */
      wstrim(line); /* Remove trailing spaces */

      /* We do not strip off leading spaces because:
          1. The keywords we are looking for here must be left-justified (no
             leading spaces).
          2. We parse out spaces between the = sign and the beginning of the
             RHS (value).
          3. The value may have leading spaces (if quoted) and therefore does
             not have leading spaces stripped (at a later time).
      */

      if ( ( line[0] == 's' ) && ( line[1] == 'e' ) && ( line[2] == 't' ) )
      {
         if ( NULL == ( op = parse_set_line(line) ) )
         {
            ErrorMessage("ERROR: Failed to parse set line number %d.\n", lineno);
            /* Do not return error. Let them stack up */
            ob->parse_error++;
         }
         else
         {
            /* Insert into the linked list. */
            op->next = ob->opstart;
            ob->opstart = op;
         }
      }

      /* Terminating conditions */
      if ( ( line[0] == 'g' ) || ( line[0] == 'G' ) )
      {
         if ( ( line[1] == 'o' ) || ( line[1] == 'O' ) )
         {
            /* Check for a short list of potentially valid characters following "go" */
            if ( ( line[2] == '\t' ) || ( line[2] == 0 ) || ( line[2] == ' ' ) || ( line[2] == '#' ) )
            {
               ob->action = PARSE_GO;
               return(ob);
            }
         }
      }


      if ( ( line[0] == 'e' ) || ( line[0] == 'E' ) )
      {
         if ( ( line[1] == 'v' ) || ( line[1] == 'V' ) )
         {
            if ( ( line[2] == 'a' ) || ( line[2] == 'A' ) )
            {
               if ( ( line[3] == 'l' ) || ( line[3] == 'L' ) )
               {
                  /* Check for a short list of potentially valid characters following "eval" */
                  if ( ( line[4] == '\t' ) || ( line[4] == 0 ) || ( line[4] == ' ' ) || ( line[4] == '#' ) )
                  {
                     ob->action = PARSE_EVAL;
                     return(ob);
                  }
               }
            }
         }
      }
   }

   return(ob);
}

/* ========================================================================= */
struct optbase *filename_parse(char *filename)
{
   FILE *fh;
   struct optbase *ob = NULL;

   /* STUB: Should stat() the file here to insure that it is a file */

   if ( NULL == ( fh = fopen(filename, "r") ) )
   {
      ErrorMessage("ERROR: Unable to open \"%s\" for input.\n", filename);
      return(NULL);
   }

   /* Don't bother checking for errors - it will not matter */
   ob = file_parse(fh);
   
   fclose(fh);

   return(ob);
}


/* ========================================================================= */
struct optbase *stdin_parse(void)
{
   FILE *fh;
   struct optbase *ob = NULL;

   fh = stdin;

   /* Don't bother checking for errors - it will not matter */
   ob = file_parse(fh);
   
   fclose(fh);

   return(ob);
}

/* ========================================================================= */
/* The output of this function is inserted directly into SetVerbosityLevel()
   so the value must be in a specific range for the option to be set. -1
   falls outside of the range and will be ignored. So we make -1 the value
   for all error conditions or cases where verbosity is not determined. */
int get_verbosity_level(struct optbase *ob)
{
   char *value;
   char VALUE[32];
   int i;

   if ( NULL == ob )
      return(-1);

   if ( NULL == ( value = get_str_value(ob, "VERBOSITY") ) )
      return(-1);

   i = 0;
   while( value[i] != 0 )
   {
      if ( ( value[i] >= 'a' ) && ( value[i] <= 'z' ) )
         VALUE[i] = value[i] - 32;
      else
         VALUE[i] = value[i];

      i++;

      /* Drop out if we are pulling too much information */
      if ( i > 30 )
         return(-1);
   }
   VALUE[i] = 0;
   
   if ( 0 == strcmp(VALUE, "SILENT") )
      return(VL_SILENCE);

   if ( ( 0 == strcmp(VALUE, "NORMAL") ) || ( 0 == strcmp(VALUE, "ERRORS") ) || ( 0 == strcmp(VALUE, "FALSE") ) || ( 0 == strcmp(VALUE, "0") ) )
      return(VL_NORMALV);

   if ( ( 0 == strcmp(VALUE, "VERBOSE") ) || ( 0 == strcmp(VALUE, "TRUE") ) || ( 0 == strcmp(VALUE, "1") ) )
      return(VL_VERBOSE);
          
   if ( 0 == strcmp(VALUE, "DEBUG") )
      return(VL_DBG_MSG);
          
   /* No match - bail with "invalid" value */
   return(-1);
}

/* ========================================================================= */
struct optbase *ReadOptions(int argc, char *argv[])
{
   struct optbase *ob;

   /* Set the verbosity level to a default value */
   SetVerbosityLevel(VL_DEFAULT);

   switch ( argc )
   {
   case 1:
      ob = stdin_parse();
      break;
   case 2:
      ob = filename_parse(argv[1]);
      break;
   default:
      ob = NULL;
      ErrorMessage("ERROR: Unable to determine input options. Odd command line count.\n");
      break;
   }

   if ( ob )
   {
      SetVerbosityLevel(get_verbosity_level(ob));
   }
      
   return(ob);
}

/* ========================================================================= */
void EvalOptions(struct optbase *ob)
{
   assert ( NULL != ob );

   if ( ob->action == PARSE_EVAL )
   {
      printf("All options parsed correctly and within limits. Ok to run.\n");
      fflush(stdout);
      exit(0);
   }
}



/* ========================================================================= */
/* ========================================================================= */
/* ======================= BEGIN NEW PARSING METHODS ======================= */
/* ========================================================================= */
/* ========================================================================= */


/* ========================================================================= */
int name_to_ll(long long *val, struct optbase *ob, char *label)
{
   long long pval;
   long long sign;
   char *rawvalue;

   if ( NULL == (rawvalue = get_str_value(ob, label)) )
   {
      /* Not necessarily an error condition */
      return(1);
   }

   /* Strip off the sign - if it exists */
   sign = 1;
   if ( *rawvalue == '-' )
   {
      sign = -1;
      rawvalue++;
   }

   /* Walk the string building the number */
   pval = 0;
   while ( *rawvalue != 0 )
   {
      if ( ( *rawvalue >= '0' ) && ( *rawvalue <= '9' ) )
      {
         pval *= 10;
         pval += (long long)(*rawvalue - '0');
      }
      else
      {
         ErrorMessage("ERROR: Invalid character \'%c\' parsing \"%s\".\n", *rawvalue, label);
         exit(1);
      }

      rawvalue++;
   }

   /* Put the sign back */
   pval *= sign;

   *val = pval;
   return(0);
}

/* ========================================================================= */
int handle_uint8(struct optbase *ob,
                 char *pair_name,
                 uint8_t *data,
                 uint8_t *lower_bound,
                 uint8_t *upper_bound,
                 uint8_t *default_value)
{
   long long parsedval;
   uint8_t workval;

   /* Retrieve the numerical data for the label */
   if (name_to_ll(&parsedval, ob, pair_name))
   {
      /* Handle not finding the data */

      if ( NULL == default_value )
      {
         ErrorMessage("ERROR: No value provided for required option \"%s\".\n", pair_name);
         exit(1);
      }

      /* Use the default */
      *data = *default_value;
      return(0);
   }

   /* Test for absolute bounds */
   if (( parsedval < 0 ) || ( parsedval > UINT8_MAX ))
   {
      ErrorMessage("ERROR: Value for \"%s\" is out of bounds for the type.\n", pair_name);
      exit(1);
   }

   workval = parsedval;

   if ( lower_bound )
      if ( workval < *lower_bound )
      {
         ErrorMessage("ERROR: Value for \"%s\" is below limit of %d.\n", pair_name, *lower_bound);
         exit(1);
      }

   if ( upper_bound )
      if ( workval > *upper_bound )
      {
         ErrorMessage("ERROR: Value for \"%s\" is above limit of %d.\n", pair_name, *upper_bound);
         exit(1);
      }

   *data = workval;
   return(0);
}

/* ========================================================================= */
int handle_int8(struct optbase *ob,
                 char *pair_name,
                 int8_t *data,
                 int8_t *lower_bound,
                 int8_t *upper_bound,
                 int8_t *default_value)
{
   long long parsedval;
   int8_t workval;

   /* Retrieve the numerical data for the label */
   if (name_to_ll(&parsedval, ob, pair_name))
   {
      /* Handle not finding the data */

      if ( NULL == default_value )
      {
         ErrorMessage("ERROR: No value provided for required option \"%s\".\n", pair_name);
         exit(1);
      }

      /* Use the default */
      *data = *default_value;
      return(0);
   }

   /* Test for absolute bounds */
   if (( parsedval < INT8_MIN ) || ( parsedval > INT8_MAX ))
   {
      ErrorMessage("ERROR: Value for \"%s\" is out of bounds for the type.\n", pair_name);
      exit(1);
   }

   workval = parsedval;

   if ( lower_bound )
      if ( workval < *lower_bound )
      {
         ErrorMessage("ERROR: Value for \"%s\" is below limit of %d.\n", pair_name, *lower_bound);
         exit(1);
      }

   if ( upper_bound )
      if ( workval > *upper_bound )
      {
         ErrorMessage("ERROR: Value for \"%s\" is above limit of %d.\n", pair_name, *upper_bound);
         exit(1);
      }

   *data = workval;
   return(0);
}

/* ========================================================================= */
int handle_uint16(struct optbase *ob,
                 char *pair_name,
                 uint16_t *data,
                 uint16_t *lower_bound,
                 uint16_t *upper_bound,
                 uint16_t *default_value)
{
   long long parsedval;
   uint16_t workval;

   /* Retrieve the numerical data for the label */
   if (name_to_ll(&parsedval, ob, pair_name))
   {
      /* Handle not finding the data */

      if ( NULL == default_value )
      {
         ErrorMessage("ERROR: No value provided for required option \"%s\".\n", pair_name);
         exit(1);
      }

      /* Use the default */
      *data = *default_value;
      return(0);
   }

   /* Test for absolute bounds */
   if (( parsedval < 0 ) || ( parsedval > UINT16_MAX ))
   {
      ErrorMessage("ERROR: Value for \"%s\" is out of bounds for the type.\n", pair_name);
      exit(1);
   }

   workval = parsedval;

   if ( lower_bound )
      if ( workval < *lower_bound )
      {
         ErrorMessage("ERROR: Value for \"%s\" is below limit of %d.\n", pair_name, *lower_bound);
         exit(1);
      }

   if ( upper_bound )
      if ( workval > *upper_bound )
      {
         ErrorMessage("ERROR: Value for \"%s\" is above limit of %d.\n", pair_name, *upper_bound);
         exit(1);
      }

   *data = workval;
   return(0);
}

/* ========================================================================= */
int handle_int16(struct optbase *ob,
                 char *pair_name,
                 int16_t *data,
                 int16_t *lower_bound,
                 int16_t *upper_bound,
                 int16_t *default_value)
{
   long long parsedval;
   int16_t workval;

   /* Retrieve the numerical data for the label */
   if (name_to_ll(&parsedval, ob, pair_name))
   {
      /* Handle not finding the data */

      if ( NULL == default_value )
      {
         ErrorMessage("ERROR: No value provided for required option \"%s\".\n", pair_name);
         exit(1);
      }

      /* Use the default */
      *data = *default_value;
      return(0);
   }

   /* Test for absolute bounds */
   if (( parsedval < INT16_MIN ) || ( parsedval > INT16_MAX ))
   {
      ErrorMessage("ERROR: Value for \"%s\" is out of bounds for the type.\n", pair_name);
      exit(1);
   }

   workval = parsedval;

   if ( lower_bound )
      if ( workval < *lower_bound )
      {
         ErrorMessage("ERROR: Value for \"%s\" is below limit of %d.\n", pair_name, *lower_bound);
         exit(1);
      }

   if ( upper_bound )
      if ( workval > *upper_bound )
      {
         ErrorMessage("ERROR: Value for \"%s\" is above limit of %d.\n", pair_name, *upper_bound);
         exit(1);
      }

   *data = workval;
   return(0);
}

/* ========================================================================= */
int handle_uint32(struct optbase *ob,
                 char *pair_name,
                 uint32_t *data,
                 uint32_t *lower_bound,
                 uint32_t *upper_bound,
                 uint32_t *default_value)
{
   long long parsedval;
   uint32_t workval;

   /* Retrieve the numerical data for the label */
   if (name_to_ll(&parsedval, ob, pair_name))
   {
      /* Handle not finding the data */

      if ( NULL == default_value )
      {
         ErrorMessage("ERROR: No value provided for required option \"%s\".\n", pair_name);
         exit(1);
      }

      /* Use the default */
      *data = *default_value;
      return(0);
   }

   /* Test for absolute bounds */
   if (( parsedval < 0 ) || ( parsedval > UINT32_MAX ))
   {
      ErrorMessage("ERROR: Value for \"%s\" is out of bounds for the type.\n", pair_name);
      exit(1);
   }

   workval = parsedval;

   if ( lower_bound )
      if ( workval < *lower_bound )
      {
         ErrorMessage("ERROR: Value for \"%s\" is below limit of %d.\n", pair_name, *lower_bound);
         exit(1);
      }

   if ( upper_bound )
      if ( workval > *upper_bound )
      {
         ErrorMessage("ERROR: Value for \"%s\" is above limit of %d.\n", pair_name, *upper_bound);
         exit(1);
      }

   *data = workval;
   return(0);
}

/* ========================================================================= */
int handle_int32(struct optbase *ob,
                 char *pair_name,
                 int32_t *data,
                 int32_t *lower_bound,
                 int32_t *upper_bound,
                 int32_t *default_value)
{
   long long parsedval;
   int32_t workval;

   /* Retrieve the numerical data for the label */
   if (name_to_ll(&parsedval, ob, pair_name))
   {
      /* Handle not finding the data */

      if ( NULL == default_value )
      {
         ErrorMessage("ERROR: No value provided for required option \"%s\".\n", pair_name);
         exit(1);
      }

      /* Use the default */
      *data = *default_value;
      return(0);
   }

   /* Test for absolute bounds */
   if (( parsedval < INT32_MIN ) || ( parsedval > INT32_MAX ))
   {
      ErrorMessage("ERROR: Value for \"%s\" is out of bounds for the type.\n", pair_name);
      exit(1);
   }

   workval = parsedval;

   if ( lower_bound )
      if ( workval < *lower_bound )
      {
         ErrorMessage("ERROR: Value for \"%s\" is below limit of %d.\n", pair_name, *lower_bound);
         exit(1);
      }

   if ( upper_bound )
      if ( workval > *upper_bound )
      {
         ErrorMessage("ERROR: Value for \"%s\" is above limit of %d.\n", pair_name, *upper_bound);
         exit(1);
      }

   *data = workval;
   return(0);
}

/* ========================================================================= */
int handle_uint64(struct optbase *ob,
                 char *pair_name,
                 uint64_t *data,
                 uint64_t *lower_bound,
                 uint64_t *upper_bound,
                 uint64_t *default_value)
{
   long long parsedval;
   uint64_t workval;

   /* Retrieve the numerical data for the label */
   if (name_to_ll(&parsedval, ob, pair_name))
   {
      /* Handle not finding the data */

      if ( NULL == default_value )
      {
         ErrorMessage("ERROR: No value provided for required option \"%s\".\n", pair_name);
         exit(1);
      }

      /* Use the default */
      *data = *default_value;
      return(0);
   }

   /* Test for absolute bounds */
   if (( parsedval < 0 ) || ( parsedval > UINT64_MAX ))
   {
      ErrorMessage("ERROR: Value for \"%s\" is out of bounds for the type.\n", pair_name);
      exit(1);
   }

   workval = parsedval;

   if ( lower_bound )
      if ( workval < *lower_bound )
      {
         ErrorMessage("ERROR: Value for \"%s\" is below limit of %d.\n", pair_name, *lower_bound);
         exit(1);
      }

   if ( upper_bound )
      if ( workval > *upper_bound )
      {
         ErrorMessage("ERROR: Value for \"%s\" is above limit of %d.\n", pair_name, *upper_bound);
         exit(1);
      }

   *data = workval;
   return(0);
}

/* ========================================================================= */
int handle_int64(struct optbase *ob,
                 char *pair_name,
                 int64_t *data,
                 int64_t *lower_bound,
                 int64_t *upper_bound,
                 int64_t *default_value)
{
   long long parsedval;
   int64_t workval;

   /* Retrieve the numerical data for the label */
   if (name_to_ll(&parsedval, ob, pair_name))
   {
      /* Handle not finding the data */

      if ( NULL == default_value )
      {
         ErrorMessage("ERROR: No value provided for required option \"%s\".\n", pair_name);
         exit(1);
      }

      /* Use the default */
      *data = *default_value;
      return(0);
   }

   /* Test for absolute bounds */
   if (( parsedval < INT64_MIN ) || ( parsedval > INT64_MAX ))
   {
      ErrorMessage("ERROR: Value for \"%s\" is out of bounds for the type.\n", pair_name);
      exit(1);
   }

   workval = parsedval;

   if ( lower_bound )
      if ( workval < *lower_bound )
      {
         ErrorMessage("ERROR: Value for \"%s\" is below limit of %d.\n", pair_name, *lower_bound);
         exit(1);
      }

   if ( upper_bound )
      if ( workval > *upper_bound )
      {
         ErrorMessage("ERROR: Value for \"%s\" is above limit of %d.\n", pair_name, *upper_bound);
         exit(1);
      }

   *data = workval;
   return(0);
}

/* ========================================================================= */
int handle_boolean(struct optbase *ob,
                   char *pair_name,
                   mb_bool *data,
                   mb_bool *lower_bound,
                   mb_bool *upper_bound,
                   mb_bool *default_value)
{
   char *rawvalue;
   int ivalue;
   char VALUE[MAX_VALUE_SIZE];

   /* This returns a pointer to the matching value */
   if ( NULL == (rawvalue = get_str_value(ob, pair_name)) )
   {
      /* Handle not finding the data */

      if ( NULL == default_value )
      {
         ErrorMessage("ERROR: No value provided for required option \"%s\".\n", pair_name);
         exit(1);
      }

      /* Use the default */
      *data = *default_value;
      return(0);
   }

   copy_uc_string(VALUE, rawvalue);

   /* This is an impossible value that we will check for later */
   ivalue = -1;

   /* Test for potential true values */
   if ( 0 == strcmp(VALUE, "TRUE") )
      ivalue = 1;

   if ( 0 == strcmp(VALUE, "T") )
      ivalue = 1;

   if ( 0 == strcmp(VALUE, "YES") )
      ivalue = 1;

   if ( 0 == strcmp(VALUE, "ON") )
      ivalue = 1;

   if ( 0 == strcmp(VALUE, "1") )
      ivalue = 1;


   /* Test for potential false values */
   if ( 0 == strcmp(VALUE, "FALSE") )
      ivalue = 0;

   if ( 0 == strcmp(VALUE, "F") )
      ivalue = 0;

   if ( 0 == strcmp(VALUE, "NO") )
      ivalue = 0;

   if ( 0 == strcmp(VALUE, "OFF") )
      ivalue = 0;

   if ( 0 == strcmp(VALUE, "0") )
      ivalue = 0;


   /* If it was set, then it was parsed */
   if ( ivalue != -1 )
   {
      *data = ivalue;
      return(0);
   }

   /* Not parsed */
   ErrorMessage("ERROR: Unable to parse a proper boolean identifier for \"%s\".\n", pair_name);
   exit(1);

   /* unreachable */
   /* return(1); */
}


/* ========================================================================= */
int handle_time(struct optbase *ob,
                   char *pair_name,
                   time_t *data,
                   time_t *lower_bound,
                   time_t *upper_bound,
                   time_t *default_value)
{
   char *rawvalue;
   time_t pvalue;

   /* This returns a pointer to the matching value */
   if ( NULL == (rawvalue = get_str_value(ob, pair_name)) )
   {
      /* Handle not finding the data */

      if ( NULL == default_value )
      {
         ErrorMessage("ERROR: No value provided for required option \"%s\".\n", pair_name);
         exit(1);
      }

      /* Use the default */
      *data = *default_value;
      return(0);
   }

   /* Skip through whitespace */
   while ((( *rawvalue == ' ' ) || ( *rawvalue == '\t' )) && ( *rawvalue != 0 ))
      rawvalue++;

   if (( *rawvalue < '0' ) || ( *rawvalue > '9' ))
   {
      ErrorMessage("ERROR: Time input for \"%s\" is unparsable.\n", pair_name);
      exit(1);
   }

   /* Pull in the number */
   pvalue = 0;
   while (( *rawvalue >= '0' ) &&  ( *rawvalue <= '9' ))
   {
      pvalue *= 10;
      pvalue += *rawvalue - '0';
      rawvalue++;
   }

   /* Skip through white space (if it exists) */
   while ((( *rawvalue == ' ' ) || ( *rawvalue == '\t' )) && ( *rawvalue != 0 ))
      rawvalue++;

   switch ( *rawvalue )
   {
   case 0:  /* No factor value is the same as seconds */
   case 's':
   case 'S':
      break;
   case 'm':
   case 'M':
      pvalue *= 60;
      break;
   case 'h':
   case 'H':
      pvalue *= 3600;
      break;
   case 'd':
   case 'D':
      pvalue *= 86400;
      break;
   default:
      /* Nobody expects the unexpected. */
      ErrorMessage("ERROR: Unknown modifier found in time string for \"%s\".\n", pair_name);
      exit(1);
   }

   /* Check against expected bounds */
   if ( lower_bound )
      if ( pvalue < *lower_bound )
      {
         ErrorMessage("ERROR: Value for \"%s\" is below limit of %d.\n", pair_name, *lower_bound);
         exit(1);
      }

   if ( upper_bound )
      if ( pvalue > *upper_bound )
      {
         ErrorMessage("ERROR: Value for \"%s\" is above limit of %d.\n", pair_name, *upper_bound);
         exit(1);
      }

   *data = pvalue;
   return(0);
}

/* ========================================================================= */
int handle_string(struct optbase *ob,
                  char *pair_name,
                  char *data,
                  int *lower_bound,
                  int *upper_bound,
                  char *default_value)
{
   int val_len;
   char *rawvalue;


   /* This returns a pointer to the matching value */
   if ( NULL == (rawvalue = get_str_value(ob, pair_name)) )
   {
      /* Handle not finding the data */

      if ( NULL == default_value )
      {
         ErrorMessage("ERROR: No value provided for required option \"%s\".\n", pair_name);
         exit(1);
      }

      /* Use the default */
      strcpy(data, default_value);
      return(0);
   }

   val_len = strlen(rawvalue);

   /* Check against expected bounds */
   if ( lower_bound )
      if ( val_len < *lower_bound )
      {
         ErrorMessage("ERROR: Value for \"%s\" is shorter than %d chars.\n", pair_name, *lower_bound);
         exit(1);
      }

   if ( upper_bound )
      if ( val_len > *upper_bound )
      {
         ErrorMessage("ERROR: Value for \"%s\" is longer than %d chars.\n", pair_name, *upper_bound);
         exit(1);
      }

   strcpy(data, rawvalue);
   return(0);
}

/* ========================================================================= */
int handle_STRING(struct optbase *ob,
                  char *pair_name,
                  char *data,
                  int *lower_bound,
                  int *upper_bound,
                  char *default_value)
{
   int val_len;
   char *rawvalue;


   /* This returns a pointer to the matching value */
   if ( NULL == (rawvalue = get_str_value(ob, pair_name)) )
   {
      /* Handle not finding the data */

      if ( NULL == default_value )
      {
         ErrorMessage("ERROR: No value provided for required option \"%s\".\n", pair_name);
         exit(1);
      }

      /* Use the default */
      copy_uc_string(data, default_value);
      return(0);
   }

   val_len = strlen(rawvalue);

   /* Check against expected bounds */
   if ( lower_bound )
      if ( val_len < *lower_bound )
      {
         ErrorMessage("ERROR: Value for \"%s\" is shorter than %d chars.\n", pair_name, *lower_bound);
         exit(1);
      }

   if ( upper_bound )
      if ( val_len > *upper_bound )
      {
         ErrorMessage("ERROR: Value for \"%s\" is longer than %d chars.\n", pair_name, *upper_bound);
         exit(1);
      }

   copy_uc_string(data, rawvalue);
   return(0);
}

/* ========================================================================= */
int GetOptionValue(struct optbase *ob,
                   char *pair_name,
                   int data_type,
                   void *data,
                   void *lower_bound,
                   void *upper_bound,
                   void *default_value)
{
   assert(NULL != ob);
   assert(NULL != pair_name);
   assert(0 != pair_name[0]);
   assert(NULL != data);
   /* No assert on lower, upper, or default. */

   switch(data_type)
   {
	case GOV_BOOLEAN:
      return(handle_boolean(ob, pair_name, data, lower_bound, upper_bound, default_value));
	case GOV_UINT8:
      return(handle_uint8(ob, pair_name, (uint8_t*)data, (uint8_t*)lower_bound, (uint8_t*)upper_bound, (uint8_t*)default_value));
	case GOV_INT8:
      return(handle_int8(ob, pair_name, (int8_t*)data, (int8_t*)lower_bound, (int8_t*)upper_bound, (int8_t*)default_value));
	case GOV_UINT16:
      return(handle_uint16(ob, pair_name, (uint16_t*)data, (uint16_t*)lower_bound, (uint16_t*)upper_bound, (uint16_t*)default_value));
	case GOV_INT16:
      return(handle_int16(ob, pair_name, (int16_t*)data, (int16_t*)lower_bound, (int16_t*)upper_bound, (int16_t*)default_value));
	case GOV_UINT32:
      return(handle_uint32(ob, pair_name, (uint32_t*)data, (uint32_t*)lower_bound, (uint32_t*)upper_bound, (uint32_t*)default_value));
	case GOV_INT32:
      return(handle_int32(ob, pair_name, (int32_t*)data, (int32_t*)lower_bound, (int32_t*)upper_bound, (int32_t*)default_value));
	case GOV_UINT64:
      return(handle_uint64(ob, pair_name, (uint64_t*)data, (uint64_t*)lower_bound, (uint64_t*)upper_bound, (uint64_t*)default_value));
	case GOV_INT64:
      return(handle_int64(ob, pair_name, (int64_t*)data, (int64_t*)lower_bound, (int64_t*)upper_bound, (int64_t*)default_value));
	case GOV_TIME:
      return(handle_time(ob, pair_name, data, lower_bound, upper_bound, default_value));
	case GOV_STRING:
      return(handle_string(ob, pair_name, data, lower_bound, upper_bound, default_value));
	case GOV_UCSTRING:
      return(handle_STRING(ob, pair_name, data, lower_bound, upper_bound, default_value));
	case GOV_NONE:
   default:
      ErrorMessage("ERROR: Module library usage error. Unhandled name-value type.\n");
      exit(1);
   }
}
















































/* ========================================================================= */
/* ========================================================================= */
/* ====================== OLD CODE BELOW THIS LINE ========================= */
/* ========================================================================= */
/* ========================================================================= */



/* ========================================================================= */
int IsInvalidOption(struct optbase *ob, char *label, int test_type)
{
   struct optpair *op;
   char LABEL[MAX_LABEL_SIZE];
   char VALUE[MAX_VALUE_SIZE];
   int found;
   int isgood;   /* "Global" test pass/fail */
   int thistest; /* Per-test pass/fail */
   int i;        /* Index value - may be used more than once */

   /* Create an uppercase label */
   copy_uc_string(LABEL, label);

   /* Does it exist (at all) */
   op = ob->opstart;
   found = 0;
   while ( op )
   {
      if ( 0 == strcmp(op->name, LABEL) )
      {
         found = 1;
         break;
      }

      op = op->next;
   }

   if ( 0 == found )
      return(1);

   /* op is now on the matching pair */

   /* No tests have run. So the results are good */
   isgood = 1;

   copy_uc_string(VALUE, op->cpvalue);


   /* ============================== */
   if ( test_type & IVO_BOOLEAN )
   {
      thistest = 1;

      if ( VALUE[1] == 0 )
      {
         /* value is one character */
         switch ( VALUE[0] )
         {
         case '1':
         case '0':
         case 'T':
         case 'F':
            thistest = 0;
            break;
         }
      }
      else
      {
         if ( 0 == strcmp(VALUE, "TRUE") )
            thistest = 0;
         
         if ( 0 == strcmp(VALUE, "FALSE") )
            thistest = 0;

         if ( 0 == strcmp(VALUE, "ON") )
            thistest = 0;

         if ( 0 == strcmp(VALUE, "OFF") )
            thistest = 0;

         if ( thistest )
            isgood = 0;
      } /* if ( IVO_BOOLEAN ) */


      /* ============================== */
      if ( test_type & IVO_NUMERIC )
      {
         thistest = 0;
         i = 0;

         if ( op->cpvalue[0] == '-' )
            i++;

         while ( op->cpvalue[i] != 0 )
         {
            if ( ( op->cpvalue[i] < '0' ) || ( op->cpvalue[i] > '9' ) )
               thistest = 1;

            i++;

         } 

         if ( thistest )
            isgood = 0;
      }

      /* ============================== */
      if ( test_type & IVO_NNNUMERIC )
      {
         thistest = 0;
         i = 0;

         while ( op->cpvalue[i] != 0 )
         {
            if ( ( op->cpvalue[i] < '0' ) || ( op->cpvalue[i] > '9' ) )
               thistest = 1;

            i++;

         } 

         if ( thistest )
            isgood = 0;
      }


      if ( 0 == isgood )
         return(1);
   }


   return(0);
}

/* ========================================================================= */
int GetULValue(struct optbase *ob,
               unsigned long *setval,
               char *pair_name,
               unsigned long lower_bound,
               unsigned long upper_bound)
{
   struct optpair *op;
   unsigned long ulvalue;
   char LABEL[MAX_LABEL_SIZE];

   assert(NULL != setval);
   assert(NULL != pair_name);
   assert(0 != pair_name[0]);
   assert(lower_bound < upper_bound);

   copy_uc_string(LABEL, pair_name);

   /* Does it exist (at all) */
   op = ob->opstart;
   while ( op )
   {
      if ( 0 == strcmp(op->name, LABEL) )
      {
         /* This does not cover the range - stubbed for now */
         ulvalue = atol(op->cpvalue);

         if ( ulvalue < lower_bound )
            return(1);

         if ( ulvalue > upper_bound )
            return(1);

         *setval = ulvalue;
         return(0);
      }

      op = op->next;
   }


   return(1);
}

/* ========================================================================= */
int GetUSValue(struct optbase *ob,
               unsigned short *setval,
               char *pair_name,
               unsigned short lower_bound,
               unsigned short upper_bound)
{
   char *rawvalue;
   unsigned short usvalue;

   assert(NULL != setval);
   assert(NULL != pair_name);
   assert(0 != pair_name[0]);
   assert(lower_bound < upper_bound);

   if ( NULL == (rawvalue = get_str_value(ob, pair_name)) )
      return(1);

   /* This does not cover the range - stubbed for now */
   usvalue = atol(rawvalue);

   if ( usvalue < lower_bound )
      return(1);

   if ( usvalue > upper_bound )
      return(1);

   *setval = usvalue;
   return(0);
}

/* ========================================================================= */
int GetUCValue(struct optbase *ob,
               unsigned char *setval,
               char *pair_name,
               unsigned char lower_bound,
               unsigned char upper_bound)
{
   char *rawvalue;
   unsigned char ucvalue;

   assert(NULL != setval);
   assert(NULL != pair_name);
   assert(0 != pair_name[0]);
   assert(lower_bound < upper_bound);

   if ( NULL == (rawvalue = get_str_value(ob, pair_name)) )
      return(1);

   /* This does not cover the range - stubbed for now */
   ucvalue = atoi(rawvalue);

   if ( ucvalue < lower_bound )
      return(1);

   if ( ucvalue > upper_bound )
      return(1);

   *setval = ucvalue;
   return(0);
}

/* ========================================================================= */
int GetIValue(struct optbase *ob,
              int *setval,
              char *pair_name,
              int lower_bound,
              int upper_bound)
{
   char *rawvalue;
   int ivalue;

   assert(NULL != setval);
   assert(NULL != pair_name);
   assert(0 != pair_name[0]);
   assert(lower_bound < upper_bound);

   if ( NULL == (rawvalue = get_str_value(ob, pair_name)) )
      return(1);

   ivalue = atoi(rawvalue);

   if ( ivalue < lower_bound )
      return(1);

   if ( ivalue > upper_bound )
      return(1);

   *setval = ivalue;
   return(0);
}

/* ========================================================================= */
int GetStrValue(struct optbase *ob,
                char *setval,
                char *pair_name,
                int lower_bound,
                int upper_bound)
{
   int val_len;
   char *rawvalue;

   assert(NULL != setval);
   assert(NULL != pair_name);
   assert(0 != pair_name[0]);
   assert(lower_bound < upper_bound);
   assert(upper_bound <= MAX_LABEL_SIZE);

   /* This returns a pointer to the matching value */
   if ( NULL == (rawvalue = get_str_value(ob, pair_name)) )
      return(1);

   val_len = strlen(rawvalue);

   if ( val_len < lower_bound )
      return(1);

   if ( val_len > upper_bound )
      return(1);

   strcpy(setval, rawvalue);         
   return(0);
}

/* ========================================================================= */
int GetSTRValue(struct optbase *ob,
                char *setval,
                char *pair_name,
                int lower_bound,
                int upper_bound)
{
   int val_len;
   char *rawvalue;

   assert(NULL != setval);
   assert(NULL != pair_name);
   assert(0 != pair_name[0]);
   assert(lower_bound < upper_bound);
   assert(upper_bound <= MAX_LABEL_SIZE);

   /* This returns a pointer to the matching value */
   if ( NULL == (rawvalue = get_str_value(ob, pair_name)) )
      return(1);

   val_len = strlen(rawvalue);

   if ( val_len < lower_bound )
      return(1);

   if ( val_len > upper_bound )
      return(1);

   copy_uc_string(setval, rawvalue);         
   return(0);
}

/* ========================================================================= */
int GetTimeValue(struct optbase *ob,
               unsigned long *setval,
               char *pair_name,
               unsigned long lower_bound,
               unsigned long upper_bound)
{
   unsigned long ulvalue;
   char *rawvalue;

   assert(NULL != setval);
   assert(NULL != pair_name);
   assert(0 != pair_name[0]);
   assert(lower_bound < upper_bound);

   ulvalue = 0;

   /* This returns a pointer to the matching value */
   if ( NULL == (rawvalue = get_str_value(ob, pair_name)) )
      return(1);

   /* Skip through whitespace */
   while ((( *rawvalue == ' ' ) || ( *rawvalue == '\t' )) && ( *rawvalue != 0 ))
      rawvalue++;

   /* Pull in the number */
   while (( *rawvalue >= '0' ) &&  ( *rawvalue <= '9' ))
   {
      ulvalue *= 10;
      ulvalue += *rawvalue - '0';
      rawvalue++;
   }

   /* Skip through white space */
   while ((( *rawvalue == ' ' ) || ( *rawvalue == '\t' )) && ( *rawvalue != 0 ))
      rawvalue++;

   switch ( *rawvalue )
   {
   case 0:  /* No factor value is the same as seconds */
   case 's':
   case 'S':
      break;
   case 'm':
   case 'M':
      ulvalue *= 60;
      break;
   case 'h':
   case 'H':
      ulvalue *= 3600;
      break;
   case 'd':
   case 'D':
      ulvalue *= 86400;
      break;
   default:
      /* Nobody expects the unexpected. */
      return(1);
   }

   *setval = ulvalue;
   return(0);
}

/* ========================================================================= */
int GetBoolValue(struct optbase *ob,
                 int *setval,
                 char *pair_name)
{
   char *rawvalue;
   int ivalue;
   char VALUE[MAX_VALUE_SIZE];

   assert(NULL != setval);
   assert(NULL != pair_name);
   assert(0 != pair_name[0]);

   /* This returns a pointer to the matching value */
   if ( NULL == (rawvalue = get_str_value(ob, pair_name)) )
      return(1);

   copy_uc_string(VALUE, rawvalue);

   /* This is an impossible value that we will check for later */
   ivalue = -1;

   /* Test for potential true values */
   if ( 0 == strcmp(VALUE, "TRUE") )
      ivalue = 1;

   if ( 0 == strcmp(VALUE, "T") )
      ivalue = 1;

   if ( 0 == strcmp(VALUE, "YES") )
      ivalue = 1;

   if ( 0 == strcmp(VALUE, "ON") )
      ivalue = 1;

   if ( 0 == strcmp(VALUE, "1") )
      ivalue = 1;


   /* Test for potential false values */
   if ( 0 == strcmp(VALUE, "FALSE") )
      ivalue = 0;

   if ( 0 == strcmp(VALUE, "F") )
      ivalue = 0;

   if ( 0 == strcmp(VALUE, "NO") )
      ivalue = 0;

   if ( 0 == strcmp(VALUE, "OFF") )
      ivalue = 0;

   if ( 0 == strcmp(VALUE, "0") )
      ivalue = 0;


   /* If it was set, then it was parsed */
   if ( ivalue != -1 )
   {
      *setval = ivalue;
      return(0);
   }

   /* Not parsed */
   return(1);
}
