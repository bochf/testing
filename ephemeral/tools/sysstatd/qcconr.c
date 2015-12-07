
#include <stdio.h>

#include "qcconr.h"
#include "dyntree.h"
#include "defaults.h"
#include "support.h"

int render_json_qc_tree(struct iobuf *ob, struct query *q, struct options *o);
int render_yaml_qc_tree(struct iobuf *ob, struct query *q, struct options *o);

#define QC_NOTALINE   0   /* Undefined line / no line retrieved / error      */
#define QC_NAMEVALUE  1   /* LABEL: string                                   */
#define QC_TESTRESULT 2   /* nnn:string:n:XXXX[:String]                      */

#define QC_MAX_LABEL_SIZE 32
#define QC_MAX_DESC_SIZE  128
#define QC_MAX_TSTSTR_SIZE 8
#define QC_MAX_RESULT_SIZE 128
#define QC_MAX_LINE_SIZE 256

struct qc_conr_entry
{
   int type;
   char label[QC_MAX_LABEL_SIZE];
   char description[QC_MAX_DESC_SIZE];
   int  testrv;
   char testrvstr[QC_MAX_TSTSTR_SIZE];
   char resultstr[QC_MAX_RESULT_SIZE];
};

FILE *open_qc_conr_results(struct options *o, int version);
int close_qc_conr_results(FILE *qcf);
int get_qc_conr_entry(struct qc_conr_entry *qce, FILE *qcf);
/* The following two are not used */
int is_a_test_line(char *line);
int is_a_value_line(char *line);
/* The following is used */
int line_type(char *line);

int parse_namevalue_line(struct qc_conr_entry *qce, char *line);
int parse_test_line(struct qc_conr_entry *qce, char *line);

/* ========================================================================= */
int DynamicQCTree(struct iobuf *ob, struct query *q, struct options *o)
{
   /* This is just a giant switch director.... The alternative is to handle
      all file formats in one location. I think re-implementing the get
      data part (retrieving the process data) is easier because it is already
      wrapped with code that removes the OS implemetation details and makes
      the retrieval easy.
   */
   switch ( q->format )
   {
   case FORMAT_JSON:
      return(render_json_qc_tree(ob, q, o));
      break;
   case FORMAT_YAML:
      return(render_yaml_qc_tree(ob, q, o));
      break;
   }

   /* STUB: Handle this better */
   return(1);
}

/* ========================================================================= */
int render_json_qc_tree(struct iobuf *ob, struct query *q, struct options *o)
{
   FILE *qcf;
   struct qc_conr_entry qce;
   int first_item = 1;
   char day[8];
   int iDay;

   ResetIOBuf(ob);

   iDay = 0;
   if ( GetLabelValue(day, 8, q, "day") )
   {
      /* OK. Admittedly, this is kind of hokey to parse a string, get
         a number then convert back to a string. But, we can error check
         in the process. */

      if ( is_pnumeric(day) )
         iDay = atoi(day);
   }

   if ( NULL == (qcf = open_qc_conr_results(o, iDay)) )
   {
      ob->error = IOBUF_ERR_NOFILE;
      return(1);
   }

   /* Drop the opneing curly braces */
   IOBufWrite(ob, "{\n");
  
   while ( get_qc_conr_entry(&qce, qcf) )
   {
      if ( 0 == first_item )
         IOBufWrite(ob, ",\n");

      first_item = 0; /* This is no longer the first item in the output */

      if ( qce.type == QC_NAMEVALUE )
      {
         indent_json(ob, 1);

         if ( is_numeric(qce.description) )
            IOBufWrite(ob, "\"%s\": %s", qce.label, qce.description);
         else
            IOBufWrite(ob, "\"%s\": \"%s\"", qce.label, qce.description);
      }

      if ( qce.type == QC_TESTRESULT )
      {
         indent_json(ob, 1);
         IOBufWrite(ob, "\"QCTest\": {\n");

         indent_json(ob, 2);
         IOBufWrite(ob, "\"TestNumber\": %s,\n", qce.label);

         indent_json(ob, 2);
         IOBufWrite(ob, "\"TestDesc\": \"%s\",\n", qce.description);

         indent_json(ob, 2);
         IOBufWrite(ob, "\"TestRV\": \"%d\",\n", qce.testrv);

         indent_json(ob, 2);
         IOBufWrite(ob, "\"TestRVStr\": \"%s\",\n", qce.testrvstr);
         
         indent_json(ob, 2);
         IOBufWrite(ob, "\"TestRMsg\": \"%s\",\n", qce.resultstr);
         
         indent_json(ob, 1);
         IOBufWrite(ob, "}");
      }
   }

   close_qc_conr_results(qcf);

   /* Drop the closing curly braces */
   IOBufWrite(ob, "\n}\n");

   return(0);
}

