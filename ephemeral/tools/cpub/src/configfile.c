#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "providers.h"
#include "mainhelp.h"
#include "configfile.h"

/* Discussion
   - Error messages in this code can simply go to stderr as this will be
     called long before any potential fork()s (and closing of stderr).
   - Returning (an error) without freeing memory seems a bit dangerous. But
     failures to parse data are treated as critical errors. If we fail to
     understand what the user is requesting, then we will send an error
     message and die. It is NOT appropriate to muddle on in the face of 
     an obvious error.

   - Config file parsing is not a "full" parse. Meaning, if some syntatical
     errors are encountered, then they will be ignored, and not errored, or
     (attempted to) parseed.
   - The config file parsing is not responsible for "comprehending" the
     items in the writer args or the provider list. It only insures that 
     they are of proper format, and then puts them in a parsed format
     that makes it easier for the writer/provider to utilize.
*/



/* ========================================================================= */
struct prov_name *init_pname(void)
{
   struct prov_name *p;

   if ( NULL == (p = (struct prov_name *)malloc(sizeof(struct prov_name))) )
   {
      error_msg("ERROR: Failed to allocate memory for a provider list.");
      return(NULL);
   }

   p->quad = NULL;
   p->args = NULL;  /* The raw argument string(s) */

   p->header = NULL;
   p->alarm_type = ALARM_NO;
   p->alarm_value = 0;
   p->fact_from = FACT_FROM_NONE;
   p->fact_to = FACT_TO_NONE;

   p->next = NULL;

   return(p);
}

/* ========================================================================= */
struct cfg_wm_arg *init_warg(char *lhs, char *rhs)
{
   struct cfg_wm_arg *wa;

   if ( lhs == NULL )
   {
      error_msg("ERROR: Internal error parsing writer arg list.");
      return(NULL);
   }

   if ( NULL == (wa = (struct cfg_wm_arg *)malloc(sizeof(struct cfg_wm_arg))) )
   {
      error_msg("ERROR: Failed to allocate memory for a provider list.");
      return(NULL);
   }

   if ( NULL == ( wa->option = mkstring(lhs) ) )
      return(NULL);

   /* RHS *could* be NULL */
   if ( NULL == rhs )
   {
      wa->value = NULL;
   }
   else
   {
      if ( NULL == ( wa->value = mkstring(rhs) ) )
         return(NULL);
   }

   wa->next = NULL;

   return(wa);
}

/* ========================================================================= */
char *parse_string_arg(char *args, char *thestring)
{
   char arg_data[32];
   char *loc;
   int usesq = 0; /* Uses quotes to delimit text */
   int i;

   if ( NULL == (loc = StrStr(args, thestring)))
      return(NULL);

   loc += strlen(thestring); /* Now after the string */

   while (( *loc == ' ' ) || ( *loc == '=' ) || ( *loc == '\t' ))
      loc++;

   if ( *loc == '"' )
   {
      usesq = 1;
      loc++;
   }

   if ( usesq )
   {
      i = 0;
      while( (loc[i] != '"') && (loc[i] != 0) )
      {
         arg_data[i] = loc[i];
         i++;
      }
      arg_data[i] = 0;
   }
   else
   {
      i = 0;
      while( (loc[i] != ' ') && (loc[i] != 0) && (loc[i] != '\t') )
      {
         arg_data[i] = loc[i];
         i++;
      }
      arg_data[i] = 0;
   }

   /* I reuse loc here for a different purpose */
   if ( NULL == (loc = mkstring(arg_data)) )
      return(NULL);

   return(loc);
}

