#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/sysinfo.h>
#include <kstat.h>

#include "kstatprov.h"
#include "../mainhelp.h"

/* Commentary about this provider
   - String support is basically non-existent. You can register string
     types, but you cant get a list of them. This basically excludes
     3000 items in the listings on systems I have seen. (NOTE: kstat()
     returns transient items. So there will be variation by system.)
   - Actually, any support for non-64bit values is non-existent
   - Some qparts (iname) are over 50 bytes in size. The supported size
     has been changed to 64 bytes in response. They could be longer as
     the module, name, and name value strings are all KSTAT_STRLEN
     in length. This is 30 (usable) characters each.
   

*/


/* This is attached to pitem->dstruct */
struct kdescriptor
{
   char ks_module[KSTAT_STRLEN]; /* provider module name */
   int  ks_instance;	/* provider module's instance */
   char ks_name[KSTAT_STRLEN]; /* kstat name */
   char nv_name[KSTAT_STRLEN]; /* name of counter */
   int data_type; /* This is the kstat data type! */
   void *last_data;
   void *this_data;


};




/* ========================================================================= */
static struct provider *myself;



/* Generally there is an ability to pass arguments to a provider for instance
   support. That will not happen here. The provider item list will be generated
   each time and the data items will be directly callable without args. */



/* ========================================================================= */
/* Forward declarations                                                      */

struct kdescriptor *build_kdescriptor(struct qparts *qp);
int update_ksitem(kstat_ctl_t *kc, struct kdescriptor *ksd);
int resolve_data_type(struct kdescriptor *ksd);

/* The three functions */
int kstat_update(int interval);
int kstat_list(int dflag);
struct pitem *kstat_enablepitem(struct qparts *qp);

/* ========================================================================= */
int KstatRegister(struct proot *p)
{
   myself = RegisterProvider(p,
                             "kstat",
                             kstat_update,
                             kstat_list,
                             kstat_enablepitem);


   if ( NULL == myself )
      return(1);

   return(0);
}

