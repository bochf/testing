#ifndef PROCESS_H
#define PROCESS_H

#include <time.h>
#include <dirent.h>

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
   DIR *procdir;
   struct dirent direntry;
};

/* =========================================================================
 * Name: InitProcessEntry
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
struct procitem *InitProcessEntry(struct procitem *p);

/* =========================================================================
 * Name: StartProcessEntry
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes: 
 */
int StartProcessEntry(struct procitem *p);

/* =========================================================================
 * Name: GetProcessEntry
 * Description: 
 * Paramaters: 
 * Returns: 1 on successful read (please try again for another). 0 on end
 *          of the directory listing (don't come back for more).
 * Side Effects: 
 * Notes: 
 */
int GetProcessEntry(struct procitem *p);


#endif
