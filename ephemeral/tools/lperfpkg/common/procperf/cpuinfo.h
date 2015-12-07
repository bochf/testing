#ifndef CPUINFO_H
#define CPUINFO_H



/* Use this */

/* =========================================================================
 * Name: 
 * Description: 
 * Paramaters: None
 * Returns: The SMT setting or negative number on error
 * Side Effects: Not expedient, will cause a readdir() and a fopen() for
 *         each logical CPU. So, best to call this once, and save the data.
 * Notes:
 */
#define GSMTV_ERR_NODIR -2

int GetSMTValue(void);


/* This could be just one API call that returns a single struct (with all the
   info), or it could be lots of little calls that return primitives. */








/* All Dev from here on down */

struct cpuinfo
{
   int count;
};

unsigned long GetCPUInfo(void *buf, unsigned long bufsz);

struct cpuinfo *ParseCPUInfo(struct cpuinfo *cpui, void *statbuf);







#endif
