#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <libperfstat.h>

#include "../mainhelp.h"
#include "lpdadapter.h"


/* ToDos
   [ ] Generally convert the following function to the new code (It is all that remains)
   [ ] Remove stubs
   [ ] Define a dstruct
*/

struct daidata /* Disk Adapter Instance DATA */
{
   int instance;
};


static perfstat_diskadapter_t *this_da;
static perfstat_diskadapter_t *last_da;
static perfstat_id_t *IBM_DIPSHITTERY;

/* The items we support */
static const char *diname[] = {"name",       "description", "number"   \
                               "size",       "free",        "xrate",   \
                               "xfers",      "rblks",       "wblks",   \
                               "time",       "version",     NULL};

static const int ditype[]  =  {PDT_STRING,   PDT_STRING,   PDT_INT32,   \
                               PDT_UINT64,   PDT_UINT64,   PDT_UINT64,  \
                               PDT_UINT64,   PDT_UINT64,   PDT_UINT64,  \
                               PDT_UINT64,   PDT_UINT64};

static struct provider *this_provider;

static int saved_iostat_flag;
static int adapter_count;

/* ========================================================================= */
int init_dadata(void)
{
   long count;
#ifdef ENABLE_DISK_COLLECTION
   struct vario var;
#endif

   /* This gets the count of adapter devices */
   if ( -1 == (adapter_count = perfstat_diskadapter(NULL, NULL, sizeof(perfstat_diskadapter_t), 0)) )
      return(1);
   
   /* A lame cast - likely irrelevant, but is about the malloc size */
   count = adapter_count;

   if ( NULL == (this_da = (perfstat_diskadapter_t *)malloc(count * sizeof(perfstat_diskadapter_t))) )
      return(1);

   if ( NULL == (last_da = (perfstat_diskadapter_t *)malloc(count * sizeof(perfstat_diskadapter_t))) )
      return(1);

#ifdef ENABLE_DISK_COLLECTION
   sys_parm(SYSP_GET,SYSP_V_IOSTRUN, &var);

   saved_iostat_flag = var.v.v_iostrun.value;
   
   var.v.v_iostrun.value=1; /* 1 to set & 0 to unset */
   sys_parm(SYSP_SET,SYSP_V_IOSTRUN, &var);
#endif

   if ( adapter_count != perfstat_diskadapter(IBM_DIPSHITTERY, this_da, sizeof(perfstat_diskadapter_t), adapter_count) )
   {
      error_msg("ERROR: Unable to retrieve perfstat data.");
      return(1);
   }

   return(0);
}

/* ========================================================================= */
int lpda_update(int interval)
{
   perfstat_diskadapter_t *temp_da;
   struct pitem *update_pi;
   int rv;
   int instance;

   /* rotate this and last */
   temp_da = this_da;
   this_da = last_da;
   last_da = temp_da;

   if ( adapter_count != perfstat_diskadapter(IBM_DIPSHITTERY, this_da, sizeof(perfstat_diskadapter_t), adapter_count))
   {
      error_msg("ERROR: Unable to retrieve perfstat data.");
      return(1);
   }


   rv = 0;
   update_pi = this_provider->ui_list;
   while( update_pi )
   {
      instance = ((struct daidata *)update_pi->dstruct)->instance;

      if ( update_pi->data_type == PDT_STRING )
      {
         update_pi->data_ptr = (void *)((unsigned long)&this_da[instance] + (unsigned long)update_pi->sioffset);
      }
      else
      {
         rv += CalcData(update_pi,
                        (void *)((unsigned long)&last_da[instance] + (unsigned long)update_pi->sioffset),
                        (void *)((unsigned long)&this_da[instance] + (unsigned long)update_pi->sioffset),
                        interval);
      }

      update_pi = update_pi->next_ui;
   }

   return(rv);
}

/* ========================================================================= */
int lpda_listquads(int dflag)
{
   int icount = 0;

   while ( diname[icount] )
   {
      DumpQuadData(dflag, ditype[icount]);
      printf("perfstat.diskadapter:*:%s\n", diname[icount]);

      icount++;
   }

   return(icount);
}

/* ========================================================================= */
struct pitem *lpda_itemenable(int data_size, int data_type, unsigned long sioffset, struct qparts *qp)
{
   struct pitem *this_pi;
   void *data_ptr;


   /* STUB: Not for a string */
   /* Allocate space specifically for the destination data */
   if ( NULL == (data_ptr = malloc(data_size)) )
      return(NULL);

   /* Allocate pitem */
   if ( NULL == (this_pi = NewPItem(qp, data_type, data_ptr)) )
      return(NULL);

   /* Set offset to data */
   this_pi->sioffset = sioffset;

   if ( NULL == (this_pi->dstruct = malloc(sizeof(struct daidata))) )
      return(NULL);

   /* Insert into ui list */
   this_pi->next_ui = this_provider->ui_list;
   this_provider->ui_list = this_pi;

   return(this_pi);
}

/* ========================================================================= */
#define IF_MATCH_ENABLE_UINT64(VARNAME); if ( 0 == strcmp(qp->iname, #VARNAME ) )          \
                                         { this_pitem = lpda_itemenable(sizeof(uint64_t),  \
                                                                  PDT_UINT64,              \
                                                                  (unsigned long)&this_da->VARNAME - (unsigned long)this_da, \
                                                                  qp); } 


