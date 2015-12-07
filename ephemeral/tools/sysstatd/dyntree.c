

#include "dyntree.h"
#include "defaults.h"

/* The fmtr files are used to define indent count and char */
#include "jsonfmtr.h"
#include "yamlfmtr.h"

/* ========================================================================= */
int indent_json(struct iobuf *ob, int indent)
{
   int i;

   indent *= JSON_INDENT_SIZE;

   i = 0;
   while ( i < indent )
   {
      IOBufWrite(ob, "%c", JSON_INDENT_CHAR);
      i++;
   }

   return(indent);
}

/* ========================================================================= */
int indent_yaml(struct iobuf *ob, int indent)
{
   int i;

   indent *= YAML_INDENT_SIZE;

   i = 0;
   while ( i < indent )
   {
      IOBufWrite(ob, "%c", YAML_INDENT_CHAR);
      i++;
   }

   return(indent);
}
