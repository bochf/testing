#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

#include "../mainhelp.h"
#include "meminfoprov.h"
#include "linproviders.h"

///////////////////////////////////////////////////////////////////////////////#
/* 
   Discussion on this provider
   - The inclination is to make the provider entirely dynaminc. This is
     because the file that it pulls from is one of the best formatted files
     in proc. The data has names with mixed case (and the convention is to
     use all lower case) but this is a mere string conversion away. In fact
     the list function dynamically generates this list based on the raw
     content from the file.
   - There are two problems with the "full dynamic" retrieval method.
     1) It is expensive to walk a list and do compares to conditionally pull
        data from the proc file. You have lots of string comparing going on
        and it is somewhat simpler to just parse it into a well formed 
        struct and then conditionally manage the data from there.*
        [* Interestingly... It might be cheaper to have a struct that was
        simply a list of pointers to the *raw* data as read from the file.
        If the data was chosen, it could THEN be parsed from string form
        to a native data type. This is still somewhat academic - see point
        #2. ]
     2) Not all proc files are created equal. Many have structured but un-
        labeled data. This means that the provider needs to be aware of
        the data and must parse the data appropriately. The provider must
        also be aware when the format of the proc file changes.
   - The choice here is to create a reference implementation that is likely
     to work with other data sources in Linux (specifically /proc).
   - This list can be pulled from a system using:
     cat /proc/meminfo | sed -e "s/:.STAR//"
     Where STAR is really a * (splat).
   - The data is *in* the buffer struct. This is a poor choice. The buffer
     fill can be farmed out to a common function to be used by all proc-
     based providers. 
*/


/* This is the struct that holds the data. There are three of them.
   Two are for this and last data (required for diff capability) and the
   third is for the data that will be displayed. The data here is a 32-bit
   signed value. This was chosen as all data here is a measure of KB
   of memory. So we should never overflow a 32 bit signed value. This
   means that we have 2^41 bytes (2TB) of range. The point of using a
   signed instead of an unsigned value is that diffs need to work...
   specifically they need to go negative.
*/
struct minfo_data
{
   int32_t memtotal;
   int32_t memfree;
   int32_t buffers;
   int32_t cached;
   int32_t swapcached;
   int32_t active;
   int32_t inactive;
   int32_t hightotal;
   int32_t highfree;
   int32_t lowtotal;
   int32_t lowfree;
   int32_t swaptotal;
   int32_t swapfree;
   int32_t dirty;
   int32_t writeback;
   int32_t anonpages;
   int32_t mapped;
   int32_t slab;
   int32_t pagetables;
   int32_t nfs_unstable;
   int32_t bounce;
   int32_t commitlimit;
   int32_t committed_as;
   int32_t vmalloctotal;
   int32_t vmallocused;
   int32_t vmallocuhunk;
   int32_t hugepages_total;
   int32_t hugepages_free;
   int32_t hugepages_rsvd;
   int32_t hugepagesize;
};

/* How much memory to reserve for the read buffer. This is a raw buffer
   that we will read the /proc file into in a single read() call. It is
   grossly oversized (yet still fairly small) so that there shall never
   be a read overflow.
*/
#define BUFFER_ALLOCATION 2048

/* This is a linked list item used to retrieve and hold items from the
   file that will then be used to generate the data provider (item) list.
   See notes above about dynamic data change detection... but here is
   a recap in a nutshell:
   - Dynamically pulling data (just for the list) is contrary to the 
     actual data that the provider supports. They should BOTH use the
     same list. (But they don't because managing this list each iteration
     makes the implementation slightly heavier than I would like.
   - This leaks lots of mem - but it is only used for the list function,
     which means that the app will exit shortly.
   - This should be removed and replaced with a static list.
*/
struct mi_items
{
   char name[32];

   struct mi_items *next;
};

/* This is the core set of data for the provider. It is static to this
   file. (Much like it would be local to a class.) The buffer struct 
   sits inside of another struct because the buffer struct is from a
   "library"/common function.

   All members of lastpd and thispd are parsed/filled. The items in
   usepd (what goes to the writer) are contidionally filled. */
struct memi_data
{
   struct buffer *rbuf;
   
   struct minfo_data *lastpd;    /* Last */
   struct minfo_data *thispd;    /* This (latest) */
   /*
   struct minfo_data *usedpd;    / * Copy of latest or diff (use this one) */

};


/* Forward declarations */
int init_midata(void);
int32_t retrieve_number(char *line);
int meminfo_update(int interval);
int meminfo_list(int dflag);
int meminfo_to_dataname(char *line);
struct mi_items *build_mi_list(void);
struct pitem *meminfo_enablepitem(struct qparts *qp);
int parse_proc_file(void);