/* ========================================================================= */
/* Just for debuggery ----> #define SHOW_CURR_VALUE */
int kstat_list(int dflag)
{
   int icount = 0;
   kstat_ctl_t *kc;
   kstat_t *ksp;
   kstat_named_t *tsd_named;
   char provstr[64];
   int data_type;
   int i;
   int is_supported_type;


#ifdef SHOW_CURR_VALUE
   int64_t value;
#endif
   
   if ( NULL == (kc = kstat_open()) )
      return(0);

   /* This walks through the chain - rather than doing a kstat_lookup() */
   ksp = kc->kc_chain;
   while ( ksp )
   {
      /*** FILTER OUT UNWANTED ***/
#ifdef NO_FILTERING_DUDE
      if ( 0 == strcmp(ksp->ks_class, "rpc"))
      {
         ksp = ksp->ks_next;
         continue;
      }

      if ( 0 == strcmp(ksp->ks_module, "nxge"))
      {
         ksp = ksp->ks_next;
         continue;
      }

      if ( 0 == strcmp(ksp->ks_module, "fcip"))
      {
         ksp = ksp->ks_next;
         continue;
      }

      /* These are a fucking disaster! Some of the quads:
         UINT64 kstat::[ipf][0][outbound][dropped:pps ceiling]      <----- Colon
         UINT64 kstat::[ipf][0][outbound][ip upd. fail]             <----- period, space
         UINT64 kstat::[ipf][0][inbound][src != route]              <----- equality
         UINT64 kstat::[ipf][0][inbound][new frag state compl. pkt] <----- Really?!

         I am 99% sure the "dropped:pps celing" will not parse with the colon in.
      */
      if ( 0 == strcmp(ksp->ks_module, "ipf")) /* IP Filter */
      {
         ksp = ksp->ks_next;
         continue;
      }
#endif

      if (( ksp->ks_name[0] == 'v' ) && ( ksp->ks_name[2] == 'p' ) && (( ksp->ks_name[9] >= '0' ) && ( ksp->ks_name[9] <= '9' )))
      {
         //printf("FILTERED(%s)\n", ksp->ks_name);
         ksp = ksp->ks_next;
         continue;
      }

      if ( 0 == strcmp(ksp->ks_name, "mntinfo"))
      {
         //printf("FILTERED(%s)\n", ksp->ks_name);
         ksp = ksp->ks_next;
         continue;
      }

      /*** name value pairs ***/
      if (ksp->ks_type == KSTAT_TYPE_NAMED) 
      {
         if ( -1 == kstat_read(kc, ksp, NULL))   /* <------------------------------------ kstat_read() */
         {
            //perror("kstat_read");
            fprintf(stderr,"ERROR: Failure reading kstats.\n");
            return(1);
         }

         tsd_named = KSTAT_NAMED_PTR(ksp);

         i = 0;
         while ( i < ksp->ks_ndata )
         {
            tsd_named[i].name[KSTAT_STRLEN-1] = '\0';

            sprintf(provstr, "[%s][%d][%s][%s]",
                   ksp->ks_module,
                   ksp->ks_instance,
                   ksp->ks_name,
                   tsd_named[i].name);


            is_supported_type = 0; /* Assume we cant use it */
            switch ( tsd_named[i].data_type )
            {
            case KSTAT_DATA_STRING:
            case KSTAT_DATA_CHAR:
               data_type = PDT_STRING;
               break;
            case KSTAT_DATA_INT32:
               data_type = PDT_INT32;
               is_supported_type = 1;
               break;
            case KSTAT_DATA_UINT32:
               data_type = PDT_UINT32;
               is_supported_type = 1;
               break;
            case KSTAT_DATA_INT64:
               data_type = PDT_INT64;
               is_supported_type = 1;
#ifdef SHOW_CURR_VALUE
               value = tsd_named[i].value.i64;
#endif
               break;
            case KSTAT_DATA_UINT64:
               data_type = PDT_UINT64;
               is_supported_type = 1;
#ifdef SHOW_CURR_VALUE
               value = tsd_named[i].value.ui64;
#endif
               break;
            default:
               data_type = PDT_UNKNOWN;
               break;
            }


            if ( is_supported_type )
            {
               DumpQuadData(dflag, data_type);
#ifdef SHOW_CURR_VALUE
               printf("kstat::%s ---> [%lld]\n", provstr, value);
#else
               printf("kstat::%s\n", provstr);
#endif
               icount++;
            }

            i++;
            
         }
      }


      ksp = ksp->ks_next;
   }

   return(icount);
}

/* ========================================================================= */
struct kdescriptor *build_kdescriptor(struct qparts *qp)
{
   struct kdescriptor *ksd;
   int i, j;
   int bl, br;

   /* Go ahead and malloc() - there will be no free() on error because at 
      this point exit() always follows failure. */
   if (NULL == (ksd = (struct kdescriptor *)malloc(sizeof(struct kdescriptor))))
      return(NULL);

   /* Do some basic sanity checking */
   bl = 0;
   br = 0;
   i = 0;
   while ( qp->iname[i] != 0 )
   {
      if ( qp->iname[i] == '[' )
         bl++;

      if ( qp->iname[i] == ']' )
         br++;

      i++;
   }

   if ( ( bl != 4 ) || ( br != 4 ) )
   {
      error_msg("ERROR: kstat quad not in expected format.");
      return(NULL);
   }

   if ( qp->iname[0] != '[' )
   {
      error_msg("ERROR: kstat quad item does not begin with \"[\" char.");
      return(NULL);
   }

   /* parse the module name */
   i = 1;
   j = 0;
   while((qp->iname[i] != ']') && (qp->iname[i] != 0))
      ksd->ks_module[j++] = qp->iname[i++];

   ksd->ks_module[j] = 0;  /* terminate */

