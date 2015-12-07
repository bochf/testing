#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
/* For the timeval and timespec data types */
#include <time.h>
#include <sys/time.h>

#include "../mainhelp.h"
#include "wmalarm.h"

/*
  What has to happen here:

  This is happening (the code is written):
  - The data is promoted in the config file reader. The compare must happen
    at the larger size, rather than "negotiating" a size. So the compare is
    done as an int64_t (regardless of the native type).
  This is a suggestion (the code does not exist):
  - The alarm event needs to be defined and supported. Meaning it has to 
    be validated in the alarm init code. Some potential alarm targets:
     - tracestop    <----- AIX specific trace stop
     - actlog       <----- Standardized message to act.log
     - filelog      <----- To a specific file.
*/

/* ========================================================================= */
/* These are values for the alarm_to value. They will be used repeatedly
   in switch() statements to determine how the alarm should be handled. */
#define ALRM_TO_NOWHERE  0    /* Error condition - value not set */
#define ALRM_TO_STDOUT   1 
#define ALRM_TO_FILE     2
#define ALRM_TO_ACTLOG   3
#define ALRM_TO_SYSLOG   4

/* ========================================================================= */
struct alrm_data
{
   int TYPE;            /* The MAGIC used to recognize/validate this struct  */
   int alarm_to;        /* Possible palces to alarm to. See: ALRM_TO_*       */
   char *actlog_tag;    /* Tag to use for act.log messages                   */
   int survive_turn;    /* No errors if we cant write to act.log             */
   int include_timestamp; /* Include a timestamp in the output               */
   char *file_name;     /* the file name when using the file alarm_to option */ 
   struct proot *p;
   struct config *cfg;
};

