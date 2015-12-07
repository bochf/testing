#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <stdint.h>

#include "query.h"
#include "ssearch.h"
#include "filefmt.h"

struct listitem
{
   char *display; /* What you display or use for case sensitive searches */
   char *pattern; /* What you use for case insensitive seraches - in upper case */

   struct listitem *next;
};

/* ========================================================================= */
int key_match(char *haystack, char *needles)
{
   char needle[32];
   int i, j;

   if ( needles == NULL )
      return(1);

   if ( needles[0] == 0 )
      return(1);

   i = 0;
   while ( needles[i] != 0 )
   {
      j = 0;
      while ( ( needles[i] != ',' ) && ( needles[i] != 0 ) )
         needle[j++] = needles[i++];
         
      needle[j] = 0;
      if ( needles[i] == ',' ) /* if we stopped on a , move off */
         i++;

      if ( NULL == strstr(haystack, needle) )
         return(0);
   }

   return(1);
}


/* ========================================================================= */
inline char *get_entry_item_offset(void *entryloc, int offsetloc)
{
   /* Account for the header */
   uint16_t offsetval = DBV2_ENTRY_FIRSTD;
   uint16_t *itemval;

   /* Walk the list, stop out when we have what we need */

   if ( offsetloc == DBV2_ENTRY_NAME )
      return(entryloc + offsetval);

   itemval = entryloc + DBV2_ENTRY_NAME;
   offsetval += *itemval;

   if ( offsetloc == DBV2_ENTRY_DESC )
      return(entryloc + offsetval);

   itemval = entryloc + DBV2_ENTRY_DESC;
   offsetval += *itemval;

   if ( offsetloc == DBV2_ENTRY_KEYS )
      return(entryloc + offsetval);

   itemval = entryloc + DBV2_ENTRY_KEYS;
   offsetval += *itemval;

   if ( offsetloc == DBV2_ENTRY_LOCN )
      return(entryloc + offsetval);

   itemval = entryloc + DBV2_ENTRY_LOCN;
   offsetval += *itemval;

   if ( offsetloc == DBV2_ENTRY_PFRM )
      return(entryloc + offsetval);

   itemval = entryloc + DBV2_ENTRY_PFRM;
   offsetval += *itemval;

   if ( offsetloc == DBV2_ENTRY_OWNR )
      return(entryloc + offsetval);

   itemval = entryloc + DBV2_ENTRY_OWNR;
   offsetval += *itemval;

   if ( offsetloc == DBV2_ENTRY_FILE )
      return(entryloc + offsetval);

   return(NULL);
}

char *GetEntryItemOffset(void *el, int ol)
{
   return(get_entry_item_offset(el, ol));
}

/* ========================================================================= */
char *parse_impact_value(char *strtofill, uint16_t bitfield)
{
   strtofill[0] = 0;

   if ( DBV2_ENTRY_IMPACT_NONE == (bitfield & DBV2_ENTRY_IMPACT_MASK) )
      strcpy(strtofill, "None");

   if ( DBV2_ENTRY_IMPACT_LOWI == (bitfield & DBV2_ENTRY_IMPACT_MASK) )
      strcpy(strtofill, "Low");

   if ( DBV2_ENTRY_IMPACT_MEDI == (bitfield & DBV2_ENTRY_IMPACT_MASK) )
      strcpy(strtofill, "Medium");

   if ( DBV2_ENTRY_IMPACT_HIGH == (bitfield & DBV2_ENTRY_IMPACT_MASK) )
      strcpy(strtofill, "High");

   return(strtofill);
}

/* ========================================================================= */
#define DUMP_CASE_SENSITIVE 0
#define DUMP_UPPER_CASE     1
#define DUMP_LOWER_CASE     2
int dump_item_list(struct listitem *li, int caseflag)
{
   struct listitem *thisitem;
   int cnt;

   /* If we are asked to dump the top of an empty list, then just return */
   if ( NULL == li )
      return(0);

   if (( caseflag == DUMP_LOWER_CASE ) || ( caseflag == DUMP_UPPER_CASE ))
   {
      if ( li->pattern == NULL )
         caseflag = DUMP_CASE_SENSITIVE;
   }

   cnt = 0;
   thisitem = li;
   while ( thisitem )
   {
      switch ( caseflag )
      {
      case DUMP_LOWER_CASE:
         ConvertToLC(thisitem->pattern);
      case DUMP_UPPER_CASE:
         printf("%s\n", thisitem->pattern);
         break;
      case DUMP_CASE_SENSITIVE:
      default:
         printf("%s\n", thisitem->display);
         break;
      }

      thisitem = thisitem->next;
   }
   return(0);
}