#define IF_MATCH_ENABLE_STRING(VARNAME); if ( 0 == strcmp(qp->iname, #VARNAME ) )          \
                                        { this_pitem = lpda_itemenable(IDENTIFIER_LENGTH,  \
                                                                 PDT_STRING,               \
                                                                 (unsigned long)&this_da->VARNAME - (unsigned long)this_da, \
                                                                 qp); } 

#define IF_MATCH_ENABLE_INT32(VARNAME); if ( 0 == strcmp(qp->iname, #VARNAME ) )           \
                                        { this_pitem = lpda_itemenable(sizeof(int32_t),    \
                                                                 PDT_INT32,                \
                                                                 (unsigned long)&this_da->VARNAME - (unsigned long)this_da, \
                                                                 qp); } 


struct pitem *lpda_enablepitem(struct qparts *qp)
{
   struct pitem *this_pitem;
   struct pitem *last_pitem;
   struct pitem *return_pitem;
   perfstat_diskadapter_t *match_da;
   struct daidata *dai;
   int i;
   int instance;

   assert( NULL != qp );
  
   /* Make sure we were called with the right quad (provider) name (internal/assert issue) */
   if ( 0 != strcmp(qp->pname, "perfstat.diskadapter") )
      return(NULL);

   /* There MUST be a provider argument */
   if ( NULL == qp->pargs )
   {
      error_msg("ERROR: perfstat.diskadapter requires a provider (2nd quad) argument.");
      return(NULL);
   }

   /* See LP*Register() notes - Allocate ONLY when we have to. */
   if ( NULL == this_da )
   {
      if(init_dadata())
      {
         error_msg("ERROR: Failed to allocate memory for perfstat.diskadapter provider.");
         return(NULL);
      }
   }
   
   /* This provider is now "hot" */
   this_provider->update_required = 1;



   /* Handle the two potential cases ( * = ALL, or a specific adapter name ) */
   if (( qp->pargs[0] == '*' ) && ( qp->pargs[1] == 0 ))
   {
      match_da = this_da;

      last_pitem = NULL;
      return_pitem = NULL;
      i = 0;
      while ( i < adapter_count )
      {
         IF_MATCH_ENABLE_STRING(name);
         IF_MATCH_ENABLE_STRING(description);
         IF_MATCH_ENABLE_INT32(number);
         IF_MATCH_ENABLE_UINT64(size);
         IF_MATCH_ENABLE_UINT64(free);
         IF_MATCH_ENABLE_UINT64(xrate);
         IF_MATCH_ENABLE_UINT64(xfers);
         IF_MATCH_ENABLE_UINT64(rblks);
         IF_MATCH_ENABLE_UINT64(wblks);
         IF_MATCH_ENABLE_UINT64(time);
         IF_MATCH_ENABLE_UINT64(version);

         if ( NULL == this_pitem )
            return(NULL);

         /* What up with dai?
            I could cast, but:
            - this is more readable and clearer intent
            - additional members may be added
            - I hate those nasty casts
         */
         dai = (struct daidata *)this_pitem->dstruct;
         dai->instance = i;

         /* Build the list - and return it whole */
         if ( NULL == return_pitem )
            return_pitem = this_pitem;

         if ( last_pitem )
            last_pitem->next_opi = this_pitem;

         last_pitem = this_pitem;
         
         i++;
         match_da++;
      }
   }
   else
   {
      match_da = this_da;
      instance = -1;
      i = 0;
      while ( i < adapter_count )
      {
         if ( 0 == StrCmp(match_da[i].name, qp->pargs) )
            instance = i;
         
         i++;
      }

      /* if no match found */
      if ( -1 == instance )
         return(NULL);

      IF_MATCH_ENABLE_STRING(name);
      IF_MATCH_ENABLE_STRING(description);
      IF_MATCH_ENABLE_INT32(number);
      IF_MATCH_ENABLE_UINT64(size);
      IF_MATCH_ENABLE_UINT64(free);
      IF_MATCH_ENABLE_UINT64(xrate);
      IF_MATCH_ENABLE_UINT64(xfers);
      IF_MATCH_ENABLE_UINT64(rblks);
      IF_MATCH_ENABLE_UINT64(wblks);
      IF_MATCH_ENABLE_UINT64(time);
      IF_MATCH_ENABLE_UINT64(version);
      
      if ( NULL == this_pitem )
         return(NULL);
      else
         return_pitem = this_pitem;

      dai = (struct daidata *)this_pitem->dstruct;
      dai->instance = instance;
   }

   /* The item was not found */
   return(return_pitem);
}

/* ========================================================================= */
int LPDiskAdapterRegister(struct proot *p)
{

   /* Set this to NULL. The data for this will not be allocated until
      lpcput_emablepitem() is called. It will use this variable specifically
      to see if the operation has been done or not. The point here is to NOT
      do the allocations and the first request unless it is requred. */
   this_da = NULL;

   if (IBM_DIPSHITTERY = malloc(sizeof(perfstat_id_t)))
   {
      strcpy(IBM_DIPSHITTERY->name, FIRST_DISKADAPTER);
   }
   else
      return(1);

   /* Some "compile time" tests to insure our data types are correct */
   assert(sizeof(u_longlong_t) == sizeof(uint64_t));
   assert(sizeof(int) == sizeof(int32_t));

   /* this_provider is global/static to this module. */
   if ( NULL == (this_provider = RegisterProvider(p, "perfstat.diskadapter",
                                                  lpda_update,
                                                  lpda_listquads,
                                                  lpda_enablepitem)) )
   {
      return(1);
   }

   return(0);

}