/* ========================================================================= */
void *InitAlarmWM(struct proot *p, struct config *cfg)
{
   void *rv;
   struct alrm_data *wmalrm;
   struct cfg_wm_arg *ap;
   struct pitem *this_pi;

   /* Used to parse the option/value pairs                                   */
   int exit_on_error; 
   int bad_value;
   int bad_option;

   /* Allocate memory for the common structure (For this writer's functions) */
   if ( NULL == ( rv = malloc(sizeof(struct alrm_data)) ) )
   {
      error_msg("ERROR: Failed to allocate memory for alarm writer data.");
      return(NULL);
   }

   /* Just one cast rather than many */
   wmalrm = (struct alrm_data *)rv;

   /* Set defaults */
   wmalrm->TYPE = WM_ALARM_TYPE; /* This TYPE is devined in ../configfile.h */
   wmalrm->alarm_to = ALRM_TO_NOWHERE;  /* default = error condition */
   wmalrm->actlog_tag = NULL;           /* default = will be set later */
   wmalrm->survive_turn = 1;
   wmalrm->include_timestamp = 1;
   wmalrm->file_name = NULL;
   wmalrm->p = p;

   /* Parse what we got as options */
   ap = cfg->walist;
   exit_on_error = 0; /* Assume no errors to start */
   while(ap)
   {
      bad_option = 1; /* Assume that option is bad from the start */

      if ( ap->option == StrStr(ap->option, "alarm_to") )
      {
         bad_option = 0; /* This is a valid option */
         bad_value = 1;  /* Now assume value is bad */

         if ( ap->value == StrStr(ap->value, "stdout") )
         {
            bad_value = 0; /* stdout is recognized */
            wmalrm->alarm_to = ALRM_TO_STDOUT;
         }

         if ( ap->value == StrStr(ap->value, "actlog") )
         {
            bad_value = 0; /* actlog is recognized */
            wmalrm->alarm_to = ALRM_TO_ACTLOG;
         }

         if ( ap->value == StrStr(ap->value, "file") )
         {
            bad_value = 0; /* file is recognized */
            wmalrm->alarm_to = ALRM_TO_FILE;
         }

         if ( bad_value )
         {
            error_msg("ERROR: Unable to parse \"%s\" argument to alarm_to writer option.",
                    ap->value);
            exit_on_error = 1; /* Don't exit. Let errors stack up. */
         }
      }

      if ( ap->option == StrStr(ap->option, "actlog_tag") )
      {
         bad_option = 0; /* This is a valid option */

         if ( ap->value[0] == 0 )
         {
            error_msg("ERROR: Unable to parse argument to actlog_tag writer option.");
            exit_on_error = 1; /* Don't exit. Let errors stack up. */
         }
         else
         {
            if ( NULL == (wmalrm->actlog_tag = mkstring(ap->value)) )
               return(NULL); /* Just bail */
         }
      }

      if ( ap->option == StrStr(ap->option, "survive_turn") )
      {
         bad_option = 0; /* This is a valid option */
         bad_value = 1;  /* Now assume value is bad */

         if ( ap->value == StrStr(ap->value, "true") )
         {
            bad_value = 0; /* stdout is recognized */
            wmalrm->survive_turn = 1;
         }

         if ( ap->value == StrStr(ap->value, "false") )
         {
            bad_value = 0; /* stdout is recognized */
            wmalrm->survive_turn = 0;
         }

         if ( bad_value )
         {
            error_msg("ERROR: Unable to parse \"%s\" argument to survive_turn writer option.",
                    ap->value);
            exit_on_error = 1; /* Don't exit. Let errors stack up. */
         }
      }

      if ( ap->option == StrStr(ap->option, "include_timestamp") )
      {
         bad_option = 0; /* This is a valid option */
         bad_value = 1;  /* Now assume value is bad */

         if ( ap->value == StrStr(ap->value, "true") )
         {
            bad_value = 0; /* stdout is recognized */
            wmalrm->include_timestamp = 1;
         }

         if ( ap->value == StrStr(ap->value, "false") )
         {
            bad_value = 0; /* stdout is recognized */
            wmalrm->include_timestamp = 0;
         }

         if ( bad_value )
         {
            error_msg("ERROR: Unable to parse \"%s\" argument to include_timestamp writer option.",
                    ap->value);
            exit_on_error = 1; /* Don't exit. Let errors stack up. */
         }
      }

      if ( ap->option == StrStr(ap->option, "file_name") )
      {
         bad_option = 0; /* This is a valid option */
         bad_value = 1;  /* Now assume value is bad */

         if ( NULL != ap->value )
         {
            if ( 0 != ap->value[0] )
            {
               if ( NULL == (wmalrm->file_name = mkstring(ap->value)) )
                  return(NULL); /* Just bail on malloc() failures - mkstring handles the message */
         
               bad_value = 0;
            }
         }

         if ( bad_value )
         {
            error_msg("ERROR: Unable to parse argument to file_name writer option.");
            exit_on_error = 1; /* Don't exit. Let errors stack up. */
         }
      }

      if ( bad_option )
      {
         error_msg("ERROR: Alarm writer option \"%s\" not understood.",
                 ap->option);
         exit_on_error = 1; /* Don't exit. Let errors stack up. */
      }

      ap = ap->next;
   }

   /* STUB - Consider removing this - let second take precedence */
   if ( exit_on_error )
      return(NULL); /* No free(), just exit().
                       Death awaits us for this transgression. */


   /* Make sure our tag is set. */
   if ( NULL == wmalrm->actlog_tag )
   {
      if ( NULL == (wmalrm->actlog_tag = mkstring(DEFAULT_ACTLOG_TAG)) )
         return(NULL); /* Just bail */
   }

   /* STUB - Validate we are not using some kind of "reserved" tag */

   switch ( wmalrm->alarm_to )
   {
   case ALRM_TO_NOWHERE:
      error_msg("ERROR: Required alarm writer \"alarm_to\" option not specified.");
      exit_on_error = 1;
      break;
   case ALRM_TO_STDOUT:
      /* Insure we are not a daemon */
      if ( cfg->run_method == RM_DAEMON )
      {
         error_msg("ERROR: The alarm writer cannot \"alarm_to\" stdout when in daemon mode.");
         exit_on_error = 1;
      }
      break;
   case ALRM_TO_ACTLOG:
      if ( ALOpenPipe(wmalrm->actlog_tag) )
      {
         if ( wmalrm->survive_turn )
         {
            error_msg("NOTE: Failed on openpipe command in alarm writer module.");
         }
         else
         {
            error_msg("ERROR: Failed on openpipe command in alarm writer module.");
            return(NULL); /* Don't wait, just exit */
         }
      }
      break;
   case ALRM_TO_FILE:
      if ( NULL == wmalrm->file_name )
      {
         error_msg("ERROR: alarm_to file was specified without a file_name argument.");
         exit_on_error = 1;
      }
      else
      {
         /* This is unlike me - but these are only used here */
         FILE *tf;
         time_t now;

         /* The "w" will create and/or truncate! */
         if ( NULL == (tf = fopen(wmalrm->file_name, "w")) )
         {
            error_msg("ERROR: Unable to open \"%s\" for writing.", wmalrm->file_name);
            exit_on_error = 1;
         }
         else
         {
            time(&now);
            
            fprintf(tf, "Alarm file opened for writing on %s", ctime(&now));
            fclose(tf);
         }
      }
      break;
   case ALRM_TO_SYSLOG:
   default:
      error_msg("ERROR (Internal): Unsupported \"alarm_to\" destination.");
      exit_on_error = 1;
      break;
   }

   if ( exit_on_error )
      return(NULL); /* No free(), just exit().
                       Death awaits us for this transgression. */

  /* Validate the alarm items */
   this_pi = p->pi_olist;
   while ( this_pi )
   {
      /* Make sure that there is a copare set */
      if ( this_pi->alarm_type == ALARM_NO )
      {
         error_msg("ERROR: The quad for \"%s\" does not have an alarm_at clause.", this_pi->name);
         exit_on_error = 1;
      }

      /* Check the data type compatibility */
      switch ( this_pi->data_type )
      {
      case PDT_INT8:
      case PDT_UINT8:
      case PDT_INT16:
      case PDT_UINT16:
      case PDT_INT32:
      case PDT_UINT32:
      case PDT_INT64:
      case PDT_UINT64:
         break;
      case PDT_UNKNOWN:
      case PDT_TIMEVAL:
      case PDT_TIMESPEC:
      case PDT_STRING:
      default:
         error_msg("ERROR: The quad for \"%s\" has an unsupported alarm_at data type.", this_pi->name);
         exit_on_error = 1;
         break;
      }

      /* Check the quad iargs for incompatibilities */
      if ( MUNGE_FACTOR == (this_pi->munge_flag & MUNGE_FACTOR) )
      {
         error_msg("ERROR: The quad for \"%s\" cannot be factored and alarmed.", this_pi->name);
         exit_on_error = 1;
      }

      this_pi = this_pi->next_opi;
   }

   /* Check for errors from the previous set of checks */
   if ( exit_on_error )
      return(NULL); /* Same issues on return without free() */

   return(rv);

}


