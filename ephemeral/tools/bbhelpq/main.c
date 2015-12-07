#include <stdio.h>

#include "filefmt.h"
#include "options.h"
#include "compile.h"
#include "decompile.h"
#include "query.h"
#include "endian.h"
#include "version.h"

/* ========================================================================= */
void usage(int helplevel);

/* ========================================================================= */
int main(int argc, char *argv[])
{
   struct options *o;
   struct database *db;

   /*** Read the command line options (and ENV) ***/
   if ( NULL == ( o = ReadOptions(argc, argv) ) )
      return(1);

   /*** Handle error on input ***/
   if ( o->bError )
   {
      fprintf(stderr, "ERROR: Unable to parse input. Use bbhelpq -h for help.\n");
      return(1);
   }

   /*** Handle basic "about" requests ***/
   if ( o->bVersion )
   {
      printf("Version: %s\n", VERSION_STRING);
      printf("Endianess: %s\n", GetEndianValue() ? "big" : "little");
      return(0);
   }

   if ( o->iHelp )
   {
      usage(o->iHelp);
      return(0);
   }

   /*** Handle basic non-compile requests ***/
   if ( o->bDumpTemplate )
   {
      DumpFileTemplate();
      return(0);
   }

   /*** OK - we are here for the real work ***/
   if ( NULL == (db = LoadDatabase(o)) )
      return(1);

   HandleQueries(db, o); /* This will simply skip out if we are compiling */
   DeCompileDB(db, o);   /* This will simply skip out if we are not -o */

   CloseDatabase(db, o);

   return(0);
}


/* ========================================================================= */
void usage(int helplevel)
{
   printf("bbhelpq - A bbhelp query tool.\n");
   printf("  Usage: bbhelpq <options>\n");
   printf("     -h        Show this help (stackable: -hhh)\n");
   printf("     -c        Compile source files into database (Implies verbosity)\n");
   printf("     -d <dir>  Overide default and/or BBHELPDIR variable\n");
   printf("     -i <lvl>  Limit -l(list) results by level of impact\n");
   printf("     -k <key>  Limit -l(ist) results by keyword (csv list accepted)\n");
   printf("     -l <type> Generate a list of item types (keys|name|platform|description)\n");
   printf("     -p <OS>   Override the default platform ('uname -s'|all)\n");
   printf("     -r        Force a runtime compile (even if DB exists)\n");
   /* ntf("     -s <string> Search for \"string\"\n"); */
   printf("     -t <name> Dump all info on a named tool\n");
   printf("     -n        Generate a blank source file\n");
   printf("     -v        Dump version information\n");
   
   if (helplevel > 1)
   {
   printf("\n");
   printf("  General:\n");
   printf("  Default bbhelp directory: %s\n", DEFAULT_BBHELPDIR);
   printf("    (Override by setting $BBHELPDIR variable.)\n");
   printf("  The (well formatted) source files and optional compiled database sits in the\n");
   printf("    bbhelp directory. No additional files should be stored here.\n");
   printf("  If a compiled database file exists in the bbehelp directory, it will be used\n");
   printf("    instead of the individual source files. The source files can be force used\n");
   printf("    with the -r switch. This will cause a runtime compile of the files for each\n");
   printf("    invocation of the tool.\n");
   printf("\n");
   printf("  Queries:\n");
   printf("  The -k <key> option is a limiting factor on a -l <type> query. It will limit\n");
   printf("    the results of the list output only when listing names of commands. The\n");
   printf("    listing of platforms and keys cannot be limited by key type.\n");
   printf("  Multiple keys can be used with the -k switch. They should be separated using\n");
   printf("    a \",\". The effect of multiple keys will be an \"AND\" operation. Meaning:\n");
   printf("    both conditions (key matches) must be true for the pattern match.\n");
   printf("  The -p <platform> option is used to show results only for the supplied\n");
   printf("    platform. It will also allow you to pick the desired command when there is\n");
   printf("    more than one command entry in the database.\n");
   printf("\n");
   printf("  Compiling:\n");
   printf("  The source files can be compiled into a single file. The intent is to reduce\n");
   printf("    opens, reads, closes, and parsing to a single mmap operation.\n");
   }

   if (helplevel > 2)
   {
   printf("\n");
   printf("  Design Notes:\n");
   printf("  The intent of this tool is to simply retrieve results from the source files\n");
   printf("    to then be presented in a menuing (wrapper) script. Putting this level of\n");
   printf("    complexity between the wrapper script and the source files allows for a\n");
   printf("    rich & powerful wrapper with the simplicity of just dropping command\n");
   printf("    (source) files in the help directory to extend the scope of the results.\n");
   printf("\n");
   printf("  Author:\n");
   printf("    Karl H. Marx <worker@revolution.net>\n");
   printf("    William Favorite <wfavorite2@bloomberg.net>\n");
   }
}
