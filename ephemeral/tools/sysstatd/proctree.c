#include "proctree.h"
#include "coredata.h"
#include "dyntree.h"

#ifdef PORT_Linux
#include "Linux/osspecific.h"
#endif

#ifdef PORT_AIX
#include "AIX/osspecific.h"
#endif

#ifdef PORT_SunOS
#include "SunOS/osspecific.h"
#endif

int render_json_proc_tree(struct iobuf *ob);
int render_yaml_proc_tree(struct iobuf *ob);

#ifdef STUB_MOVED_ELSEWHERE
int indent_json(struct iobuf *ob, int indent);
#endif

/* ========================================================================= */
int DynamicProcTree(struct iobuf *ob, int format)
{
   /* This is just a giant switch director.... The alternative is to handle
      all file formats in one location. I think re-implementing the get
      data part (retrieving the process data) is easier because it is already
      wrapped with code that removes the OS implemetation details and makes
      the retrieval easy.
   */
   switch ( format )
   {
   case FORMAT_JSON:
      return(render_json_proc_tree(ob));
      break;
   case FORMAT_YAML:
      return(render_yaml_proc_tree(ob));
      break;
   }

   /* STUB: Handle this better */
   return(1);
}

/* ========================================================================= */
int render_json_proc_tree(struct iobuf *ob)
{
   char ts[64];
   struct procitem p;
   unsigned long cntr;

   /* Setup process tree */
   InitProcessEntry(&p);

   StartProcessEntry(&p);

   ResetIOBuf(ob);

   /* Grab the timestamp now */
   GetTimestamp(ts, 64);

   /* Drop the opneing curly braces */
   IOBufWrite(ob, "{\n");

   indent_json(ob, 1);
   IOBufWrite(ob, "\"TimeStamp\": \"%s\",\n", ts);

   indent_json(ob, 2);
   IOBufWrite(ob, "\"ProcessList\": [");

   cntr = 0;
   while ( GetProcessEntry(&p) )
   {
      /* This is really about terminating last item */
      if ( cntr > 0 )
         IOBufWrite(ob, ",\n");
      else
         IOBufWrite(ob, "\n");

      indent_json(ob, 3);
      IOBufWrite(ob, "{\n");

      indent_json(ob, 4);
      IOBufWrite(ob, "\"pid\": %ld,\n", p.pid);

      indent_json(ob, 4);
      IOBufWrite(ob, "\"pname\": \"%s\",\n", p.pname);

      indent_json(ob, 4);
      IOBufWrite(ob, "\"stime\": %lld,\n", (long long)p.stime);

      indent_json(ob, 4);
      IOBufWrite(ob, "\"SizeKB\": %llu,\n", p.K_size);

      indent_json(ob, 4);
      IOBufWrite(ob, "\"RSSKB\": %llu\n", p.K_rss); /* Last item, no comma */

      indent_json(ob, 3);
      IOBufWrite(ob, "}");

      cntr++;
   }
   
   IOBufWrite(ob, "\n"); /* LF from the last item */

   /* End it all */
   indent_json(ob, 2);
   IOBufWrite(ob, "],\n");

   indent_json(ob, 1);
   IOBufWrite(ob, "\"ProcessCount\": %lu\n}\n", cntr);
         
   return(0);
}

/* ========================================================================= */
int render_yaml_proc_tree(struct iobuf *ob)
{
   char ts[64];
   struct procitem p;
   unsigned long cntr;

   /* Setup process tree */
   InitProcessEntry(&p);

   StartProcessEntry(&p);

   ResetIOBuf(ob);

   /* Grab the timestamp now */
   GetTimestamp(ts, 64);

   /* Drop the opneing curly braces */
   IOBufWrite(ob, "TimeStamp: %s\n", ts);

   cntr = 0;
   while ( GetProcessEntry(&p) )
   {
      IOBufWrite(ob, "Process: %ld\n", p.pid);

      indent_yaml(ob, 1);
      IOBufWrite(ob, "pname: %s\n", p.pname);

      indent_yaml(ob, 1);
      IOBufWrite(ob, "stime: %lld\n", (long long)p.stime);

      indent_yaml(ob, 1);
      IOBufWrite(ob, "SizeKB: %llu\n", p.K_size);

      indent_yaml(ob, 1);
      IOBufWrite(ob, "RSSKB: %llu\n", p.K_rss);

      cntr++;
   }
   
   IOBufWrite(ob, "ProcessCount: %lu\n", cntr);
         
   return(0);
}