/* ========================================================================= */
/* A short discussion on factoring
    The rules for the iarg string:
    - Always begins with the 4 letters: "fact"
    - Must include a from and to clause
    - From and to clause are surrounded by ()
    - From and to are separated by "to"
    The possible from args are:
    - 'bi'   Input data is in bits (not supported at this time) 
    - 'b'    Input data is in Bytes
    - 'k'    Input data is in KBytes
    - '4kp'  Input data is in 4K Pages
    - '8kp'  Input data is in 8K Pages
    - '64kp' Input data is in 64K Pages
    The possible to args are:
    - 'a'    Auto - Each item will be auto-ranged
*/
int parse_factor_args(struct prov_name *p)
{
   char FDATA[32];
   char *loc;
   int usesq = 0; /* Uses quotes to delimit text */
   int i;

   if ( NULL == (loc = StrStr(p->args, "factor")))
      return(1);

   loc += strlen("factor"); /* Now after the string */

   /* Move off whitespace */
   while (( *loc == ' ' ) || ( *loc == '\t' ))
      loc++;

   /* An "=" is expected at this point */
   if ( *loc == '=' )
      loc ++;
   else
      return(1);

   /* Move off whitespace */
   while (( *loc == ' ' ) || ( *loc == '\t' ))
      loc++;


   if ( *loc == '"' )
   {
      usesq = 1;
      loc++;
   }

   if ( usesq )
   {
      i = 0;
      while( (loc[i] != '"') && (loc[i] != 0) )
      {
         if (( loc[i] >= 'a' ) && ( loc[i] <= 'z' ))
            FDATA[i] = loc[i] - 32;
         else
            FDATA[i] = loc[i];

         i++;
      }
      FDATA[i] = 0;
   }
   else
   {
      i = 0;
      while( (loc[i] != ' ') && (loc[i] != 0) && (loc[i] != '\t') )
      {
         if (( loc[i] >= 'a' ) && ( loc[i] <= 'z' ))
            FDATA[i] = loc[i] - 32;
         else
            FDATA[i] = loc[i];

         i++;
      }
      FDATA[i] = 0;
   }

   i = 0;
   /* Now look for patterns */
   if ( ( FDATA[0] == 'B' ) && ( FDATA[1] == 'T' ) && ( FDATA[2] == 'O' ) )
   {
      p->fact_from = FACT_FROM_BYTES;
      i = 3;
   }

   if ( ( FDATA[0] == 'D' ) && ( FDATA[1] == 'T' ) && ( FDATA[2] == 'O' ) )
   {
      p->fact_from = FACT_FROM_DECIMAL;
      i = 3;
   }

   if ( ( FDATA[0] == 'K' ) && ( FDATA[1] == 'T' ) && ( FDATA[2] == 'O' ) )
   {
      p->fact_from = FACT_FROM_KBYTES;
      i = 3;
   }

   if ( ( FDATA[0] == '4' ) && ( FDATA[1] == 'K' ) && ( FDATA[2] == 'T' ) && ( FDATA[3] == 'O' ) )
   {
      p->fact_from = FACT_FROM_4KPAGE;
      i = 4;
   }

   if ( ( FDATA[0] == '8' ) && ( FDATA[1] == 'K' ) && ( FDATA[2] == 'T' ) && ( FDATA[3] == 'O' ) )
   {
      p->fact_from = FACT_FROM_8KPAGE;
      i = 4;
   }

   if ( ( FDATA[0] == '6' ) &&  ( FDATA[1] == '4' ) && ( FDATA[2] == 'K' ) && ( FDATA[3] == 'T' ) && ( FDATA[4] == 'O' ) )
   {
      p->fact_from = FACT_FROM_64KPAGE;
      i = 5;
   }

   /* The from value was not parsed */
   if ( p->fact_from == 0 )
      return(1);


   /* Look for destination value */
   if ( FDATA[i] == 'A' )
   {
      if ( p->fact_from == FACT_FROM_DECIMAL )
         p->fact_to = FACT_TO_AUTOD;
      else
         p->fact_to = FACT_TO_AUTOB;
   }

   if ( FDATA[i] == 'H' )
   {
      if ( p->fact_from == FACT_FROM_DECIMAL )
         p->fact_to = FACT_TO_HEX;
      else
         return(1); /* Can only be from decimal when going to hex */
   }

   /* If to value is not set, clear from and exit with error */
   if ( p->fact_to == 0 )
   {
      p->fact_from = 0;
      return(1);
   }

   return(0);
}