   /* move over the ][ */
   if ((qp->iname[i] == ']') && (qp->iname[i + 1] == '['))
      i += 2;
   else
   {
      error_msg("ERROR: kstat descriptor is not delimited by \"][\" char.");
      return(NULL);
   }

   /* parse the instance */
   ksd->ks_instance = 0;
   while((qp->iname[i] != ']') && (qp->iname[i] != 0))
   {
      if ((qp->iname[i] >= '0') && (qp->iname[i] <= '9'))
      {
         ksd->ks_instance *= 10;
         ksd->ks_instance += qp->iname[i] - '0';
      }
      else
      {
         error_msg("ERROR: kstat instance descriptor is not numeric.");
         return(NULL);
      }

      i++;
   }

   /* move off the ][ */
   if ((qp->iname[i] == ']') && (qp->iname[i + 1] == '['))
      i += 2;
   else
   {
      error_msg("ERROR: kstat descriptor is not delimited by \"][\" char.\n");
      return(NULL);
   }

   /* parse the ks_name (not the name=value pair name) */
   j = 0;
   while((qp->iname[i] != ']') && (qp->iname[i] != 0))
      ksd->ks_name[j++] = qp->iname[i++];

   ksd->ks_name[j] = 0;

   /* move off the ][ */
   if ((qp->iname[i] == ']') && (qp->iname[i + 1] == '['))
      i += 2;
   else
   {
      error_msg("ERROR: kstat descriptor is not delimited by \"][\" char.\n");
      return(NULL);
   }

   /* parse the name=value name (not the ks_name) */
   j = 0;
   while((qp->iname[i] != ']') && (qp->iname[i] != 0))
      ksd->nv_name[j++] = qp->iname[i++];

   ksd->nv_name[j] = 0;

   /* resolve the data types (and malloc the memory) */
   if ( resolve_data_type(ksd) )
   {
      error_msg("ERROR: Unable to determine data type for kstat() name=value pair.\n");
      return(NULL);
   }

   return(ksd);
}




/* ========================================================================= */
int resolve_data_type(struct kdescriptor *ksd)
{
   kstat_ctl_t *kc;
   kstat_t *ksp;
   kstat_named_t *tsd_named;
   int i;

   ksd->data_type = -1; /* This is an error condition. Clear it to survive */

   kc = kstat_open();
   
   if (NULL == (ksp = kstat_lookup(kc, ksd->ks_module, ksd->ks_instance, ksd->ks_name)))
      return(1);

   if (-1 == kstat_read(kc, ksp, NULL))
      return(1);

   tsd_named = KSTAT_NAMED_PTR(ksp);

   i = 0;
   while( i < ksp->ks_ndata )
   {
      tsd_named[i].name[KSTAT_STRLEN-1] = '\0';

      if ( 0 == strcmp(tsd_named[i].name, ksd->nv_name))
      {
         /* Take care of the data type */
         switch ( tsd_named[i].data_type )
         {
         case KSTAT_DATA_STRING:
         case KSTAT_DATA_CHAR:
            ksd->data_type = tsd_named[i].data_type;
            /* STUB Not quite supported */
            ksd->this_data = malloc(32);
            ksd->last_data = malloc(32);
            break;
         case KSTAT_DATA_INT32:
         case KSTAT_DATA_UINT32:
            ksd->data_type = tsd_named[i].data_type;
            ksd->this_data = malloc(sizeof(int32_t));
            ksd->last_data = malloc(sizeof(int32_t));
            break;
         case KSTAT_DATA_INT64:
         case KSTAT_DATA_UINT64:
            ksd->data_type = tsd_named[i].data_type;
            ksd->this_data = malloc(sizeof(int64_t));
            ksd->last_data = malloc(sizeof(int64_t));
            break;
         default:
            return(1);
            break;
         }

         if (( NULL == ksd->this_data ) ||( NULL == ksd->last_data ) || ( -1 == ksd->data_type ))
             return(1);

         return(0);
      }

      i++;
   }

   kstat_close(kc);

   return(1);
}

