#include <stdio.h>
#include <stdarg.h>
#include "options.h"
#include "version.h"

/* ========================================================================= */
void DisplayUsage(void)
{
  printf("%s - Alternative CPU display\n", APP_NAME);
  printf("   Usage:\n");
  printf("     -c X    Use X columns\n");
  printf("     -s X    Use X SMT columns\n");
  printf("     -h      Show this help\n");
  printf("     -v      Show version info\n");
  printf("     -m      Display output in monochrome\n");
  printf("     -x      Display extended data\n");
  printf("                sysc - syscalls\n");
  printf("                sysr - read syscalls\n");
  printf("                sysw - write syscalls\n");
  printf("                min  - minor page faults\n");
  printf("                maj  - major page faults\n");
  printf("                runq - run queue\n");
  printf("                cxsw - context switches\n");



}

/* ========================================================================= */
void DisplayVersion(void)
{
  printf("%s - Alternative CPU display\n", APP_NAME);
  printf("   Version: %s\n\n", VERSION_STRING);
  printf("   William Hacket <whacket@clerkenwell.bridewell.co.uk>\n");
  printf("   William Favorite <wfavorite2@bloomberg.net>\n");
}

#define ANSI_RED      31
#define ANSI_GREEN    32
#define ANSI_YELLOW   33
#define ANSI_BLUE     34
#define ANSI_MAGENTA  35
#define ANSI_CYAN     36
#define ANSI_WHITE    37
#define ANSI_NORMAL   0

void SetColor(int c)
{
  printf("%c[%dm", 27, c);
}

void cprintf(int c, const char *format, ...)
{
  va_list ap;
  
  printf("%c[%dm", 27, c);
  va_start(ap, format);
  vprintf(format, ap);
  va_end(ap);
  printf("%c[0m", 27);
}

int main(int argc, char *argv[])
{
  struct options *o;

  if(NULL == (o = ParseOptions(argc, argv)))
  {
    printf("ERROR: ParseOptions returns NULL.\n");
    return(1);
  }

  if(o->show_help)
    DisplayUsage();

  if(o->show_version)
    DisplayVersion();

  printf("Start ");
  SetColor(ANSI_RED);
  printf("red ");
  SetColor(ANSI_GREEN);
  printf("green ");
  SetColor(ANSI_NORMAL);
  printf("End.\n");

  /*
ANSI_RED
ANSI_GREEN
ANSI_YELLOW
ANSI_BLUE
ANSI_MAGENTA
ANSI_CYAN
ANSI_WHITE
  */
  printf("Start==>");
  cprintf(ANSI_RED, "#");
  cprintf(ANSI_GREEN, "#");
  cprintf(ANSI_YELLOW, "#");
  cprintf(ANSI_BLUE, "#");
  cprintf(ANSI_MAGENTA, "#");
  cprintf(ANSI_CYAN, "#");
  cprintf(ANSI_WHITE, "#");

  printf("<==End.\n");


  return(0);
}
