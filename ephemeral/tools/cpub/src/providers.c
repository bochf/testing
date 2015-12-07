#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "mainhelp.h"
#include "providers.h"

/* ========================================================================= */
struct proot *Providers(void)
{
   struct proot *p;

   if ( NULL == (p = (struct proot *)malloc(sizeof(struct proot))) )
   {
      return(NULL);
   }

   p->prov_list = NULL;
   p->pi_olist = NULL;
   p->pitem_count = 0;

   return(p);
}

/* ========================================================================= */
struct pitem *NewPItem(struct qparts *qp,
                       int data_type,
                       void *data_ptr)
{
   struct pitem *new_pi;

   /* Validate input */
   if (( NULL == data_ptr) || ( NULL == qp ) || ( NULL == qp->cfpn ))
      return(NULL);

   /* Allocate memory */
   if ( NULL == (new_pi = (struct pitem *)malloc(sizeof(struct pitem))) )
      return(NULL);

   /* set basic / default / required values */
   strcpy(new_pi->name, qp->iname);
   new_pi->data_type = data_type;
   new_pi->data_ptr = data_ptr;
   new_pi->next_ui = NULL;
   new_pi->next_opi = NULL;
   new_pi->sign_flag = 0; /* This might never be used set it to 0 */
   new_pi->wdata = NULL;

   /* Discussion point - on assigning writer args to the pitem
      - This is done because the writer needs to see the arguments as directly
        related to the pitem, not the prov_name (the quad info as read from
        the config file.) This is because there may not be a 1:1 relationship
        between the items the user input and the number of pitems. For example,
        if the user specified some sort of wild-card matching character in 
        the provider args and 10 pitems were spawned for a single quad. (No,
        there is no current support for that, but their could be.)
      - This makes the binding between configfile.h/c, providers.h/c, and
        the writer module more tight. So this represents a challenge.
      - This cannot be too-loose because the provider might need to weigh
        in on the arguments used. For example:
        * The provider can say that some data cannot be alarmed or factored
          (this is really unlikely)
        * The provider can modify the header (quite possible)
      - This is NOT a clean hand-off, but the restriction is that the hand-
        off happens in this function.
   */

   /* Move over the header - if set */
   if ( NULL != qp->cfpn->header )
      strncpy(new_pi->header, qp->cfpn->header, MAX_QPART_LEN);
   else
      strcpy(new_pi->header, qp->iname);
   
   /* Move over the alarm_at info */
   new_pi->alarm_type = qp->cfpn->alarm_type;
   new_pi->alarm_value = qp->cfpn->alarm_value;
   
   /* Move over the factor info */
   new_pi->fact_from = qp->cfpn->fact_from;
   new_pi->fact_to = qp->cfpn->fact_to;
   
   /* Parse iargs */
   new_pi->munge_flag = MUNGE_RAW; /* Default to nothing */

   if ( ShouldDiff(qp) )
      new_pi->munge_flag |= MUNGE_DIFF;

   if ( ShouldPSAvg(qp) )
      new_pi->munge_flag |= MUNGE_PSAVG;

   if ((new_pi->fact_from != 0) && (new_pi->fact_to != 0))
      new_pi->munge_flag |= MUNGE_FACTOR;

   return(new_pi);
}

/* ========================================================================= */
struct provider *RegisterProvider(struct proot *p,
                                  char *name,
                                  int (*refresh)(int interval),
                                  int (*listavail)(int dflag),
                                  struct pitem *(*enablepitem)(struct qparts *qp))
{
   struct provider *this_pro;

   if (NULL == (this_pro = (struct provider *)malloc(sizeof(struct provider))))
      return(NULL);

   this_pro->next = p->prov_list;
   p->prov_list = this_pro;

   strcpy(this_pro->name, name);
   this_pro->update_required = 0;
   this_pro->refresh = refresh;
   this_pro->listavail = listavail;
   this_pro->enablepitem = enablepitem;
   this_pro->ui_list = NULL;

