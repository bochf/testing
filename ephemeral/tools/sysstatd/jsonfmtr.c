#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "jsonfmtr.h"
#include "support.h"

int data_to_buf_json(struct iobuf *ob, struct dataitem *dt, int indent);

/* ========================================================================= */
/* Don't recurse here - make a sub function reentrant / recursive */
int DataToBufJSON(struct iobuf *ob, struct dataitem *dt)
{
   assert(NULL != ob);
   assert(NULL != dt);

   /* Reset buffer - tabla rasa */
   ResetIOBuf(ob);

   /* Drop the opneing curly braces */
   IOBufWrite(ob, "{\n");

   /* Walk the tree */
   data_to_buf_json(ob, dt, JSON_INDENT_SIZE);

   /* Drop the terminating curly braces */
   IOBufWrite(ob, "}\n");

   return(0);
}


/* ========================================================================= */
#define MAX_JSON_DATA_SIZE 1024
int data_to_buf_json(struct iobuf *ob, struct dataitem *dt, int indent)
{
   char data[MAX_JSON_DATA_SIZE];
   int i;

   while(dt)
   {
      if ( DI_TYPE_BRANCH == dt->type )
      {
         i = 0;
         while ( i < indent )
         {
            IOBufWrite(ob, "%c", JSON_INDENT_CHAR);
            i++;
         }

         if ( dt->datasrc )
         {
            data[0] = 0;
            dt->datasrc(data, MAX_JSON_DATA_SIZE);

            /* Don't try to write data if it is empty */
            if ( data[0] == 0 )
               IOBufWrite(ob, "\"%s\": {\n", dt->label);
            else
               IOBufWrite(ob, "\"%s\": \"%s\" {\n", dt->label, data);
         }
         else
         {
            IOBufWrite(ob, "\"%s\": {\n", dt->label);
         }
         
         data_to_buf_json(ob, dt->sublist, indent + JSON_INDENT_SIZE);

         i = 0;
         while ( i < ( indent + JSON_INDENT_SIZE ) )
         {
            IOBufWrite(ob, "%c", JSON_INDENT_CHAR);
            i++;
         }

         if ( dt->next )
            IOBufWrite(ob, "},\n");
         else
            IOBufWrite(ob, "}\n");
      }
      else
      {
         i = 0;
         while ( i < indent )
         {
            IOBufWrite(ob, "%c", JSON_INDENT_CHAR);
            i++;
         }

         /* NOTE: Here is the explanation and reasoning for the 
                  following code:
                  - We copy the results into the local "data" buffer.
                    We copy that into the iobuf buffer.
                  - The alternative to this is to do three writes
                    (two via IOBufWrite() and one directly to the
                    iobuf.
                  - Neither makes me happy, and some happy alternative
                    resides on the other end of some engineering
                    effort. But I am keeping this for now as long as
                    my discomfort with the solution is noted here.
         */

         data[0] = 0; /* Alternative to checking return value */
         dt->datasrc(data, MAX_JSON_DATA_SIZE);

         if ( is_numeric(data) )
         {
            if ( dt->next )
               IOBufWrite(ob, "\"%s\": %s,\n", dt->label, data);
            else
               IOBufWrite(ob, "\"%s\": %s\n", dt->label, data);
         }
         else
         {
            if ( dt->next )
               IOBufWrite(ob, "\"%s\": \"%s\",\n", dt->label, data);
            else
               IOBufWrite(ob, "\"%s\": \"%s\"\n", dt->label, data);
         }
            
      }

      dt = dt->next;
   }

   return(0);
}