/* These items are "local" to this "class" */
static struct provider *this_provider;
static struct memi_data *this_data;

/* ========================================================================= */
int MemInfoRegister(struct proot *p)
{
   /* The conditionall malloc (init_midata) will key off this value later */
   this_data = NULL;

   /* Best pracitces would dictate that the provider assert() data type
      sizes being equal. That is NOT necessary here because the struct 
      this provider pulls from is of our own design. There are no ambiguous 
      data types used here - this provider pulls from a text string and
      assigns to its own type. */

   this_provider = RegisterProvider(p,               /* The provider "root" structure */
                                    "proc.meminfo",  /* The name of the provider Q1   */
                                    meminfo_update,
                                    meminfo_list,
                                    meminfo_enablepitem);

   /* Check status */
   if ( NULL == this_provider )
      return(1);

   return(0);

}

/* ========================================================================= */
int meminfo_update(int interval)
{
   struct pitem *this_pitem;
   int32_t *lastdptr;
   int32_t *thisdptr;

   if ( FillReadBuff(this_data->rbuf) )
      return(1);

   /* Move unstructured data to the parsed data struct */
   parse_proc_file();

   /* Walk the update item list - update items */
   this_pitem = this_provider->ui_list;
   while ( this_pitem )
   {
      thisdptr = (int32_t *)((unsigned long)this_data->thispd + (unsigned long)this_pitem->sioffset);
      lastdptr = (int32_t *)((unsigned long)this_data->lastpd + (unsigned long)this_pitem->sioffset);
      if(CalcData(this_pitem, lastdptr, thisdptr, interval))
         return(1);

      this_pitem = this_pitem->next_ui;
   }

   return(0);
}



/* ========================================================================= */
struct mi_items *build_mi_list(void)
{
   char line[80]; /* Far more than required */
   FILE *fh;
   struct mi_items *mitop;
   struct mi_items *mithis;

   if ( NULL == ( fh = fopen("/proc/meminfo", "r")) )
      return(NULL);

   mitop = NULL;
   while ( fgets(line, 80, fh) )
   {
      if ( meminfo_to_dataname(line) )
      {
         /* Malloc space */
         if ( NULL == ( mithis = (struct mi_items *)malloc(sizeof(struct mi_items))) )
         {
            fclose(fh); 
            /* Close the file handle - but leave the memory - the app will
               be exiting soon on this error */
            return(NULL);
         }
            
         /* Copy in the data */
         strcpy(mithis->name, line);

         /* Smiple insert */
         mithis->next = mitop;
         mitop = mithis;
      }

   }
   
   fclose(fh);

   return(mitop);
}

/* ========================================================================= */
/* This leaks memory - but we don't care - no point in freeing as the app
   will be exiting soon. */
int meminfo_list(int dflag)
{
   struct mi_items *milist;
   int icount = 0;

   milist = build_mi_list();

   while ( milist )
   {
      DumpQuadData(dflag, PDT_INT32);
      printf("proc.meminfo::%s\n", milist->name);

      icount++;

      milist = milist->next;
   }

   return(icount);
}


#define COMP_SET_NAME(INAME); if ( 0 == strcmp(qp->iname, #INAME ) ) \
                              { data_ptr = malloc(sizeof(int32_t));     \
                                sioffset = (unsigned long)&this_data->thispd->INAME - (unsigned long)this_data->thispd; \
                                goto foundmatch; }

/* ========================================================================= */
struct pitem *meminfo_enablepitem(struct qparts *qp)
{
   struct pitem *this_pi;
   int32_t *data_ptr = NULL;
   unsigned long sioffset;

   /* Make sure we were called with the right quad (provider) name */
   if ( 0 != strcmp(qp->pname, "proc.meminfo") )
      return(NULL);

   /* Flag update for the provider */
   this_provider->update_required = 1;

   /* Now that this provider is active, allocate memory */
   if ( NULL == this_data )
   {
      if(init_midata())
      {
         error_msg("Failed to allocate memory for meminfo provider.");
         return(NULL);
      }
   }

   /* Parse iarg (psavg) before trying to go further. */
   if ( ShouldPSAvg(qp) )
   {
      error_msg("ERROR: \"psavg\" arguments are not supported in the proc.meminfo items.");
      return(NULL);
   }

   sioffset = 0xDEADBEEF;