   return(this_pro);
}

/* ========================================================================= */
int DumpPItemList(struct proot *root, int dflag)
{
   struct provider *this_p;
   int cnt;

   this_p = root->prov_list;
   cnt = 0;

   while(this_p)
   {
      printf("%s\n", this_p->name);
      this_p->listavail(dflag);

      cnt++;
      this_p = this_p->next;
   }

   return(cnt);
}

/* ========================================================================= */
void DumpQuadData(int dflag, int data_type)
{
   printf("  ");


   if ( dflag & DPIL_DATATYPE )
   {
      switch ( data_type )
      {
      case PDT_STRING:
         printf("STRING ");
         break;
      case PDT_INT8:
         printf("INT8   ");
         break;
      case PDT_UINT8:
         printf("UINT8  ");
         break;
      case PDT_INT16:
         printf("INT16  ");
         break;
      case PDT_UINT16:
         printf("UINT16 ");
         break;
      case PDT_INT32:
         printf("INT32  ");
         break;
      case PDT_UINT32:
         printf("UINT32 ");
         break;
      case PDT_INT64:
         printf("INT64  ");
         break;
      case PDT_UINT64:
         printf("UINT64 ");
         break;
      case PDT_TIMEVAL:
         printf("TMVAL  ");
         break;
      case PDT_TIMESPEC:
         printf("TMSPC  ");
         break;
      case PDT_UNKNOWN:
      default:
         printf("UNK    ");
         break;
      }
   }
}

/* ========================================================================= */
struct provider *GetProviderByNameStr(struct proot *p, char *quad)
{
   struct qparts qp;
   struct provider *prov;

   if ( StrToQuadPartsFill(&qp, quad) )
      return(NULL);

   prov = p->prov_list;

   while(prov)
   {
      if (0 == strcmp(prov->name, qp.pname))
         return(prov);

      prov = prov->next;
   }

   return(NULL);
}

/* ========================================================================= */
struct provider *GeProviderByNameQP(struct proot *p, struct qparts *qp)
{
   struct provider *prov;

   if ( NULL == qp )
      return(NULL);

   prov = p->prov_list;

   while(prov)
   {
      if (0 == strcmp(prov->name, qp->pname))
         return(prov);

      prov = prov->next;
   }

   return(NULL);
}

/* ========================================================================= */
struct qparts *StrToQuadPartsCreate(char *quad)
{
   struct qparts qp;
   struct qparts *rqp;
   int colon_count;
   int i, j;

   if ( NULL == quad )
      return(NULL);

   if ( 0 == quad[0] )
      return(NULL);

   i = 0;
   colon_count = 0;
   while ( quad[i] != 0 )
   {
      if ( ':' == quad[i] )
         colon_count++;

      i++;
   }

   if (( colon_count < 2) || ( colon_count > 3 ))
      return(NULL);

   /* Go ahead and clear the reference to prov_name. It is not passed, this
      function has no visibility to it. So don't pretend we do. */
   qp.cfpn = NULL;

   /* Parse off the first quad part - the provider */
   i = 0;
   j = 0;
   while ( quad[i] != ':' )
      qp.pname[j++] = quad[i++];
      
   qp.pname[j] = 0;

   /* Parse off the first quad part - the provider */
   i = 0;
   j = 0;
   while ( quad[i] != ':' )
      qp.pname[j++] = quad[i++];
      
   qp.pname[j] = 0;
   i++;

   if ( j == 0 )
      return(NULL);

   /* Parse off the second quad part - the provider args */
   j = 0;
   while ( quad[i] != ':' )
      qp.pargs[j++] = quad[i++];
      
   qp.pargs[j] = 0;
   i++;

   /* Parse off the third quad part - the data point (name) */
   j = 0;
   while (( quad[i] != ':' ) || ( quad[i] != 0 ))
      qp.iname[j++] = quad[i++];
      
   qp.iname[j] = 0;
   i++;

