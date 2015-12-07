#ifndef PROCESS_H
#define PROCESS_H

#include <time.h>
#include "../defaults.h" /* For MAX_PNAME_LEN */

struct procitem
{
   /* Public - consistent across implementations */
   long pid;        /* It needs to big enough to hold PIDs in each OS, but
                       also big enough to be signed so we can ask for -1. */
   char pname[MAX_PNAME_LEN];
   unsigned long long K_size;
   unsigned long long K_rss;
   time_t stime;

   /* Private - Specifc to this implementation */
   pid_t index;   /* Used by getproce64() to determine next pid */
};

/* =========================================================================
 * Name: InitProcessEntry
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: Called once at app startup.
 */
struct procitem *InitProcessEntry(struct procitem *p);

/* =========================================================================
 * Name: StartProcessEntry
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: Called once for each process list retrieval
 */
int StartProcessEntry(struct procitem *p);

/* =========================================================================
 * Name: GetProcessEntry
 * Description: 
 * Paramaters: 
 * Returns: 1 on successful read (please try again for another). 0 on end
 *          of the directory listing (don't come back for more).
 * Side Effects: 
 * Notes: Called repeatedly until it returns 0 (no process returned)
 */
int GetProcessEntry(struct procitem *p);

#endif
