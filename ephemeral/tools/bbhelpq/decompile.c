#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "decompile.h"
#include "query.h"
#include "filefmt.h"

/* ========================================================================= */
int dump_each_file(struct database *db, struct options *o)
{
   FILE *f;

   int count;
   void *thisentry;

   char *filename;
   char *name;
   char *description;
   char *platform;
   char *keylist;
   char *location;
   char *owner;
   char impact[8];

   uint16_t *entry_size;
   uint16_t *bitfield;
   uint16_t *platform_size;



   /* Set the pointer to the first entry (assuming it exists) */
   thisentry = db->dblocation + DBV2_ENTRY_START;

   printf("De-compiling the database to base (source) files.\n");
   count = 0;
   while(count < db->actual_entries)
   {
      entry_size = thisentry + DBV2_ENTRY_SIZE; /* Size offset is really 0 */


      filename = GetEntryItemOffset(thisentry, DBV2_ENTRY_FILE);
      name = GetEntryItemOffset(thisentry, DBV2_ENTRY_NAME);
      description = GetEntryItemOffset(thisentry, DBV2_ENTRY_DESC);
      keylist = GetEntryItemOffset(thisentry, DBV2_ENTRY_KEYS);
      location = GetEntryItemOffset(thisentry, DBV2_ENTRY_LOCN);
      platform = GetEntryItemOffset(thisentry, DBV2_ENTRY_PFRM);
      platform_size = thisentry + DBV2_ENTRY_PFRM;
      owner = GetEntryItemOffset(thisentry, DBV2_ENTRY_OWNR);
      bitfield = thisentry + DBV2_ENTRY_BITF;

      switch ( *bitfield & DBV2_ENTRY_IMPACT_MASK )
      {
      case DBV2_ENTRY_IMPACT_NONE:
         strcpy(impact, "None");
         break;
      case DBV2_ENTRY_IMPACT_LOWI:
         strcpy(impact, "Low");
         break;
      case DBV2_ENTRY_IMPACT_MEDI:
         strcpy(impact, "Medium");
         break;
      case DBV2_ENTRY_IMPACT_HIGH:
         strcpy(impact, "High");
         break;
      default:
         impact[0] = 0;
         break;
      }

      printf("  %s...", filename);
      if (NULL != (f = fopen(filename, "w")))
      {
         fprintf(f, "%s %s\n", NAME_LINE_KEY, name);
         fprintf(f, "%s %s\n", DESC_LINE_KEY, description);
         fprintf(f, "%s %s\n", KEYS_LINE_KEY, keylist);
         fprintf(f, "%s %s\n", LOCN_LINE_KEY, location);
         if ( 1 == *platform_size )
            fprintf(f, "%s ALL\n", PFRM_LINE_KEY);
         else
            fprintf(f, "%s %s\n", PFRM_LINE_KEY, platform);
         fprintf(f, "%s %s\n", IMPT_LINE_KEY, impact);
         fprintf(f, "%s %s\n", OWNR_LINE_KEY, owner);
         fclose(f);
         printf("....Done.\n");
         fflush(stdout);
      }
      else
      {
         printf("Failed.\n");
         fflush(stdout);
      }

      thisentry = thisentry + *entry_size;
      count++;
   }

   return(0);
}







int DeCompileDB(struct database *db, struct options *o)
{
   if ( NULL == db )
      return(0);

   if ( NULL == o )
      return(0);


   if ( 0 == o->bDecompile )
      return(0);

   dump_each_file(db, o);

   return(0);
}