/* ========================================================================= */
#define STUB_STRING_LEN 32
struct pitem *kstat_enablepitem(struct qparts *qp)
{
   struct pitem *this_pi;
   int data_type;
   void *data_ptr;

   struct kdescriptor *ksd;

   if ( NULL == (ksd = build_kdescriptor(qp)) )
      return(NULL);

   switch ( ksd->data_type )
   {
   case KSTAT_DATA_STRING:
   case KSTAT_DATA_CHAR:
      data_type = PDT_STRING;
      data_ptr = malloc(STUB_STRING_LEN);
      break;
   case KSTAT_DATA_INT32:
      data_type = PDT_INT32;
      data_ptr = malloc(sizeof(int32_t));
      break;
   case KSTAT_DATA_UINT32:
      data_type = PDT_UINT32;
      data_ptr = malloc(sizeof(uint32_t));
      break;
   case KSTAT_DATA_INT64:
      data_type = PDT_INT64;
      data_ptr = malloc(sizeof(int64_t));
      break;
   case KSTAT_DATA_UINT64:
      data_type = PDT_UINT64;
      data_ptr = malloc(sizeof(uint64_t));
      break;
   default:
      data_type = PDT_UNKNOWN;
      data_ptr = NULL;
      break;
   }

   /* Create the pitem */
   if ( NULL == (this_pi = NewPItem(qp, data_type, data_ptr)) )
      return(NULL);
   
   /* Associate the ksd with the pitem */
   this_pi->dstruct = (void *)ksd;

   /* Insert into the update item list */
   if(InsertUpdateItem(myself, this_pi))
      return(NULL);

   /* Flag update for the provider */
   myself->update_required = 1;


   return(this_pi);
}


/* ========================================================================= */
int kstat_update(int interval)
{
   struct pitem *this_pi;
   kstat_ctl_t *kc;
   struct kdescriptor *ksd;
   int rv;

   rv = 0;

   if (NULL == (kc = kstat_open()))
      return(1);

   this_pi = myself->ui_list;
   while ( this_pi )
   {
      ksd = (struct kdescriptor *)this_pi->dstruct;

      if (update_ksitem(kc, ksd))
         rv = 1;
      else
      {
         CalcData(this_pi, ksd->last_data, ksd->this_data, interval);



#ifdef STUB_REMOVE_THIS
         /*
         if(this_pi->data_type == PDT_INT64)
         {
            if ( this_pi->munge_flag & MUNGE_DIFF )
            {
               *((int64_t *)this_pi->data_ptr) = *((int64_t *)ksd->this_data) - *((int64_t *)ksd->last_data);
            }
            else
            {
               *((int64_t *)this_pi->data_ptr) = *((int64_t *)ksd->this_data);
            }

            if ( this_pi->munge_flag & MUNGE_PSAVG )
            {
               *((int64_t *)this_pi->data_ptr) /= (int64_t)interval;
            }

         }

         if(this_pi->data_type == PDT_UINT64)
         {
            if ( this_pi->munge_flag & MUNGE_DIFF )
            {
               *((uint64_t *)this_pi->data_ptr) = *((uint64_t *)ksd->this_data) - *((uint64_t *)ksd->last_data);
            }
            else
            {
               *((uint64_t *)this_pi->data_ptr) = *((uint64_t *)ksd->this_data);
            }

            if ( this_pi->munge_flag & MUNGE_PSAVG )
            {
               *((int64_t *)this_pi->data_ptr) /= (int64_t)interval;
            }
         }
         */
#endif

      }

      this_pi = this_pi->next_ui;
   }

   return(rv);
}