   if ( j == 0 )
      return(NULL);
   
   if ( colon_count == 3 )
   {
      /* Parse off the fourth quad part - the data point args */
      j = 0;
      while (( quad[i] != 0 ) && ( quad[i] != ' ' ) && ( quad[i] != '\t' ))
         qp.iargs[j++] = quad[i++];
      
      qp.iargs[j] = 0;
   }
   else
      qp.iargs[0] = 0;

   /* Ok - the data is good, let's allocate a perm place for it */
   if ( NULL == (rqp = (struct qparts *)malloc(sizeof(struct qparts))) )
      return(NULL); /* this is exceptionally unlikely - no bother genning errno */

   memcpy(rqp, &qp, sizeof(struct qparts));

   return(rqp);
}

/* ========================================================================= */
int StrToQuadPartsFill(struct qparts *qp, char *quad)
{
   int colon_count;
   int i, j;

   if ( NULL == quad )
      return(1);

   if ( 0 == quad[0] )
      return(1);

   i = 0;
   colon_count = 0;
   while ( quad[i] != 0 )
   {
      if ( ':' == quad[i] )
         colon_count++;

      i++;
   }

   if (( colon_count < 2) || ( colon_count > 3 ))
      return(1);

   /* Go ahead and clear the reference to prov_name. It is not passed, this
      function has no visibility to it. So don't pretend we do. */
   qp->cfpn = NULL;

   /* Parse off the first quad part - the provider */
   i = 0;
   j = 0;
   while ( quad[i] != ':' )
      qp->pname[j++] = quad[i++];
      
   qp->pname[j] = 0;
   i++; /* Get off the colon */

   /* Parse off the second quad part - the provider args */
   j = 0;
   while ( quad[i] != ':' )
      qp->pargs[j++] = quad[i++];
      
   qp->pargs[j] = 0;
   i++;

   /* Parse off the third quad part - the data point (name) */
   j = 0;
   while (( quad[i] != ':' ) && ( quad[i] != 0 ) && ( quad[i] != ' ' ) && ( quad[i] != '\t' ))
      qp->iname[j++] = quad[i++];
      
   qp->iname[j] = 0;
   i++;

   if ( j == 0 )
      return(1);
   
   if ( colon_count == 3 )
   {
      /* Parse off the fourth quad part - the data point args */
      j = 0;
      while (( quad[i] != 0 ) && ( quad[i] != ' ' ) && ( quad[i] != '\t' ))
         qp->iargs[j++] = quad[i++];
      
      qp->iargs[j] = 0;
   }
   else
      qp->iargs[0] = 0;

   return(0);
}

/* ========================================================================= */
int ShouldDiff(struct qparts *qp)
{
   char IARGS[MAX_QPART_LEN];
   int i;

   /* Capitalize - the cost here is not significant (and we do not do this
      often */
   i = 0;
   while(i < MAX_QPART_LEN)
   {
      if (( qp->iargs[i] >= 'a' ) && ( qp->iargs[i] <= 'z' ))
         IARGS[i] = qp->iargs[i] - 32;
      else
         IARGS[i] = qp->iargs[i];

      i++;
   }

   if ( strstr(IARGS, "DIFF") )
      return(1);

   return(0);
}

/* ========================================================================= */
int ShouldPSAvg(struct qparts *qp)
{
   char IARGS[MAX_QPART_LEN];
   int i;

   /* Capitalize - the cost here is not significant (and we do not do this
      often */
   i = 0;
   while(i < MAX_QPART_LEN)
   {
      if (( qp->iargs[i] >= 'a' ) && ( qp->iargs[i] <= 'z' ))
         IARGS[i] = qp->iargs[i] - 32;
      else
         IARGS[i] = qp->iargs[i];

      i++;
   }

   if ( strstr(IARGS, "PSAVG") )
      return(1);

   return(0);
}