/* ========================================================================= */
int is_in_cslist(struct listitem *listtop, char *str)
{
   struct listitem *thisitem;

   /* Handle the empty list issue */
   if ( NULL == listtop )
      return(0);

   thisitem = listtop;
   while ( thisitem )
   {
      if ( 0 == strcmp(thisitem->display, str) )
         return(1);

      thisitem = thisitem->next;
   }
   return(0);
}

/* ========================================================================= */
int is_in_cilist(struct listitem *listtop, char *str)
{
   struct listitem *thisitem;

   /* Handle the empty list issue */
   if ( NULL == listtop )
      return(0);

   thisitem = listtop;
   while ( thisitem )
   {
      if ( 0 == strcmp(thisitem->pattern, str) )
         return(1);

      thisitem = thisitem->next;
   }
   return(0);
}

/* ========================================================================= */
struct listitem *cs_list_insert(struct listitem *listtop, char *str)
{
   struct listitem *li;

   if ( is_in_cslist(listtop, str) )
      return(listtop);

   /* The NULL check here is very unlikely to fail at this point - so this
      is a calculated risk with a shortcut of a solution */
   if ( NULL == ( li = (struct listitem *)malloc(sizeof(struct listitem))))
      return(listtop);

   li->display = MakeString(str);
   li->pattern = NULL; /* This is case sensitve, no need to save UC string */
   li->next = listtop;

   return(li);
}

/* ========================================================================= */
struct listitem *ci_list_insert(struct listitem *listtop, char *str)
{
   struct listitem *li;

   if ( is_in_cilist(listtop, str) )
      return(listtop);

   /* The NULL check here is very unlikely to fail at this point - so this
      is a calculated risk with a shortcut of a solution */
   if ( NULL == ( li = (struct listitem *)malloc(sizeof(struct listitem))))
      return(listtop);

   li->display = MakeString(str);
   li->pattern = MakeUCString(str);
   li->next = listtop;

   return(li);
}

/* ========================================================================= */
int dump_namelist(struct database *db, struct options *o)
{
   struct listitem *litop;
   void *thisentry;
   int count;
   char *name;
   char *platform;
   char *keylist;
   int insert_it;
   uint16_t *bitfield;
   uint16_t *entry_size;
   uint16_t *platform_size;


   /* Commands are case sensitive - All inserts need to be that way */
   litop = NULL;

   /* Set the pointer to the first entry (assuming it exists) */
   thisentry = db->dblocation + DBV2_ENTRY_START;

   count = 0;
   while(count < db->actual_entries)
   {
      entry_size = thisentry + DBV2_ENTRY_SIZE; /* Size offset is really 0 */
      insert_it = 0;
      name = get_entry_item_offset(thisentry, DBV2_ENTRY_NAME);
      platform = get_entry_item_offset(thisentry, DBV2_ENTRY_PFRM);
      keylist = get_entry_item_offset(thisentry, DBV2_ENTRY_KEYS);
      platform_size = thisentry + DBV2_ENTRY_PFRM;

      /* The entry must pick up three "ticks" to get inserted */
      if ( 0 == strcmp(o->systype, "ALL") )
         insert_it++;
      else
      {
         if ( 1 == *platform_size )
            insert_it++;
         else
         {
            if (strstr(platform, o->systype))
               insert_it++;
         }
      }

      if (o->keycons)
      {
         if(key_match(keylist, o->keycons))
            insert_it++;
      }
      else
         insert_it++;

      if ( o->iImpact != IMPACT_UNSET )
      {
         bitfield = thisentry + DBV2_ENTRY_BITF;

         if ( ( DBV2_ENTRY_IMPACT_MASK & *bitfield ) <= o->iImpact )
            insert_it++;
      }
      else
         insert_it++;

      if ( 3 == insert_it )
         litop = cs_list_insert(litop, name);

      thisentry = thisentry + *entry_size;
      count++;
   }

   /* Commands are case sensitive - Dump that way */
   return(dump_item_list(litop, DUMP_CASE_SENSITIVE));
}