/* ========================================================================= */
int parse_alarm_args(struct prov_name *p)
{
   char arg_data[32];
   char *loc;
   int usesq = 0; /* Uses quotes to delimit text */
   int i;
   int alarm_type;  /* Temp storage for p->alarm_type */
   int strlimit;    /* Maximum length of input string data (# of chars in long long) */

   if ( NULL == (loc = StrStr(p->args, "alarm_at")))
      return(1);

   loc += strlen("alarm_at"); /* Now after the string */

   while (( *loc == ' ' ) || ( *loc == '\t' ))
      loc++;


   alarm_type = ALARM_NO;
   if (( loc[0] == '=' ) && ( loc[1] == '=' ))
   {
      alarm_type = ALARM_EQ;
      loc += 2;
   }

   if (( loc == strstr(loc, "!=") ) || ( loc == strstr(loc, "<>") ))
   {
      alarm_type = ALARM_NE;
      loc += 2;
   }

   if ( loc[0] == '<' )
   {
      if ( loc[1] == '=' )
      {
         alarm_type = ALARM_LE;
         loc += 2;
      }
      else if ( loc[1] != '>' ) /* Make sure <> is not picked up */
      {
         alarm_type = ALARM_LE;
         loc += 1;
      }
   }

   if ( loc == strstr(loc, ">") )
   {
      if ( loc == strstr(loc, ">=") )
      {
         alarm_type = ALARM_GE;
         loc += 2;
      }
      else
      {
         alarm_type = ALARM_GT;
         loc += 1;
      }
   }

   /* Go ahead and check alarm_type - before pressing on */
   if ( alarm_type == ALARM_NO )
      return(1);

   /* Move off the white space - if it exists */
   while (( *loc == ' ' ) || ( *loc == '\t' ))
      loc++;


   if ( *loc == '"' )
   {
      usesq = 1;
      loc++;
   }

   if ( usesq )
   {
      i = 0;
      while( (loc[i] != '"') && (loc[i] != 0) )
      {
         arg_data[i] = loc[i];
         i++;
      }
      arg_data[i] = 0;
   }
   else
   {
      i = 0;
      while( (loc[i] != ' ') && (loc[i] != 0) && (loc[i] != '\t') )
      {
         arg_data[i] = loc[i];
         i++;
      }
      arg_data[i] = 0;
   }

   if ( is_numeric(arg_data) )
   {
      /* Basically an order of magnitude is chopped off here. By this I
         mean: There is (roughly) an order of magnitude between what the
         user can enter and what the variable can hold. But these
         numbers are so insanely large it should not matter. There is
         no concievable test at this range (other than FFFFFFFFFFFFFFFF
         and there is no plan to test for this).
      */ 
      if ( arg_data[0] == '-' )
      {
         strlimit = 19;
         i = 1;
      }
      else
      {
         strlimit = 18;
         i = 0;
      }

      if ( strlimit < strlen(arg_data) )
         return(1);

      p->alarm_value = 0;
      while(arg_data[i] != 0)
      {
         p->alarm_value *= 10;
         p->alarm_value += (arg_data[i] - '0');

         i++;
      }
   }
   else
      return(1);
   
   /* alarm_type is not set until the alarm_value is properly parsed */
   p->alarm_type = alarm_type;

   return(0);
}

/* ========================================================================= */
int parse_args(struct prov_name *p)
{
   if ( NULL == p )
      return(1);

   if ( NULL == p->args )
      return(0);

   if ( p->args[0] == 0 )
      return(1);

   if ( StrStr(p->args, "header") )
   {
      p->header = parse_string_arg(p->args, "header");
   }

   if ( StrStr(p->args, "alarm_at") )
   {
      if ( parse_alarm_args(p) )
         return(1);
   }

   if ( StrStr(p->args, "factor") )
   {
      if ( parse_factor_args(p) )
      {
         return(1);
      }
   }

   return(0);
}

