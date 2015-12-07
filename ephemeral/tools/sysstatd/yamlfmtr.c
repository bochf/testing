#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "yamlfmtr.h"

int data_to_buf_yaml(struct iobuf *ob, struct dataitem *dt, int indent);

/* ========================================================================= */
/* Don't recurse here - make a sub function reentrant / recursive */
int DataToBufYAML(struct iobuf *ob, struct dataitem *dt)
{
   /* Reset this - tabla rasa */
   ob->eod = 0;

   assert(NULL != ob);
   assert(NULL != dt);

   /* Walk the tree */
   data_to_buf_yaml(ob, dt, 0);

   return(0);
}

/* ========================================================================= */
/* Recurse here */
#define MAX_YAML_DATA_SIZE 1024
int data_to_buf_yaml(struct iobuf *ob, struct dataitem *dt, int indent)
{
   char data[MAX_YAML_DATA_SIZE];
   int i;

   while(dt)
   {
      if ( DI_TYPE_BRANCH == dt->type )
      {
         i = 0;
         while ( i < indent )
         {
            IOBufWrite(ob, "%c", YAML_INDENT_CHAR);
            i++;
         }

         data[0] = 0;
         if ( dt->datasrc )
            dt->datasrc(data, MAX_YAML_DATA_SIZE);

         IOBufWrite(ob, "%s: %s\n", dt->label, data);

         data_to_buf_yaml(ob, dt->sublist, indent + YAML_INDENT_SIZE);
      }
      else
      {
         i = 0;
         while ( i < indent )
         {
            IOBufWrite(ob, "%c", YAML_INDENT_CHAR);
            i++;
         }
            
         /* NOTE: This code is not my ideal of how this should work.
                  See the note block (in the same location) in the
                  json function data_to_buf_json() for details.
         */
         data[0] = 0; /* Alternative to checking return value */
         dt->datasrc(data, MAX_YAML_DATA_SIZE);
         IOBufWrite(ob, "%s: %s\n", dt->label, data);
      }

      dt = dt->next;
   } /* while(dt) ---> no else means terminating condition */

   return(0);
}









