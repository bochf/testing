#ifndef CONFIGFILE_H
#define CONFIGFILE_H

/* This is the maximum length a line can be */
#define MAX_CLINE_LEN 1024

/* Register the WM_*_TYPEs here.
   These are used in multiple places:
   - In the config->send_to value as an "enum"
   - In the writer data struct (as the first item (an int) to validate
     that the data is correct. These can (of course) be anything, but the
     recommendation here is to make them NOT 0 or 1. It is best that these
     be some non-naturally occouring item so bad input can be detected on
     the writer data input. (I have seeded some examples with two digit 
     prime numbers.)
*/
#define WM_STDOUT_TYPE    31
#define WM_FILE_TYPE      37
#define WM_ALARM_TYPE     41
#define WM_BCD_TYPE       43
/* Some stuff I made up to reserve a few more numbers */
#define WM_BLOG_TYPE      47  /* See BCD */
#define WM_BINFILE_TYPE   53  /* See BCD */
#define WM_SOCKET_TYPE    59
#define WM_GZFILE_TYPE    61  /* See BCD */


/* These are options for run_method */
#define RM_DAEMON     1
#define RM_FOREGROUND 0
#define RM_DEFAULT    RM_FOREGROUND

/*
  Discussion on alarms and alarming
  The primary problems with alarming are
   - Putting the data in the appropriate data type for the compare
   - Where to do the compare and where to alarm
   - Handling the multitude of edge cases (comparing strings, user enters
     a quad, but does not specify an alarm value, alarm on a negative when
     the data cant be negative (ie: memfree:diff - if this goes negative then
     you have lost memory, but memfree itself cannot possibly go negative)
  Here is the basic solution:
   - There will be an alarm writer module. It will be responsible for the 
     compare and the alarm write/signaling.
   - The config file will NOT be "negotiating" data types. It will input the
     data into a signed long long. The compare will then be promoted to a
     long long compare. No counter should ever overflow a long long.
   - Only int data types can be compared. No strings (arrays), or structs.
     If you want to compare on some sub-type of a string or struct, then
     have the provider represent it in a numeric.
   - All other details are left to the alarm writer module.
*/

#define ALARM_NO  0    /* No alarm for this quad                             */
#define ALARM_EQ  1    /* Alarm if values are equal                          */
#define ALARM_NE  2    /* Alarm if values are not equal                      */
#define ALARM_GE  3    /* Alarm if greater than or equal                     */
#define ALARM_LE  4    /* Alarm if less than or equal                        */
#define ALARM_LT  5    /* Alarm if less than                                 */
#define ALARM_GT  6    /* Alarm if greater than                              */

/* Factor values. Read providers.c::parse_factor() for discussion            */
#define FACT_FROM_NONE     0
#define FACT_FROM_BYTES    1
#define FACT_FROM_DECIMAL  2
#define FACT_FROM_KBYTES   4
#define FACT_FROM_4KPAGE   8
#define FACT_FROM_8KPAGE   16
#define FACT_FROM_64KPAGE  32

#define FACT_TO_NONE       0
#define FACT_TO_AUTOB      1  /* 1234 B; 1234 K; 1234 M;...                  */
#define FACT_TO_AUTOD      2  /* 1234E1; 1234E2; 1234E3;...                  */ 
#define FACT_TO_HEX        4  /* Only supported when from decimal            */

/* Did the user accept danger mode (in the config file)? */
#define DANGER_NACK        0
#define DANGER_ACK         1

/* ========================================================================= */
/* This represents an item as parsed from the PROVIDER_LIST_* bracket in 
   the config file. The quad is a "raw" string, and not a broken down quad.  */
struct prov_name
{
   char *quad;             /* The raw (non-broken down) string of the quad   */
   char *args;             /* The raw string of arguments after the quad     */

   /* Arguments */
   int alarm_type;         /* Uses ALARM_xx defines above                    */
   long long alarm_value;  /* Value to alarm at                              */
   char *header;           /* String to display header instead of default    */
   unsigned int fact_from;       /* Type that data before factoring          */
   unsigned int fact_to;         /* Type of representation after factoring   */

   struct prov_name *next;
};

/* ========================================================================= */
/* List of option=value pairs for the writer - in a linked list              */
struct cfg_wm_arg
{
   char *option; /* LHS */
   char *value;  /* RHS */

   struct cfg_wm_arg *next;
};

/* ========================================================================= */
struct config
{
   unsigned long run_for;      /* Time in seconds to run                     */
   int run_every;              /* Time is ms between iterations              */
   int send_to;                /* The output module                          */
   int run_method;             /* Daemon or not                              */
   int quad_count;             /* Count of items in the pnlist               */
   int danger_mode;            /* User acknowledges danger mode              */
   struct prov_name *pnlist;   /* List of PROVIDER_LIST items                */
   struct cfg_wm_arg *walist;  /* List of WRITER_ARGS items                  */
};

/* ========================================================================= */
struct config *ReadConfigFile(char *filename);

#endif