/* ========================================================================= */
int add_quad_line(struct config *c, char *line)
{
   int cc;       /* colon count */
   int i,j;      /* Used to walk through char arrays */
   char *ls;     /* Used to take away leading space */

   struct prov_name *p;
   struct prov_name *this_p;
   struct prov_name *last_p;

   /* Skip over leading space */
   ls = line;
   while (( *ls == ' ' ) || ( *ls == '\t' ))
      ls++;

   /* ls now sits at the beginning of data (or a CR) */

   i = 0;
   cc = 0;
   while( (ls[i] != 0) && (ls[i] != ' ') && (ls[i] != '\t'))
   {
      if ( ls[i] == ':' )
         cc++;

      i++;
   }

   /* Invalid number of colons in the quad */
   if (( cc < 2 ) || ( cc > 3 ))
      return(1);

   /* We have sufficient cause to allocate memory */
   if (NULL == (p = init_pname()))
      return(-1);

   /* i still holds the length of the quad */
   if ( NULL == (p->quad = (char *)malloc(strlen(line) + 2)) )
      return(-1);

   /* Copy over the quad (AND args) */
   i = 0;
   j = 0;
   while(ls[j] != 0)
   {
      if ((ls[i] != 0) && (ls[i] != ' ') && (ls[i] != '\t'))
         i++;

      p->quad[j] = ls[j];

      j++;
   }
   p->quad[j] = 0;

   /* Is there more data? */
   if ( ls[i] != 0 )
   {
      /* Move off the white space */
      while( (ls[i] == ' ') || (ls[i] == '\t'))
         i++;

      /* j is length of the whole string. i is where the data starts */
      j = j - i;
      /* j is now how long the (remaining) data is */ 

      if ( j > 2 ) /* 2 = arbitrarliy chosen length of shortest possible arg */
      {
         /* j holds the length of the args */
         if ( NULL == (p->args = (char *)malloc(j + 2)) )
            return(-1);

         /* Copy the data into the args string */
         j = 0;
         /* while (( ls[i] != 0 ) && ( ls[i] != '#' )) - comment is already trimmed */
         while ( ls[i] != 0 )
         {
            p->args[j++] = ls[i++];
         }
         p->args[j] = 0;
      }

      if ( parse_args(p) ) /* This converts raw args to something we understand/use */
      {
         return(-1);
      }
   }

   /* Add to list */
   if(NULL == c->pnlist)
   {
      c->pnlist = p;
   }
   else
   {
      /* The list insert MUST BE ORDERED */

      /* Walk to the end and insert */
      this_p = c->pnlist->next;
      last_p = c->pnlist;

      while(this_p)
      {
         last_p = this_p;
         this_p = this_p->next;
      }

      last_p->next = p;
   }

   /* Add one to the count of quads registered */
   c->quad_count++;

   return(0);
}

/* ========================================================================= */
int add_arg_line(struct config *c, char *line)
{
   struct cfg_wm_arg *thiswa;
   int i;       /* Used to walk through char arrays */
   char *ls;    /* Used to take away leading space */
   char *rhs;   /* Rhight hand side of the equation */
   char *lhs;   /* Left hand side of the equation */
   char qfirst, qlast;

   /* Let's check the input */
   assert( line != NULL );
   assert( c != NULL );
   if ( line[0] == 0 )
      return(0); /* Blank lines are not an issue */

   /* Start by assuming we have neither rhs or lhs */
   rhs = NULL;
   lhs = NULL;

   /* Skip over leading space */
   ls = unlead(line);

   /* Strip off EOL marker */
   chomp(line);

   /* Strip off trailing comments */
   clamp(line);

   /* Strip off trailing white space */
   wstrim(line);

   /* ls now sits at the beginning of data (or a CR) */
   if ( ls[0] == 0 )
      return(0);     /* Not an error, just no data (possibly was a comment) */

   /* Determine RHS and LHS */
   i = 0;
   while ( (ls[i] != 0 ) && ( ls[i] != '=' ) )
      i++;

   if ( ls[i] == '=' )
   {
      ls[i] = 0;

      /* There is a LHS and (potentially) a RHS */
      lhs = ls;
      
      if ( ls[i + 1] != 0 )
      {
         rhs = &ls[i + 1];
      }
   }

   if ( ls[i] == 0 )
   {
      lhs = ls;
   }

   /* Clean up the data a bit */
   wstrim(lhs);  /* Space between the option and the = */

   if ( rhs ) /* Don't try to clean it if it is null */
   {
      rhs = unlead(rhs); /* Space between the value and the = */

      /* Un-quote it */
      qfirst = rhs[0];
      qlast = rhs[strlen(rhs) - 1];

      /* " */
      if ((qfirst == 34) && (qlast == 34))
      {
         rhs++;
         rhs[strlen(rhs) - 1] = 0;
      }

      /* ' */
      if ((qfirst == 39) && (qlast == 39))
      {      
         rhs++;
         rhs[strlen(rhs) - 1] = 0;
      }
   }
 
   /* Make it happen ---> allocate memory */
   if ( NULL == ( thiswa = init_warg(lhs, rhs) ) )
      return(-1);


   thiswa->next = c->walist;
   c->walist = thiswa;

   return(0);
}

