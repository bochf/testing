#ifndef OPTIONS_H
#define OPTIONS_H


struct options
{
   int bAbout;                /* Show about info and exit                    */
   int bHelp;                 /* Show help/usage info and exit               */
   int bPerUser;              /* Show per-user statistics                    */
   int bSizeGph;              /* Show size graph                             */
   int bError;                /* There was an error on input                 */
};

/* ========================================================================= */
struct options *ReadOptions(int argc, char *argv[]);


#endif