   COMP_SET_NAME(memtotal);
   COMP_SET_NAME(memfree);
   COMP_SET_NAME(buffers);
   COMP_SET_NAME(cached);
   COMP_SET_NAME(swapcached);
   COMP_SET_NAME(active);
   COMP_SET_NAME(inactive);
   COMP_SET_NAME(hightotal);
   COMP_SET_NAME(highfree);
   COMP_SET_NAME(lowtotal);
   COMP_SET_NAME(lowfree);
   COMP_SET_NAME(swaptotal);
   COMP_SET_NAME(swapfree);
   COMP_SET_NAME(dirty);
   COMP_SET_NAME(writeback);
   COMP_SET_NAME(anonpages);
   COMP_SET_NAME(mapped);
   COMP_SET_NAME(slab);
   COMP_SET_NAME(pagetables);
   COMP_SET_NAME(nfs_unstable);
   COMP_SET_NAME(bounce);
   COMP_SET_NAME(commitlimit);
   COMP_SET_NAME(committed_as);
   COMP_SET_NAME(vmalloctotal);
   COMP_SET_NAME(vmallocused);
   COMP_SET_NAME(vmallocuhunk);
   COMP_SET_NAME(hugepages_total);
   COMP_SET_NAME(hugepages_free);
   COMP_SET_NAME(hugepages_rsvd);
   COMP_SET_NAME(hugepagesize);

   /* WTF?!?! - YES I DID IT!
      I had to reference the K&R book to insure I was doing it right
      but I did it. The point here is we do not want to do pattern
      matches for *every* string - even after a match has been found.
      So, there are several options here... either some kind of jump
      (to another function) so we can retrun, a boolean for having
      found the match, or phuck it... just jump to the end of the
      pattern match section. And that is what I did. Look for the
      goto in the COMP_SET_NAME macro above. */
 foundmatch:

   if ( sioffset == 0xDEADBEEF )
      return(NULL);

   /* Check the malloc() - not really necessary because NewPItem() will check it */
   if ( data_ptr == NULL )
      return(NULL);

   if ( NULL == (this_pi = NewPItem(qp, PDT_INT32, data_ptr)) )
      return(NULL);

   
   this_pi->sioffset = sioffset;

   /* Insert into ui list */
   if(InsertUpdateItem(this_provider, this_pi))
      return(NULL);

   return(this_pi);
}

/* ========================================================================= */
int32_t retrieve_number(char *line)
{
   int32_t value = 0;

   while ( *line != ':' )
   {
      if ( *line == 0 )
         return(0);

      line++;
   }

   /* On the ":" */
   line++; 
   /* On the character after the ":" */

   while ( *line == ' ' )
   {
      if ( *line == 0 )
         return(0);

      line++;
   }

   while (( *line >= '0' ) && ( *line <= '9' ))
   {
      value *= 10;
      value += *line - '0';

      line++;
   }

   return(value);
}