/* ========================================================================= */
int parse_time_string(unsigned long *seconds, char *cpin)
{
   unsigned long number;
   unsigned long mmulti;  /* The memnonic multiplier (s, m, h, d) */
   int i;

   assert(seconds != NULL);
   
   /* This is marginally an error... but I will allow it to live as
      the intent being 0. */
   if ( NULL == cpin )
   {
      *seconds = 0;
      return(0);
   }

   /* Read off the number */
   i = 0;
   number = 0;
   while(( cpin[i] >= '0' ) && ( cpin[i] <= '9' ))
   {
      number *= 10;
      number += cpin[i] - '0';
      i++;
   }

   /* Do a basic sanity check */
   if ( number == 0 )
   {
      if ( cpin[0] != '0' )
      {
         /* Result is 0, but input was not 0... */
         return(1);
      }
   }

   /* Move off a space, if it exists */
   while (( cpin[i] == ' ' ) || ( cpin[i] == '\t' ))
      i++;

   /* Parse off the memnonic multiplier */
   switch ( cpin[i] )
   {
   case 's':
   case 'S':
   case 0:
      mmulti = 1;
      break;
   case 'm':
   case 'M':
      mmulti = 60;
      break;
   case 'h':
   case 'H':
      mmulti = 60 * 60;
      break;
   case 'd':
   case 'D':
      mmulti = 60 * 60 * 24;
      break;
   case 'w':
   case 'W':
      mmulti = 60 * 60 * 24 * 7;
      break;
   default:
      return(1);
      break; /* Serving compiler sanity only */
   }

   /* Do the math, return */
   *seconds = number * mmulti;
   return(0);
}

/* ========================================================================= */
int add_equality(struct config *c, char *line)
{
   char *rhs;

   /* find the rhs */
   rhs = strrchr(line, '=');

   while(*rhs == '=') /* move off the = */
      rhs++;

   while(*rhs == ' ') /* move off the spaces */
      rhs++;

   if ( *rhs == 0 )
      return(1);

   if ( line == StrStr(line, "run_every") )
   {
      if ( is_numeric(rhs) )
         c->run_every = atoi(rhs);
      else
         return(1);

      return(0);
   }

   if ( line == StrStr(line, "danger_mode") )
   {
      if (( NULL != StrStr(rhs, "accept") ) || ( NULL != StrStr(rhs, "acknowledge") ) || ( NULL != StrStr(rhs, "danger") ))
      {
         c->danger_mode = DANGER_ACK;
         return(0);
      }

      return(0);
   }

   if ( line == StrStr(line, "run_for") )
   {
      return(parse_time_string(&c->run_for, rhs));
   }

   if ( line == StrStr(line, "run_method") )
   {
      /* Include alternate (wrong) spelling */
      if (( NULL != StrStr(rhs, "daemon") ) || ( NULL != StrStr(rhs, "demon") ) || ( NULL != StrStr(rhs, "background") ))
      {
         c->run_method = RM_DAEMON;
         return(0);
      }

      /* This is the default, really only catching the input here so that
         this line is not seen as an error */
      if (( NULL != StrStr(rhs, "foreground") ) || ( NULL != StrStr(rhs, "interactive") ))
      {
         c->run_method = RM_FOREGROUND;
         return(0);
      }
   }

   if ( line == StrStr(line, "send_to") )
   {
      /* New writers must be "registered" here */
      if ( StrStr(rhs, "stdout") )
      {
         c->send_to = WM_STDOUT_TYPE;
         return(0);
      }

      if ( StrStr(rhs, "file") )
      {
         c->send_to = WM_FILE_TYPE;
         return(0);
      }

      if ( StrStr(rhs, "alarm") )
      {
         c->send_to = WM_ALARM_TYPE;
         return(0);
      }

      if ( StrStr(rhs, "bcd") )
      {
         c->send_to = WM_BCD_TYPE;
         return(0);
      }
   }

   return(1);
}

/* ========================================================================= */
int is_equality(char *line)
{
   int i;

   if ( NULL == line )
      return(0);

   if ( line[0] == '#' )
      return(0);

   i = 0;
   while( line[i] != 0 )
   {
      /* If a # is reached before a =, then not a true equality */
      if ( line[i] == '#' )
         return(0);

      if ( line[i] == '=' )
         return(1); /* Weak match - but sufficient for now */

      i++;
   }

   return(0);
}

/* ========================================================================= */
struct config *init_config(void)
{
   struct config *c;

   if ( NULL == (c = (struct config *)malloc(sizeof(struct config))) )
   {
      error_msg("ERROR: Unable to allocate memory for config data.");
      return(NULL);
   }