/* ========================================================================= */
int render_yaml_qc_tree(struct iobuf *ob, struct query *q, struct options *o)
{
   FILE *qcf;
   struct qc_conr_entry qce;
   char day[8];
   int iDay;

   ResetIOBuf(ob);

   iDay = 0;
   if ( GetLabelValue(day, 8, q, "day") )
   {
      /* See notes on similar code section in render_json_qc_tree() above */

      if ( is_pnumeric(day) )
         iDay = atoi(day);
   }

   if ( NULL == (qcf = open_qc_conr_results(o, iDay)) )
   {
      ob->error = IOBUF_ERR_NOFILE;
      return(1);
   }

   while ( get_qc_conr_entry(&qce, qcf) )
   {
      if ( qce.type == QC_NAMEVALUE )
      {
         IOBufWrite(ob, "%s: %s\n", qce.label, qce.description);
      }

      if ( qce.type == QC_TESTRESULT )
      {
         IOBufWrite(ob, "QCTest: %s\n", qce.label);

         indent_yaml(ob, 1);
         IOBufWrite(ob, "TestDesc: %s\n", qce.description);

         indent_yaml(ob, 1);
         IOBufWrite(ob, "TestRV: %d\n", qce.testrv);

         indent_yaml(ob, 1);
         IOBufWrite(ob, "TestRVStr: %s\n", qce.testrvstr);
         
         indent_yaml(ob, 1);
         IOBufWrite(ob, "TestRMsg: %s\n", qce.resultstr);
         
         IOBufWrite(ob, "\n");
      }
   }

   close_qc_conr_results(qcf);

   return(0);
}

/* ========================================================================= */
FILE *open_qc_conr_results(struct options *o, int version)
{
   FILE *qcf;
   char filename[256];

   filename[0] = 0;

   if ( version > 0 )
   {
      sprintf(filename, "%s/%s.%d",
              o->cf_qcfiledir,
              QC_CONR_BASENAME,
              version);
   }
   else
   {
      sprintf(filename, "%s/%s",
              o->cf_qcfiledir,
              QC_CONR_BASENAME);
   }

   if ( NULL == (qcf = fopen(filename, "r")) )
   {
      /* NOTE: This will be the first of two errors. The daemon will error
               again when it sends the 404 error. I am going to let the two
               errors stand, even though they are a bit redundant. I am doing
               this because:
               1. This message is specific
               2. The general "Not Found" error may be used for other trees. */
      error_msg("ERROR: Unable to open qc_conr_results file.");
      return(NULL);
   }


   return(qcf);
}

/* ========================================================================= */
int close_qc_conr_results(FILE *qcf)
{
   return(fclose(qcf));
}

/* ========================================================================= */
int get_qc_conr_entry(struct qc_conr_entry *qce, FILE *qcf)
{
   char line[QC_MAX_LINE_SIZE];
   int parse_results = 0;

   if ( NULL == qcf)
      return(0);

   while ( fgets(line, QC_MAX_LINE_SIZE, qcf) )
   {
      switch( line_type(line) )
      {
      case QC_NAMEVALUE:
         parse_results = parse_namevalue_line(qce, line);
         break;
      case QC_TESTRESULT:
         parse_results = parse_test_line(qce, line);
         break;
      case QC_NOTALINE:
      default:
         parse_results = 0;
         break;
      }

      /* If we got a good line, then return instead of going back
         to the well. */
      if ( parse_results )
         return(1);
   }

   return(0);
}

/* ========================================================================= */
int parse_namevalue_line(struct qc_conr_entry *qce, char *line)
{
   int coloncnt = 0;
   int i;
   int j;

   qce->type = QC_NOTALINE;
   qce->label[0] = 0;
   qce->description[0] = 0;
   qce->testrv = 0;
   qce->testrvstr[0] = 0;
   qce->resultstr[0] = 0;

   i = 0;
   j = 0;
   while ( line[i] != 0 )
   {
      if ( coloncnt == 0 )
      {
         /* We are working on the label */

         if ( j == QC_MAX_LABEL_SIZE )
            return(0);


         if ( line[i] == ':' )
         {
            qce->label[j] = 0;
            coloncnt++;
            j = 0;

            /* Step off the ':' */
            while ( line[i] == ':' )
               i++;

            /* Skip through leading whitespace */
            while (( line[i] == ' ' ) || ( line[i] == '\t' ))
               i++;
         }
         else
         {
            qce->label[j++] = line[i++];
         }
      }
      else
      {
         /* We are working on the value */
         if ( j == QC_MAX_DESC_SIZE )
            return(0);

         if ( line[i] == '\n' )
         {
            qce->description[j] = 0;
            qce->type = QC_NAMEVALUE;
            return(1);
         }
         else
         {
            qce->description[j++] = line[i++];
         }

      }
   }

   return(0);
}