/* ========================================================================= */
int parse_proc_file(void)
{
   /* this_data is static to this file */
   char line[32];
   int unknown_data = 0;
   struct minfo_data *temppd;

   /* Rotate data */
   temppd = this_data->lastpd;
   this_data->lastpd = this_data->thispd;
   this_data->thispd = temppd;


   /* cat meminfo | cut -c1 | sort | uniq -c | sort -r */

   while ( GetNextLine(line, this_data->rbuf, 32) )
   {
      if ( line[0] == 'H' ) /* 6 hits */
      {
         if ( line[1] == 'u' ) /* 4 hits */
         {
            if ( line[10] == 'T' )
            {  temppd->hugepages_total = retrieve_number(line); continue; }

            if ( line[10] == 'F' )
            {  temppd->hugepages_free = retrieve_number(line); continue; }

            if ( line[10] == 'R' )
            {  temppd->hugepages_rsvd = retrieve_number(line); continue; }

            if ( line[10] == 'z' )
            {  temppd->hugepagesize = retrieve_number(line); continue; }

            unknown_data++;
            continue;
         }

         if ( line[4] == 'T' )
         {  temppd->hightotal = retrieve_number(line); continue; }
         
         if ( line[4] == 'F' )
         {  temppd->highfree = retrieve_number(line); continue; }

         unknown_data++;
         continue;
      }

      if ( line[0] == 'S' ) /* 4 hits */
      {
         if ( line[4] == 'C' )
         {  temppd->swapcached = retrieve_number(line); continue; }

         if ( line[7] == 'T' )
         {  temppd->swaptotal = retrieve_number(line); continue; }

         if ( line[7] == 'F' )
         {  temppd->swapfree = retrieve_number(line); continue; }

         if ( line[7] == ':' )
         {  temppd->slab = retrieve_number(line); continue; }

         unknown_data++;
         continue;
      }

      if ( line[0] == 'V' ) /* 3 hits */
      {
         if ( line[7] == 'T' )
         {  temppd->vmalloctotal = retrieve_number(line); continue; }

         if ( line[7] == 'U' )
         {  temppd->vmallocused = retrieve_number(line); continue; }

         if ( line[7] == 'C' )
         {  temppd->vmallocuhunk = retrieve_number(line); continue; }

         unknown_data++;
         continue;
      }

      if ( line[0] == 'M' ) /* 3 hits */
      {
         if ( line[3] == 'T' )
         {  temppd->memtotal = retrieve_number(line); continue; }

         if ( line[3] == 'F' )
         {  temppd->memfree = retrieve_number(line); continue; }

         if ( line[1] == 'a' )
         {  temppd->mapped = retrieve_number(line); continue; }

         unknown_data++;
         continue;
      }

      if ( line[0] == 'C' ) /* 3 hits */
      {
         if ( line[1] == 'a' )
         {  temppd->cached = retrieve_number(line); continue; }

         if ( line[6] == 'L' )
         {  temppd->commitlimit = retrieve_number(line); continue; }

         if ( line[6] == 't' )
         {  temppd->committed_as = retrieve_number(line); continue; }

         unknown_data++;
         continue;
      }

      if ( line[0] == 'L' ) /* 2 hits */
      {
         if ( line[3] == 'T' )
         {  temppd->lowtotal = retrieve_number(line); continue; }

         if ( line[3] == 'F' )
         {  temppd->lowfree = retrieve_number(line); continue; }
         
         unknown_data++;
         continue;
      }


      if ( line[0] == 'B' ) /* 2 hits */
      {
         if ( line[1] == 'o' )
         {  temppd->bounce = retrieve_number(line); continue; }
         
         if ( line[1] == 'u' )
         {  temppd->buffers = retrieve_number(line); continue; }

         unknown_data++;
         continue;
      }


      if ( line[0] == 'A' ) /* 2 hits */
      {
         if ( line[1] == 'c' )
         {  temppd->active = retrieve_number(line); continue; }

         if ( line[1] == 'u' )
         {  temppd->anonpages = retrieve_number(line); continue; }

         unknown_data++;
         continue;
      }

      if ( line[0] == 'W' ) /* 1 hit */
      {  temppd->writeback = retrieve_number(line); continue; }

      if ( line[0] == 'P' ) /* 1 hit */
      {  temppd->pagetables = retrieve_number(line); continue; }

      if ( line[0] == 'N' ) /* 1 hit */
      {  temppd->nfs_unstable = retrieve_number(line); continue; }

      if ( line[0] == 'I' ) /* 1 hit */
      {  temppd->inactive = retrieve_number(line); continue; }

      if ( line[0] == 'D' ) /* 1 hit */
      {  temppd->dirty = retrieve_number(line); continue; }

      unknown_data++;
   }

   return(unknown_data);
}

/* ========================================================================= */
int init_midata(void)
{
   if ( NULL == (this_data = (struct memi_data *)malloc(sizeof(struct memi_data))) )
   {
      error_msg("Unable to allocate memory for meminfo core data.");
      return(1);
   }

   if ( NULL == (this_data->rbuf = InitReadBuff("/proc/meminfo", 20)) )
   {
      error_msg("Unable to allocate memory for read buffer.");
      return(1);
   }

   /* DEPRECATED - Now uses the above API / method 
   if ( NULL == (this_data->rbuf = InitProcBuff(BUFFER_ALLOCATION)) )
   {
      error_msg("Unable to allocate memory for read buffer.");
      return(1);
   }
   */

   if ( NULL == (this_data->lastpd = (struct minfo_data *)malloc(sizeof(struct minfo_data))) )
   {
      error_msg("ERROR: Unable to allocate memory for provider data structure.");
      return(1);
   }

   if ( NULL == (this_data->thispd = (struct minfo_data *)malloc(sizeof(struct minfo_data))) )
   {
      error_msg("ERROR: Unable to allocate memory for provider data structure.");
      return(1);
   }

   return(0);
}

/* ========================================================================= */
int meminfo_to_dataname(char *line)
{
   int i;

   i = 0;

   while ( line[i] != 0 )
   {
      if ( line[i] == ':' )
      {
         line[i] = 0;
         return(1);
      }

      /* Convert to lower case */
      if (( line[i] >= 'A' ) && ( line[i] <= 'Z' ))
         line[i] = line[i] + 32;

      i++;
   } 

   /* If we got here, then invalid line */
   return(0);
}