   c->run_for = 0;
   c->run_every = 1000;
   c->send_to = 0;
   c->run_method = RM_DEFAULT;
   c->danger_mode = DANGER_NACK;

   c->quad_count = 0;

   c->pnlist = NULL;
   c->walist = NULL;

   return(c);
}

/* ========================================================================= */
struct config *ReadConfigFile(char *filename)
{
   FILE *f;
   char whole_line[MAX_CLINE_LEN];
   char *line;
   struct config *c;
   int in_plist;
   int in_alist;
   int lineno;
   int error_stack;  /* Used to let errors stack up */
   int rv;
   char errstr[80];

   if ( NULL == ( c = init_config() ) )
      return(NULL);

   if ( NULL == ( f = fopen(filename, "r") ) )
   {
      dotdotdot_string(errstr, filename, 40);
      error_msg("ERROR: Unable to open config file \"%s\".", errstr); 
      return(NULL);
   }

   in_plist = 0;
   in_alist = 0;
   lineno = 0;
   while(fgets(whole_line, MAX_CLINE_LEN, f))
   {
      lineno++;

      line = unlead(whole_line);

      /* chomp() - Remove EOL */
      chomp(line);

      /* clamp() - Remove trailing comments */
      clamp(line);

      /* wstrim() - Remove trailing spaces */
      wstrim(line);

      if ( line[0] == 0 )
         continue;

      /* Does this end a provider list? */
      if ( line == strstr(line, "PROVIDER_LIST_END") )
      {
         in_plist = 0;
         continue;
      }

      /* Does this end an args list? */
      if ( line == strstr(line, "WRITER_ARGS_END") )
      {
         in_alist = 0;
         continue;
      }

      if ( in_plist )
      {
         if ( 0 != ( rv = add_quad_line(c, line) ) )
         {
            string_dotdotdot(errstr, line, 30);
            
            if ( rv == 1 )
               error_msg("ERROR: Failed to parse config line number %d: [%s]", lineno, errstr);
            else
               error_msg("ERROR: Internal error parsing config line %d: [%s]", lineno, errstr);

            return(NULL);
         }
         continue;
      }

      if ( in_alist )
      {
         if ( 0 != ( rv = add_arg_line(c, line) ) )
         {
            string_dotdotdot(errstr, line, 30);
            
            if ( rv == 1 )
               error_msg("ERROR: Failed to parse config line number %d: [%s]", lineno, errstr);
            else
               error_msg("ERROR: Internal error parsing config line %d: [%s]", lineno, errstr);

            return(NULL);
         }

         continue;
      }

      if ( line == strstr(line, "PROVIDER_LIST_BEGIN") )
      {
         in_plist = 1;
         continue;
      }

      /* Does this begin an args list? */
      if ( line == strstr(line, "WRITER_ARGS_BEGIN") )
      {
         in_alist = 1;
         continue;
      }

      if ( is_equality(line) )
      {
         if ( add_equality(c, line) )
         {
            string_dotdotdot(errstr, line, 30);
            error_msg("ERROR: Failed to parse config line number %d: [%s]", lineno, errstr);
            /* Gonna bail - This is better than mis-interpreting what the user wanted */
            /* No free() of memory - we are dead man walking anyways */
            return(NULL);
         }
      }
      else
      {
         string_dotdotdot(errstr, line, 30);
         error_msg("ERROR: Failed to parse config line number %d: [%s]", lineno, errstr);
         /* Gonna bail - This is better than mis-interpreting what the user wanted */
         /* No free() of memory - we are dead man walking anyways */
         return(NULL);
      }
   }

   fclose(f);

   error_stack = 0;  /* Start off with no errors */

   /* Validate some of the input values */
   if ( c->run_every < 100 )
   {
      error_msg("ERROR: Sampling threshold is below the minimum 100ms value.");
      error_stack = 1;
   }

   if ( c->send_to == 0 )
   {
      /* This error condition is *different* than the "you specified a wrong send_to option".
         This is about not specifying send_to at all. */
      error_msg("ERROR: The writer method is not specified with the \"send_to\" option.");
      error_stack = 1;
   }

   /* Additional errors may be tested for / detected here */




   /* Return error if we found any errors */
   if ( error_stack )
      return(NULL);


   return(c);
}