/* ========================================================================= */
int update_ksitem(kstat_ctl_t *kc, struct kdescriptor *ksd)
{
   kstat_t *ksp;
   kstat_named_t *tsd_named;
   void *temp_data;
   int i;

   if (NULL == (ksp = kstat_lookup(kc, ksd->ks_module, ksd->ks_instance, ksd->ks_name)))
   {
      return(1);
   }

   if (-1 == kstat_read(kc, ksp, NULL))
   {
      return(1);
   }

   tsd_named = KSTAT_NAMED_PTR(ksp);

   i = 0;
   while( i < ksp->ks_ndata )
   {
      tsd_named[i].name[KSTAT_STRLEN-1] = '\0';

      if ( 0 == strcmp(tsd_named[i].name, ksd->nv_name))
      {
         /* Rotate data */
         temp_data = ksd->last_data;
         ksd->last_data = ksd->this_data;
         ksd->this_data = temp_data;

         /* Assign data */
         switch ( tsd_named[i].data_type )
         {
         case KSTAT_DATA_STRING:
         case KSTAT_DATA_CHAR:
            strcpy((char *)temp_data, "Not quite supported");
            break;
         case KSTAT_DATA_INT32:
            *((int32_t *)temp_data) = tsd_named[i].value.i32;
            break;
         case KSTAT_DATA_UINT32:
            *((uint32_t *)temp_data) = tsd_named[i].value.ui32;
            break;
         case KSTAT_DATA_INT64:
            *((int64_t *)temp_data) = tsd_named[i].value.i64;
            break;
         case KSTAT_DATA_UINT64:
            *((uint64_t *)temp_data) = tsd_named[i].value.ui64;
            break;
         default:
            return(1);
            break;
         }

         return(0);
      }

      i++;
   }

   return(1);
}



















































































#ifdef STUB_OLD_CODE

   int icount = 0;



   kstat_t *ksp;
   kstat_named_t *tsd_named;
   char provstr[64];
   int data_type;
   int i;


   /* STUB - BEGIN TEST CODE */
   long long ksvalue;   /* STUB - Temp use only */
   


   
   strcpy(ksd.ks_module, "vmem");
   ksd.ks_instance = 1;
   strcpy(ksd.ks_name, "heap");
   strcpy(ksd.nv_name, "alloc");
   ksd.this_data = &ksvalue;
   ksd.last_data = &ksvalue;
   
   kc = kstat_open();

   if (update_ksitem(kc, &ksd))
      printf("Failed to get alloc value\n");      
   else
      printf("alloc = %lld\n", ksvalue);

   kstat_close(kc);





   return(0);
}

#endif




#ifdef STUB_OLD_CODE_LOCKER
   if (NULL == (ksp = kstat_lookup(kc, "vmem", 1, "heap")))
   {
      fprintf(stderr, "DBGERR: kstat_lookup() failed.\n");
      kstat_close(kc);
      return(1);
   }
   
   kstat_read(kc, ksp, NULL);

   tsd_named = KSTAT_NAMED_PTR(ksp);

   i = 0;
   while ( i < ksp->ks_ndata )
   {
      tsd_named[i].name[KSTAT_STRLEN-1] = '\0';

      if ( 0 == strcmp(tsd_named[i].name, "alloc"))
      {
         ksvalue = tsd_named[i].value.ui64;
         printf("alloc = %lld\n", ksvalue);
      }

      i++;
   }









   /* STUB - BEGIN TEST CODE */
   long long ksvalue;   /* STUB - Temp use only */
   struct kdescriptor ksd;

   kc = kstat_open();
   
   strcpy(ksd.ks_module, "vmem");
   ksd.ks_instance = 1;
   strcpy(ksd.ks_name, "heap");
   strcpy(ksd.nv_name, "alloc");
   ksd.this_data = &ksvalue;
   ksd.last_data = &ksvalue;
   
   kc = kstat_open();

   if (update_ksitem(kc, &ksd))
      printf("Failed to get alloc value\n");      
   else
      printf("alloc = %lld\n", ksvalue);

   kstat_close(kc);

   /* STUB - END TEST CODE */
#endif