/* ========================================================================= */
int64_t promote_value(void *dataptr, int sign_flag, int datatype)
{
   int64_t   value;

   /* Promote the result data to int64_t */
   switch ( datatype )
   {
   case PDT_INT8:
      value = *((int8_t *)dataptr);
      break;
   case PDT_UINT8:
      value = *((uint8_t *)dataptr);
      break;
   case PDT_INT16:
      value = *((int16_t *)dataptr);
      break;
   case PDT_UINT16:
      value = *((uint16_t *)dataptr);
      break;
   case PDT_INT32:
      value = *((int32_t *)dataptr);
      break;
   case PDT_UINT32:
      value = *((uint32_t *)dataptr);
      break;
   case PDT_INT64:
      value = *((int64_t *)dataptr);
      break;
   case PDT_UINT64:
      value = *((uint64_t *)dataptr);
      break;
   default:
      return(0);  /* Unsupported data type */
      break;
   }

   /* Apply the sign - if it is set */
   if ( sign_flag )
      value *= -1;

   return(value);
}

/* ========================================================================= */
int should_alarm(void *vplhs, int sign_flag, int64_t rhs, int compare, int datatype)
{
   int64_t   lhs;

   lhs = 0; /* Just so the compiler can be happy */

   switch ( datatype )
   {
   case PDT_INT8:
   case PDT_UINT8:
   case PDT_INT16:
   case PDT_UINT16:
   case PDT_INT32:
   case PDT_UINT32:
   case PDT_INT64:
   case PDT_UINT64:
      lhs = promote_value(vplhs, sign_flag, datatype);
      break;
   default:
      return(-1);  /* Unsupported data type */
      break;
   }

   /* Do the actual compare */
   switch ( compare )
   {
   case ALARM_EQ:
      if ( lhs == rhs )
         return(1);
      break;
   case ALARM_NE:
      if ( lhs != rhs )
         return(1);
      break;
   case ALARM_GE:
      if ( lhs >= rhs )
         return(1);
      break;
   case ALARM_LE:
      if ( lhs <= rhs )
         return(1);
      break;
   case ALARM_LT:
      if ( lhs < rhs )
         return(1);
      break;
   case ALARM_GT:
      if ( lhs > rhs )
         return(1);
      break;
   case ALARM_NO:
   default:
      return(-1);
      break;
   }
   
   return(0);
}