/* ========================================================================= */
int parse_test_line(struct qc_conr_entry *qce, char *line)
{
   int coloncnt = 0;
   int i;
   int j;
   char rv[8];

   qce->type = QC_NOTALINE;
   qce->label[0] = 0;
   qce->description[0] = 0;
   qce->testrv = 0;
   qce->testrvstr[0] = 0;
   qce->resultstr[0] = 0;

   i = 0;
   j = 0;
   while ( line[i] != 0 ) /* STUB: Bounds and term check */
   {
      if ( line[i] == ':' )
      {
         switch ( coloncnt )
         {
         case 0:
            qce->label[j] = 0;
            break;
         case 1:
            qce->description[j] = 0;
            break;
         case 2:
            rv[j] = 0;
            if ( is_numeric(rv) )
               qce->testrv = atoi(rv);
            break;
         case 3:
            qce->testrvstr[j] = 0;
            break;
         case 4:
            /* This means that there are five! */
            return(0);
            break;
         }

         coloncnt++; /* We hit another colon         */
         j = 0;      /* Reset j for the next item    */
         i++;        /* Move the index off the colon */
      }
      else
      {
         switch ( coloncnt )
         {
         case 0:
            /* Bounds check */
            if ( j == QC_MAX_LABEL_SIZE )
               return(0);

            qce->label[j++] = line[i++];
            break;
         case 1:
            /* Bounds check */
            if ( j == QC_MAX_DESC_SIZE )
               return(0);

            qce->description[j++] = line[i++];
            break;
         case 2:
            /* Bounds check */
            if ( j == 8 )
               return(0);

            rv[j++] = line[i++];
            break;
         case 3:
            /* Bounds check */
            if ( j ==  QC_MAX_TSTSTR_SIZE )
               return(0);

            if ( line[i] != '\n' )
               qce->testrvstr[j++] = line[i++];
            else
               i++;

            break;
         case 4:
            /* Bounds check */
            if ( j ==  QC_MAX_RESULT_SIZE )
               return(0);

            if ( line[i] != '\n' )
               qce->resultstr[j++] = line[i++];
            else
               i++;

            break;
         }
      }
   }

   /* Where were we when we hit the EOL? */
   switch ( coloncnt )
   {
   case 4:
      /* We were working on the results str when we stopped */
      qce->resultstr[j] = 0;
      break;
   case 3:
      /* We were working on the RV string */
      qce->testrvstr[j] = 0;
      break;
   default:
      return(0);
   }

   qce->type = QC_TESTRESULT;

   return(1);
}

/* ========================================================================= */
int line_type(char *line)
{
   int numeric = 0;
   int coloncnt = 0;
   int lablen = 0;
   char *savline = line;

   while(*line != 0)
   {
      if ( coloncnt == 0 )
      {
         if (( *line >= '0' ) && ( *line <= '9' ))
            numeric++;

         lablen++;
      }
         
      if ( *line == ':' )
         coloncnt++;

      line++;
   }

   /* This defines a valid test line */
   if (( coloncnt >= 3 ) && ( coloncnt < 5 ))
   {
      if ( numeric == 3 )
      {
         return(QC_TESTRESULT);
      }

      if (( savline[0] == 'E' ) &&
          ( savline[1] == 'N' ) &&
          ( savline[2] == 'D' ))
      {
         return(QC_TESTRESULT);
      }
   }

   /* This defines a valid label-value line */
   if (( coloncnt == 1 ) && ( lablen >= 1 ))
      return(QC_NAMEVALUE);
   

   return(QC_NOTALINE);
}

/* ========================================================================= */
int is_a_test_line(char *line)
{
   int numeric = 0;
   int coloncnt = 0;

   while(*line != 0)
   {
      if ( (( *line >= '0' ) && ( *line <= '9' )) && ( coloncnt == 0 ) )
         numeric++;
         
      if ( *line == ':' )
         coloncnt++;

      line++;
   }

   if (( coloncnt >= 3 ) && ( numeric == 3 ))
      return(1);

   return(0);
}

/* ========================================================================= */
int is_a_value_line(char *line)
{
   int lablen = 0;
   int coloncnt = 0;

   while(*line != 0)
   {
      if ( coloncnt == 0 )
         lablen++;
         
      if ( *line == ':' )
         coloncnt++;

      line++;
   }

   if (( coloncnt == 1 ) && ( lablen >= 1 ))
      return(1);

   return(0);
}
