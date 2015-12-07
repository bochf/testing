#ifndef OPTIONS_H
#define OPTIONS_H


#define ACTION_LIST  1
#define ACTION_KILL  2
#define ACTION_DFLT  1

struct options
{
   int opt_error;
   int opt_help;
   int opt_ufilter;
   int opt_action;
   int opt_rake;
   int opt_verbose;
   int kill_sig;

   char *ufilter_name;

   int opt_debug;
};

/* ========================================================================= */
struct options *ParseOptions(int argc, char *argv[]);

#endif
