#ifndef OPTIONS_H
#define OPTIONS_H

#define O_COLOR_NONE   0
#define O_COLOR_SIMPLE 1
#define O_COLOR_FRUITY 2
#define O_COLOR_MANY   3

#define O_DEF_COLOR    O_COLOR_MANY
#define O_DEF_COLUMNS  1
#define O_DEF_SMTCOL   1
#define O_DEF_ITIME    2


/* Why set this as a define? Because we may want to have more total options */
#define O_TOTALS_ALL   1
#define O_TOTALS_NONE  0

/*** Bold Linked List - All the CPUs that should be in bold ***/
#define BLT_CPU  0        /* Item is a CPU */
#define BLT_BAND 1        /* Item is a "band" of CPUs */



#define O_COLS_BASIC    0
#define O_COLS_EXTENDED 1
#define O_COLS_ALL      2


struct boldll
{
   int type;
   int cpu;
   struct boldll *next;
};

/*** The holder of all user supplied (or default) options ***/
struct options
{
   int color_output;
   int column_output;         /* Linux-only option for what data to display  */
   int show_help;
   int show_version;
   int columns;
   /* int boldopts;          / * Bold by name or by bands                    */
   char *blist;               /* List (string) of cpus to bold               */
   struct boldll *bl;         /* Linked list of cpus (or bands) to bold      */
   int bold_on;              /* Used to set all CPUs displayed as BOLD       */
   int show_timestamp;
   int show_totals;
   int smt_columns;
   int smt_user_set;          /* Boolean - Did user set SMT value or not     */
   int iteration_time;
   int it_set;
   int debug;
   int input_error;
};


struct options *ParseOptions(int argc, char *argv[]);

#endif