/* ========================================================================= */
int dump_desclist(struct database *db, struct options *o)
{
   char tmpname[96];
   char tmpdesc[96];

   struct listitem *litop;
   void *thisentry;
   int count;
   char *name;
   char *description;
   char *platform;
   char *keylist;
   int print_it;
   uint16_t *entry_size;
   uint16_t *bitfield;
   uint16_t *platform_size;

   /* Commands are case sensitive - All inserts need to be that way */
   litop = NULL;

   /* Set the pointer to the first entry (assuming it exists) */
   thisentry = db->dblocation + DBV2_ENTRY_START;

   count = 0;
   while(count < db->actual_entries)
   {
      entry_size = thisentry + DBV2_ENTRY_SIZE; /* Size offset is really 0 */
      print_it = 0;
      name = get_entry_item_offset(thisentry, DBV2_ENTRY_NAME);
      platform = get_entry_item_offset(thisentry, DBV2_ENTRY_PFRM);
      description = get_entry_item_offset(thisentry, DBV2_ENTRY_DESC);
      keylist = get_entry_item_offset(thisentry, DBV2_ENTRY_KEYS);
      platform_size = thisentry + DBV2_ENTRY_PFRM;

      /* The entry must pick up three "ticks" to get inserted */
      if ( 0 == strcmp(o->systype, "ALL") )
         print_it++;
      else
      {
         if ( 1 == *platform_size )
            print_it++;
         else
         {
            if (strstr(platform, o->systype))
               print_it++;
         }
      }

      if (o->keycons)
      {
         if(key_match(keylist, o->keycons))
            print_it++;
      }
      else
         print_it++;

      if ( o->iImpact != IMPACT_UNSET )
      {
         bitfield = thisentry + DBV2_ENTRY_BITF;

         if ( ( DBV2_ENTRY_IMPACT_MASK & *bitfield ) <= o->iImpact )
            print_it++;
      }
      else
         print_it++;

      if ( 3 == print_it )
      {
         strcpy(tmpname, name);
         strcpy(tmpdesc, description);
#define TRUNCATE_DESC_OUTPUT
#ifdef TRUNCATE_DESC_OUTPUT
         if ( 14 < strlen(tmpname) ) { tmpname[13] = '!' ; tmpname[14] = 0 ; }
         if ( 64 < strlen(tmpdesc) ) { tmpdesc[63] = '!' ; tmpdesc[64] = 0 ; }
#endif
         printf("%-14s %-64s\n", tmpname, tmpdesc);
      }

      thisentry = thisentry + *entry_size;
      count++;
   }

   return(0);
}

/* ========================================================================= */
int dump_keyslist(struct database *db, struct options *o)
{
   char akey[32];
   struct listitem *litop;
   void *thisentry;
   int count;
   char *keys;
   char *platform;
   int i, j;
   uint16_t *entry_size;

   /* Keys are case Insensitive - All inserts need to be that way */
   litop = NULL;

   /* Set the pointer to the first entry (assuming it exists) */
   thisentry = db->dblocation + DBV2_ENTRY_START;

   count = 0;
   while(count < db->actual_entries)
   {
      entry_size = thisentry + DBV2_ENTRY_SIZE; /* Size offset is really 0 */
      keys = get_entry_item_offset(thisentry, DBV2_ENTRY_KEYS);
      platform = get_entry_item_offset(thisentry, DBV2_ENTRY_PFRM);

      /* If the systype (of the query) is in the entries platform, then insert */
      if ((strstr(platform, o->systype)) || ( 0 == strstr(o->systype, "ALL")))
      {
         i = 0;
         while(keys[i] != 0)
         {
            j = 0;
            while((keys[i] != 0) && (keys[i] != ' '))
            {
               akey[j++] = keys[i++];
            }
            akey[j] = 0;

            /* Move off the space */
            while(keys[i] == ' ')
               i++;

            ConvertToUC(akey); /* This should already be UC. Convert anyways */

            litop = ci_list_insert(litop, akey);
         }
      }

      thisentry = thisentry + *entry_size;
      count++;
   }

   /* Commands are case sensitive - Dump that way */
   return(dump_item_list(litop, DUMP_LOWER_CASE));
}