/* ========================================================================= */
int InsertUpdateItem(struct provider *prov, struct pitem *pi)
{
   if (( NULL == prov ) || ( NULL == pi ))
      return(1);

   /* Order does not matter */
   pi->next_ui = prov->ui_list; /* Put existing list on the end */
   prov->ui_list = pi;          /* Make this the head of the list */

   return(0);
}

/* ========================================================================= */
int EnableDataPoint(struct proot *p, struct prov_name *pn)
{
   struct provider *prov;
   struct pitem *this_pi; /* Used to walk to the end of the linked list */
   struct pitem *last_pi; /* Used to walk to the end of the linked list */
   struct pitem *data_pi; /* The pitem that will be in our output data  */
   struct qparts qp;

   /* Here is the problem:
      - It is not sufficient to just pass the quad.
      - The header is needed.
      - The alarm values are needed.
      
      - The above values cannot be passed (held in) the prov_name (list).

   */

   /* This determines what provider supports this quad description */
   if ( NULL == (prov = GetProviderByNameStr(p, pn->quad)) )
      return(1);

   /* So the provider is supported.... convert this string quad to a qparts
      struct - so it can be passed to the enablepitem() function registered
      for the provider. */
   if(StrToQuadPartsFill(&qp, pn->quad))
      return(1);
   
   /* Piggy-back the "raw" prov name onto the quad. We know this data is
      destined for the provider that will then use NewPItem(). So we can
      tack on this info, and let NewPItem() use it. */
   qp.cfpn = pn;


   /* Enable the provider for data collection */
   /* Get MUST be called after Enable. Some providers do not create until
      they enable! */
   if ( NULL == (data_pi = prov->enablepitem(&qp)) )
   {
      /* No error message here. There is a single error message for enablement
         failures. */
      return(1);
   }

   /* Ok - the Provider is now hot - as is the pitem. Let's make any 
      modifications that the provider implementation  does not need to be
      concerned with. */

   /* The header is set in InitPItem() that is called from enablepitem().
      There is no need to set it here. The only reason is if the provider
      implementation used something else to malloc() the pitem. This
      should not happen, and should be caught in test - so no explicit
      test for it here. */

   /* WHAT IS GOING ON HERE <------ THIS IS KIND OF IMPORTANT!
      enablepitem() *can* return more than one object! If we are going to
      do anything to the returned pitem, then we need to do it to ALL
      the returned pitems.
   */
   this_pi = data_pi;
   while ( this_pi )
   {
      /* Save the alarm information in the pitem */
      this_pi->alarm_type = pn->alarm_type;
      if ( pn->alarm_type != ALARM_NO )
         this_pi->alarm_value = pn->alarm_value;

      this_pi = this_pi->next_opi;
   }

   /* Place this (list of) item(s) at the END of the linked list */
   if ( NULL == p->pi_olist )
      p->pi_olist = data_pi;
   else
   {
      this_pi = p->pi_olist->next_opi;
      last_pi = p->pi_olist;
      
      while(this_pi)
      {
         last_pi = this_pi;
         this_pi = this_pi->next_opi;
      }

      last_pi->next_opi = data_pi;
   }

   return(0);
}

/* ========================================================================= */
int GetActiveProviderCount(struct proot *p)
{
   struct provider *this_prov;
   int count;

   if ( NULL == p )
      return(-1);

   count = 0;
   this_prov = p->prov_list;
   while ( this_prov )
   {
      if ( this_prov->update_required )
         count++;

      this_prov = this_prov->next;
   }

   return(count);
}

/* ========================================================================= */
struct provider *GetNextActiveProvider(struct proot *p, struct provider *last)
{
   struct provider *this_prov;

   assert( NULL != p );

   if ( NULL == last )
      this_prov = p->prov_list; /* Start from beginning */
   else                         /* or */
      this_prov = last->next;   /* The next from the last found */

   while ( this_prov )
   {
      if ( this_prov->update_required )
         return(this_prov);

      this_prov = this_prov->next;
   }

   return(NULL);
}