/* ========================================================================= */
int generate_alarm(struct pitem *this_pi, struct alrm_data *wmalrm)
{
   FILE *f;
   time_t now;
   char *timestr;

   switch ( wmalrm->alarm_to )
   {
   /* These are just unsupported at this time */
   case ALRM_TO_SYSLOG:
   /* Fall through to something that is supported */
   case ALRM_TO_STDOUT: 
      if ( wmalrm->include_timestamp )
      {
         time(&now);
         timestr = ctime(&now);
         chomp(timestr);

         printf("%s: Value of %lld for %s raised alarm condition (alarm_at: %lld).\n",
                timestr,
                (long long)promote_value(this_pi->data_ptr, this_pi->sign_flag, this_pi->data_type),
                this_pi->header,
                (long long)this_pi->alarm_value);
      }
      else
      {
         printf("Value of %lld for %s raised alarm condition (alarm_at: %lld).\n",
                (long long)promote_value(this_pi->data_ptr, this_pi->sign_flag, this_pi->data_type),
                this_pi->header,
                (long long)this_pi->alarm_value);
 
      }
      fflush(stdout);
      break;
   case ALRM_TO_ACTLOG:
      ALPutLine(wmalrm->actlog_tag,
                "Value of %lld for %s raised alarm condition (alarm_at: %lld).\n",
                (long long)promote_value(this_pi->data_ptr, this_pi->sign_flag, this_pi->data_type),
                this_pi->header,
                (long long)this_pi->alarm_value);
      break;
   case ALRM_TO_FILE:
      /* Open the file EACH TIME. Do not try to keep it open. This aids in rolling
         the file... and... this is not likely to be written to constantly. */

      /* The "a" will append to the end of the file */
      if ( NULL != (f = fopen(wmalrm->file_name, "a")) )
      {
         if ( wmalrm->include_timestamp )
         {
            time(&now);
            timestr = ctime(&now);
            chomp(timestr);

            fprintf(f, "%s: Value of %lld for %s raised alarm condition (alarm_at: %lld).\n",
                    timestr,
                    (long long)promote_value(this_pi->data_ptr, this_pi->sign_flag, this_pi->data_type),
                    this_pi->header,
                    (long long)this_pi->alarm_value);
         }
         else
         {
            fprintf(f, "Value of %lld for %s raised alarm condition (alarm_at: %lld).\n",
                    (long long)promote_value(this_pi->data_ptr, this_pi->sign_flag, this_pi->data_type),
                    this_pi->header,
                    (long long)this_pi->alarm_value);  
         }
         
         fclose(f);
      }
      else
      {
         error_msg("ERROR: Unable to write alarm event to file destination.");
      }
      break;
   case ALRM_TO_NOWHERE:
   default:
      /* This has already been tested for - declared unreachable */
      break;
   }

   return(0);
}

/* ========================================================================= */
int WriteAlarmWM(void *data_handle)
{
   struct alrm_data *wmalrm;
   struct pitem *this_pi;

   assert(NULL != data_handle);

   /* Get a convenient reference */
   wmalrm = (struct alrm_data *)data_handle;

   this_pi = wmalrm->p->pi_olist;
   while ( this_pi )
   {
      if ( 1 == should_alarm(this_pi->data_ptr, this_pi->sign_flag, this_pi->alarm_value, this_pi->alarm_type, this_pi->data_type) )
      {
         generate_alarm(this_pi, wmalrm);
      }

      this_pi = this_pi->next_opi;
   }

   return(0);
}

/* ========================================================================= */
int FinishAlarmWM(void *data_handle)
{
   struct alrm_data *wmalrm;

   assert(NULL != data_handle);

   /* Get a convenient reference */
   wmalrm = (struct alrm_data *)data_handle;

   switch ( wmalrm->alarm_to )
   {
   case ALRM_TO_ACTLOG:
      ALClosePipe(wmalrm->actlog_tag);
      break;
   case ALRM_TO_FILE:
      /* There is NOTHING to do here. The file is open()ed, written, and
         close()d on each write. */
   default:
      break;
   }

   return(0);
}