/* ========================================================================= */
int dump_plfmlist(struct database *db, struct options *o)
{
   char aplatform[32];
   struct listitem *litop;
   void *thisentry;
   int count;
   char *platform;
   int i, j;
   uint16_t *entry_size;

   /* Platforms are case Insensitive - All inserts need to be that way */
   litop = NULL;

   /* Set the pointer to the first entry (assuming it exists) */
   thisentry = db->dblocation + DBV2_ENTRY_START;

   count = 0;
   while(count < db->actual_entries)
   {
      entry_size = thisentry + DBV2_ENTRY_SIZE; /* Size offset is really 0 */
      platform = get_entry_item_offset(thisentry, DBV2_ENTRY_PFRM);

      i = 0;
      while(platform[i] != 0)
      {
         j = 0;
         while((platform[i] != 0) && (platform[i] != ' '))
         {
            aplatform[j++] = platform[i++];
         }
         aplatform[j] = 0;

         /* Move off the space */
         while(platform[i] == ' ')
            i++;

         ConvertToUC(aplatform);

         litop = ci_list_insert(litop, aplatform);
      }

      thisentry = thisentry + *entry_size;
      count++;
   }

   /* Commands are case sensitive - Dump that way */
   return(dump_item_list(litop, DUMP_LOWER_CASE));
}

/* ========================================================================= */
/* A list query can be of three types: name, platform, or keyword */
/* A list query can be limited by platform */
int handle_list_query(struct database *db, struct options *o)
{
   if ( o->qList == LIST_NAME )
      return(dump_namelist(db, o));

   if ( o->qList == LIST_DESC )
      return(dump_desclist(db, o));

   if ( o->qList == LIST_KEYS )
      return(dump_keyslist(db, o));
   
   if ( o->qList == LIST_PLFM )
      return(dump_plfmlist(db, o));

   return(0);
}

/* ========================================================================= */
int handle_tool_dump(struct database *db, struct options *o)
{
   void *thisentry;
   char *name;
   char *platform;
   int count;
   char impact[16];

   uint16_t *entry_size;
   uint16_t *bitfield;
   uint16_t *platform_size;

   /* Set the pointer to the first entry (assuming it exists) */
   thisentry = db->dblocation + DBV2_ENTRY_START;

   count = 0;
   while(count < db->actual_entries)
   {
      entry_size = thisentry + DBV2_ENTRY_SIZE; /* Size offset is really 0 */

      name = get_entry_item_offset(thisentry, DBV2_ENTRY_NAME);
      platform = get_entry_item_offset(thisentry, DBV2_ENTRY_PFRM);
      platform_size = thisentry + DBV2_ENTRY_PFRM;

      if ( 0 == strcmp(name, o->tooldump) )
      {
         /* This *could* be a match - now check palatform */
         if ((0 == *platform_size) || (NULL != strstr(platform, o->systype)))
         {
            printf("%s %s\n", NAME_LINE_KEY, name);
            printf("%s %s\n", DESC_LINE_KEY,
                   get_entry_item_offset(thisentry, DBV2_ENTRY_DESC));
            printf("%s %s\n", LOCN_LINE_KEY,
                   get_entry_item_offset(thisentry, DBV2_ENTRY_LOCN));
            printf("%s %s\n", OWNR_LINE_KEY,
                   get_entry_item_offset(thisentry, DBV2_ENTRY_OWNR));
            printf("%s %s\n", PFRM_LINE_KEY,
                   get_entry_item_offset(thisentry, DBV2_ENTRY_PFRM));
            printf("%s %s\n", KEYS_LINE_KEY,
                   get_entry_item_offset(thisentry, DBV2_ENTRY_KEYS));
            bitfield = thisentry + DBV2_ENTRY_BITF;
            printf("%s %s\n", IMPT_LINE_KEY,
                   parse_impact_value(impact, *bitfield));

            return(0);
         }
      }

      thisentry = thisentry + *entry_size;
      count++;
   }

   return(1);
}

/* ========================================================================= */
int HandleQueries(struct database *db, struct options *o)
{
   assert(NULL != db);
   assert(NULL != o);

   /* No queries while compiling */
   if (o->bCompile)
      return(0);

   if (o->qList)
      return(handle_list_query(db, o));

   if (o->tooldump)
      return(handle_tool_dump(db, o));


   return(0);
}