/* ========================================================================= */
int CalcData(struct pitem *pi, void *last_data, void *this_data, int interval)
{
   assert(NULL != pi);
   assert(NULL != last_data);
   assert(NULL != this_data);
   assert(interval > 0);
   assert(pi->data_type != PDT_UNKNOWN);


   /* !!! READ ME !!!
      Don't come into this code and freak out because the casts do not
      match the cases (case UINT32 --> (int32_t)). This is because the sign
      is ripped off and stored elsewhere.
   */
   
   /* Handle the signed types first - the easy case */
   if ( ( pi->data_type == PDT_INT8  ) ||
        ( pi->data_type == PDT_INT16 ) ||
        ( pi->data_type == PDT_INT32 ) ||
        ( pi->data_type == PDT_INT64 ) )
   {
      if ( pi->munge_flag & MUNGE_DIFF )
      {
         /* Do a diff */
         switch ( pi->data_type )
         {
         case PDT_INT8:
            *((int8_t *)pi->data_ptr) = *((int8_t *)this_data) - *((int8_t *)last_data);
            break;
         case PDT_INT16:
            *((int16_t *)pi->data_ptr) = *((int16_t *)this_data) - *((int16_t *)last_data);
            break;
         case PDT_INT32:
            *((int32_t *)pi->data_ptr) = *((int32_t *)this_data) - *((int32_t *)last_data);
            break;
         case PDT_INT64:
            *((int64_t *)pi->data_ptr) = *((int64_t *)this_data) - *((int64_t *)last_data);
            break;
         }
      }
      else /* if ( DIFF ) */
      {
         /* Do a simple assignment */
         switch ( pi->data_type )
         {
         case PDT_INT8:
            *((int8_t *)pi->data_ptr) = *((int8_t *)this_data);
            break;
         case PDT_INT16:
            *((int16_t *)pi->data_ptr) = *((int16_t *)this_data);
            break;
         case PDT_INT32:
            *((int32_t *)pi->data_ptr) = *((int32_t *)this_data);
            break;
         case PDT_INT64:
            *((int64_t *)pi->data_ptr) = *((int64_t *)this_data);
            break;
         }
      } /* if ( DIFF ) ... else */

      /* Check for sign and remove it (this is about consistency) */
      pi->sign_flag = 0;
      switch ( pi->data_type )
      {
      case PDT_INT8:
         if ( *((int8_t *)pi->data_ptr) < 0 )
         {
            *((int8_t *)pi->data_ptr) *= (int8_t)-1;
            pi->sign_flag = 1;
         }            
         break;
      case PDT_INT16:
         if ( *((int16_t *)pi->data_ptr) < 0 )
         {
            *((int16_t *)pi->data_ptr) *= (int16_t)-1;
            pi->sign_flag = 1;
         }            
         break;
      case PDT_INT32:
         if ( *((int32_t *)pi->data_ptr) < 0 )
         {
            *((int32_t *)pi->data_ptr) *= (int32_t)-1;
            pi->sign_flag = 1;
         }            
         break;
      case PDT_INT64:
         if ( *((int64_t *)pi->data_ptr) < 0 )
         {
            *((int64_t *)pi->data_ptr) *= (int64_t)-1;
            pi->sign_flag = 1;
         }            
         break;
      }
      
      if ( pi->munge_flag & MUNGE_PSAVG )
      {
         switch ( pi->data_type )
         {
         case PDT_INT8:
            *((int8_t *)pi->data_ptr) /= (int8_t)interval;
            break;
         case PDT_INT16:
            *((int16_t *)pi->data_ptr) /= (int16_t)interval;
            break;
         case PDT_INT32:
            *((int32_t *)pi->data_ptr) /= (int32_t)interval;
            break;
         case PDT_INT64:
            *((int64_t *)pi->data_ptr) /= (int64_t)interval;
            break;
         }
      }

      return(0);
   } /* if ( signed integer ) */


   /* Handle the unsigned types */
   if ( ( pi->data_type == PDT_UINT8  ) ||
        ( pi->data_type == PDT_UINT16 ) ||
        ( pi->data_type == PDT_UINT32 ) ||
        ( pi->data_type == PDT_UINT64 ) )
   {
      pi->sign_flag = 0; /* Start off by assuming no sign */

      if ( pi->munge_flag & MUNGE_DIFF )
      {
         /* Do a diff */
         switch ( pi->data_type )
         {
         case PDT_UINT8:
            if ( *((int8_t *)last_data) > *((int8_t *)this_data) )
            {
               pi->sign_flag = 1;
               *((int8_t *)pi->data_ptr) = *((int8_t *)last_data) - *((int8_t *)this_data);
            }
            else
               *((int8_t *)pi->data_ptr) = *((int8_t *)this_data) - *((int8_t *)last_data);
            break;
         case PDT_UINT16:
            if ( *((int16_t *)last_data) > *((int16_t *)this_data) )
            {
               pi->sign_flag = 1;
               *((int16_t *)pi->data_ptr) = *((int16_t *)last_data) - *((int16_t *)this_data);
            }
            else
               *((int16_t *)pi->data_ptr) = *((int16_t *)this_data) - *((int16_t *)last_data);
            break;
         case PDT_UINT32:
            if ( *((int32_t *)last_data) > *((int32_t *)this_data) )
            {
               pi->sign_flag = 1;
               *((int32_t *)pi->data_ptr) = *((int32_t *)last_data) - *((int32_t *)this_data);
            }
            else
               *((int32_t *)pi->data_ptr) = *((int32_t *)this_data) - *((int32_t *)last_data);
            break;
         case PDT_UINT64:
            if ( *((int64_t *)last_data) > *((int64_t *)this_data) )
            {
               pi->sign_flag = 1;
               *((int64_t *)pi->data_ptr) = *((int64_t *)last_data) - *((int64_t *)this_data);
            }
            else
               *((int64_t *)pi->data_ptr) = *((int64_t *)this_data) - *((int64_t *)last_data);
            break;
         }
      }
      else
      {
         /* Do a simple assignment */
         switch ( pi->data_type )
         {
         case PDT_UINT8:
            *((int8_t *)pi->data_ptr) = *((int8_t *)this_data);
            break;
         case PDT_UINT16:
            *((int16_t *)pi->data_ptr) = *((int16_t *)this_data);
            break;
         case PDT_UINT32:
            *((int32_t *)pi->data_ptr) = *((int32_t *)this_data);
            break;
         case PDT_UINT64:
            *((int64_t *)pi->data_ptr) = *((int64_t *)this_data);
            break;
         }
      }
      
      if ( pi->munge_flag & MUNGE_PSAVG )
      {
         switch ( pi->data_type )
         {
         case PDT_UINT8:
            *((int8_t *)pi->data_ptr) /= (int8_t)interval;
            break;
         case PDT_UINT16:
            *((int16_t *)pi->data_ptr) /= (int16_t)interval;
            break;
         case PDT_UINT32:
            *((int32_t *)pi->data_ptr) /= (int32_t)interval;
            break;
         case PDT_UINT64:
            *((int64_t *)pi->data_ptr) /= (int64_t)interval;
            break;
         }
      }

      return(0);
   } /* if ( unsigned type ) */


   if ( ( pi->data_type == PDT_TIMEVAL  ) ||
        ( pi->data_type == PDT_TIMESPEC ) ||
        ( pi->data_type == PDT_STRING   ) )
   {
      /* Not handled - because previous and next values are not kept.
         the data is simply assigned on request. We could fall through
         but this needs to be an assert() */
      assert(0);
      /* the additionall thinking here is that these items are at least
         recognized. So, if there were to be a case where they were 
         *somehow* handled, then that code could be included at a later
         date. */
   }

   /* Data type not matched, not handled -> error condition */
   return(1);
}

