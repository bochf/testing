#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "options.h"

/* Internal defines - Used for "item" in read_next_* functions */
#define RN_COLUMNS 1
#define RN_SMTCOL  2
#define RN_XDATA   4
#define RN_BLIST   8

/* next item is - Used to track were we are when walking through the -b list */
#define ITEM_END      0
#define ITEM_NUMBER   1
#define ITEM_COMMA    2
#define ITEM_DASH     3
#define ITEM_BOLD     4
#define ITEM_ERROR    5
#define ITEM_BEGIN    6
#define ITEM_HANDLED  7

/* Internal prototypes - Input descriptions with functions. */
void dump_options(struct options *o);
int read_next_char(struct options *o, int item, char *argv);
int read_next_argv(struct options *o, int item, char *argv);
struct options *init_options(void);
int is_number(char *cp);
unsigned long parse_xdata(char *a);

/* Just for the blist parsing */
struct boldll *parse_blist(char *blist);
struct boldll *append_number_range(struct boldll *bl, int begin_num, int end_num);
struct boldll *append_single_number(struct boldll *bl, int number);
int next_item_is(char *cp);
int read_next_number(int *dest, char *cp);

/* ========================================================================= */
struct options *ParseOptions(int argc, char *argv[])
{
  struct options *o;
  int a;      /* Counter for the outer loop. */
  int i;      /* Counter for inner / string walking loop */
  int nexta;

  /* Set up the structure with default values */
  if(NULL == (o = init_options()))
    return(NULL);


  a = 1;
  while(a < argc)
  {
     nexta = a + 1; /* A will, by default, incrament by one */

     if(argv[a][0] == '-')
     {
        i = 1;
        while(argv[a][i] != 0)
        {
           switch(argv[a][i])
           {
           case '+':
              o->debug = 1;
              break;
           case 'a':
              o->column_output = O_COLS_ALL;
              break;
           case 'B':
              o->bold_on = 1;
              break;
           case 'b':
              if (argv[a][i + 1] != 0)
                 i += read_next_char(o, RN_BLIST, argv[a] + (i + 1));
              else
                 nexta += read_next_argv(o, RN_BLIST, argv[a + 1]);
              break;
           case 'c':
              if (argv[a][i + 1] != 0)
                 i += read_next_char(o, RN_COLUMNS, argv[a] + (i + 1));
              else
                 nexta += read_next_argv(o, RN_COLUMNS, argv[a + 1]);
              break;
           case 'd':
              o->color_output = O_COLOR_SIMPLE;
              break;
           case 'e':
              o->column_output = O_COLS_EXTENDED;
              break;
           case 'f':
              o->color_output = O_COLOR_FRUITY;
              break;
           case 'h':
              o->show_help = 1;
              break;
           case 'j':
              o->color_output = O_COLOR_MANY;
              break;
           case 'm':
              o->color_output = O_COLOR_NONE;
              break;
           case 's':
              o->smt_user_set = 1;
              if (argv[a][i + 1] != 0)
                 i += read_next_char(o, RN_SMTCOL, argv[a] + (i + 1));
              else
                 nexta += read_next_argv(o, RN_SMTCOL, argv[a + 1]);
              break;
           case 't':
              o->show_timestamp = 1;
              break;
           case 'T':
              o->show_totals = O_TOTALS_ALL;
              break;
           case 'v':
              o->show_version = 1;
              break;
           default:
              o->input_error = 1;
           }
           i++;

           /* Bust out and go to the next if nexta got incramented. */
           if (nexta != a + 1)
              break;
        }
        
     }
     else /* This item does not begin with '-' */
     {
        if(is_number(argv[a])) /* See if the param is a number */
        {
           if(o->it_set) /* If this is second+ stand alone number then error */
              o->input_error = 1;
           else
           {
              o->it_set = 1;
              o->iteration_time = atoi(argv[a]);
           }
        }
        else
           o->input_error = 1;
     } /* if ( argv[][0] == '-' ) else */
     a = nexta;
  } /* while (a < argc) */

  if(o->blist)
  {
     o->bl = parse_blist(o->blist);
  }

  if(o->debug)
    dump_options(o);

  return(o);
}

/* ========================================================================= */
void dump_options(struct options *o)
{
   struct boldll *bl;

   if(NULL == o)
   {
      printf("struct options *NULL\n");
      return;
   }

   printf("struct options\n{\n");
   printf("   int color_output = %d;\n", o->color_output);
   printf("   int column_output = %d;\n", o->column_output);
   printf("   int show_help = %d;\n", o->show_help);
   printf("   int show_version = %d;\n", o->show_version);
   printf("   int columns = %d;\n", o->columns);
   printf("   int smt_columns = %d;\n", o->smt_columns);
   /* printf("   int boldopts = %d;\n", o->boldopts); */
   if(o->blist)
      printf("   char blist = \"%s\";\n", o->blist);
   else
      printf("   char blist = NULL;\n");
   printf("   int show_timestamp = %d;\n", o->show_timestamp);
   printf("   int show_totals = %d;\n", o->show_totals);
   printf("   int iteration_time = %d;\n", o->iteration_time);
   printf("   int it_set = %d;\n", o->it_set);
   printf("   int input_error = %d;\n", o->input_error);
   printf("};\n");

   if(NULL != (bl = o->bl))
   {
      printf("\nCPUs that will be in bold:\n");
   }

   while(bl)
   {
      if(bl->type == BLT_CPU)
         printf(" BOLD: cpu%d\n", bl->cpu);
      else
         printf(" BOLD: band %d\n", bl->cpu);
      
      bl = bl->next;
   }

   printf("\n");
}

/* ========================================================================= */
int is_number(char *cp)
{
  int i = 0;
  while(cp[i] != 0)
  {
    if((cp[i] >= '0') && (cp[i] <= '9')) 
      i++;
    else
      return(0);
  }
  return(1);
}

/* ========================================================================= */
/* Read the next item from the string argv into options struct               */
/*   o       = Struct to read into
     item    = Enum of struct member / type to fill
     argv    = The partial string - from where to start reading               
     returns = Number of chars read
*/
int read_next_char(struct options *o, int item, char *argv)
{
  int read_count = 0;

  /* Grave error, try do do as little harm as possible */
  if(NULL == o)
    return(0);

  /* This is bad, as well, possible bad input, don't mess with position */
  if(NULL == argv)
  {
    o->input_error = 1;
    return(0);
  }

  /* How did this happen?! */
  if(argv[0] == 0)
  {
    o->input_error = 1;
    return(0);
  }

  /* Parse out the remaining items */
  switch(item)
  {
  case RN_COLUMNS:
    if(is_number(argv))
    {
      o->columns = atoi(argv);
      read_count = strlen(argv);
    }
    else
    {
      o->input_error = 1;
      read_count = 0;
    }
    break;
  case RN_SMTCOL:
    if(is_number(argv))
    {
      o->smt_columns = atoi(argv);
      read_count = strlen(argv);
    }
    else
    {
      o->input_error = 1;
      read_count = 0;
    }
    break;
#ifdef STUB_UNUSED
  case RN_XDATA:
    if(XD_NONE == (xdata = parse_xdata(argv)))
    {
      /* fprintf(stderr, "ERROR\n");  / * <======================================= */
      o->input_error = 1;
      /* Whatever is left is untrusted */
      read_count = strlen(argv);
    }
    else
    {
      if(XD_NONE == o->xdata_a)
         o->xdata_a = xdata;
      else
      {
         if(XD_NONE == o->xdata_b)
            o->xdata_b = xdata;
         else
            o->input_error = 1;
      }
      read_count = strlen(argv);
    }
    break;
#endif
  case RN_BLIST:
     if(argv[0] == 0)
     {
        o->input_error = 1;
        read_count = 0;
     }
     else
     {
        if(NULL == (o->blist = (char *)malloc(strlen(argv) + 1)))
        {
           fprintf(stderr, "ERROR: Failed to allocate memory.\n");
           /* Should exit, but just treat it like any other error - this will
              soon exit anyways. */
           o->input_error = 1;
           read_count = 0;
        }

        strcpy(o->blist, argv);
        read_count = strlen(argv);
     }
    break;
  default:
    o->input_error = 1;
    read_count = 0;
  }
  return(read_count);
}

/* ========================================================================= */
/* Read the next item from the argv array into options struct                */
/*   o       = Struct to read into
     item    = Enum of struct member / type to fill
     argv    = The string where we should find our data
     returns = Number of argv array members read (0 or 1)
*/
int read_next_argv(struct options *o, int item, char *argv)
{
  int read_count = 0;

  if(NULL == o)
    return(0);

  if(NULL == argv)
  {
    o->input_error = 1;
    return(0);
  }

  if(0 == argv[0])
  {
    o->input_error = 1;
    return(0);
  }

  /* Parse out the remaining items */
  switch(item)
  {
  case RN_COLUMNS:
    if(is_number(argv))
    {
      o->columns = atoi(argv);
      read_count = 1;
    }
    else
    {
      o->input_error = 1;
      read_count = 0;
    }
    break;
  case RN_SMTCOL:
    if(is_number(argv))
    {
      o->smt_columns = atoi(argv);
      read_count = 1;
    }
    else
    {
      o->input_error = 1;
      read_count = 0;
    }
    break;
#ifdef STUB_UNUSED
  case RN_XDATA:
    if(XD_NONE == (xdata = parse_xdata(argv)))
    {
      /* fprintf(stderr, "ERROR\n");  / * <======================================= */
      o->input_error = 1;
      /* Input could be something else */
      read_count = 0;
    }
    else
    {
      if(XD_NONE == o->xdata_a)
   o->xdata_a = xdata;
      else
      {
   if(XD_NONE == o->xdata_b)
     o->xdata_b = xdata;
   else
     o->input_error = 1;
      }
      read_count = 1;
    }
    break;
#endif
  case RN_BLIST:
     if(argv)
     {
        if(argv[0] == 0)
        {
           o->input_error = 1;
           read_count = 0;
        }
        else
        {
           if(NULL == (o->blist = (char *)malloc(strlen(argv) + 1)))
           {
              fprintf(stderr, "ERROR: Failed to allocate memory.\n");
              /* Should exit, but just treat it like any other error - this will
                 soon exit anyways. */
              o->input_error = 1;
              read_count = 0;
           }

           strcpy(o->blist, argv);
           read_count = 1;
        }
     }
     else
     {
        o->input_error = 1;
        read_count = 0;
     }
    break;
  default:
    o->input_error = 1;
    read_count = 0;
  }
  return(read_count);
}



/* ========================================================================= */
/* Create the struct, and fill with default values                           */
struct options *init_options(void)
{
  struct options *o;

  if(NULL != (o = (struct options *)malloc(sizeof(struct options))))
  {
    o->color_output = O_DEF_COLOR;
    o->column_output = O_COLS_BASIC;
    o->show_help = 0;
    o->show_version = 0;
    o->show_timestamp = 0;
    o->show_totals = O_TOTALS_NONE;
    /* o->boldopts = 0; */
    o->blist = NULL;
    o->bl = NULL;
    o->columns = O_DEF_COLUMNS;
    o->smt_columns = O_DEF_SMTCOL;
    o->smt_user_set = 0;
    o->iteration_time = O_DEF_ITIME;
    o->it_set = 0;
    o->input_error = 0;
    o->debug = 0;
  }
  return(o);
}


/* ========================================================================= */
int next_item_is(char *cp)
{
  if(NULL == cp)
     return(ITEM_ERROR);

  if(cp[0] == 0)
     return(ITEM_END);

  if((cp[0] >= '0') && (cp[0] <= '9'))
     return(ITEM_NUMBER);

  if(cp[0] == '-')
     return(ITEM_DASH);

  if(cp[0] == ',')
     return(ITEM_COMMA);

  return(ITEM_ERROR);
}

/* ========================================================================= */
int read_next_number(int *dest, char *cp)
{
   int reading = 0;
   int i = 0;

   if(NULL == dest)
      return(0);

   if(NULL == cp)
   {
      *dest = 0;
      return(0);
   }

   while((cp[i] >= '0') && (cp[i] <= '9'))
   {
      reading *= 10;
      reading += cp[i] - '0';
      i++;
   }

   *dest = reading;
   return(i);
}

/* ========================================================================= */
struct boldll *parse_blist(char *blist)
{
   char *cp = blist;          /* Pointer to hold our place                   */
   int last_item;             /* The previous item we got                    */
   int last_num = 0;          /* The number we previously read               */
                              /* Set when we encounter a '-'                 */
   int this_num = 0;          /* The latest number we read                   */
   int i;                     /* Simple counter for your counting needs      */
   int ccnt_got;              /* The number of chars we just read            */
   struct boldll *bl = NULL;
   int read_more;             /* Boolean / flag to continue reading          */
   int last_num_set;          /* Boolean => last_num has valid data          */
   int process_term;          /* Boolean - Process the terminating condition
                                 or not. Dash conditions are terminated by 
                                 the second number, not a comma or EOL.      */
   if(NULL == blist)
      return(NULL);

   /* ---------------------------------------------------------------------- */
   /* The first way --- bandX                                                */
   if((blist[0] == 'b') &&
      (blist[1] == 'a') &&
      (blist[2] == 'n') &&
      (blist[3] == 'd'))
   {
      i = 4;

      /* A nice default would be a perfect solution. */
      if(blist[i] == 0)
         return(NULL);

      while(blist[i] != 0)
      {
         if((blist[i] >= '0') && (blist[i] <= '9'))
            i++;
         else
            return(NULL);
      }

      if(NULL == (bl = (struct boldll *)malloc(sizeof(struct boldll))))
         return(NULL);

      bl->type = BLT_BAND;
      bl->next = NULL;
      bl->cpu = atoi(&blist[4]);

      return(bl);
   }

   /* ---------------------------------------------------------------------- */
   /* The second way --- A list of numbers                                   */

   last_num_set = 0;
   process_term = 1;

   read_more = 1;
   last_item = ITEM_BEGIN;
   while(read_more)
   {
      switch(next_item_is(cp))
      {
      /* ============================================= */
      case ITEM_NUMBER:
         if(0 == (ccnt_got = read_next_number(&this_num, cp)))
            read_more = 0;
         else
         {
            /* Move the pointer up */
            cp += ccnt_got;
            /* Set state */
            switch(last_item)
            {
            case ITEM_BEGIN:
            case ITEM_COMMA:
               last_item = ITEM_NUMBER;
               break;
            case ITEM_NUMBER:
               /* Serious error */
               return(NULL);
               break;
            case ITEM_DASH:
               if(last_num_set)
               {
                  bl = append_number_range(bl, last_num, this_num);
                  last_num_set = 0;
                  last_item = ITEM_HANDLED;
               }
               else
                  return(NULL);
               break;
            default:
               return(NULL);
               break;
            } /* switch(last_item) */
         }
         
         break;
      /* ============================================= */
      case ITEM_END:
         switch(last_item)
         {
         case ITEM_BEGIN:
         case ITEM_COMMA:
         case ITEM_DASH:
            return(NULL);
            break;
         case ITEM_HANDLED:
            return(bl);
            break;
         case ITEM_NUMBER:
            bl = append_single_number(bl, this_num);
            return(bl);
            break;
         }
         break;
      /* ============================================= */
      case ITEM_COMMA:
         cp++;
         switch(last_item)
            {
            case ITEM_BEGIN:
            case ITEM_COMMA:
            case ITEM_DASH:
               return(NULL);
               break;
            case ITEM_HANDLED:
               break;
            case ITEM_NUMBER:
               bl = append_single_number(bl, this_num);
               break;
            }
         last_item = ITEM_COMMA;
         break;
      /* ============================================= */
      case ITEM_DASH:
         cp++;
         switch(last_item)
            {
            case ITEM_BEGIN:
            case ITEM_COMMA:
            case ITEM_DASH:
            case ITEM_HANDLED:
               return(NULL);
               break;
            case ITEM_NUMBER:
               last_num = this_num;
               last_num_set = 1;
               break;
            }
         last_item = ITEM_DASH;
         break;
      /* ============================================= */
      case ITEM_ERROR:
         return(NULL);
    }
  } /* while(read_more) */

   return(NULL);
}

/* ========================================================================= */
struct boldll *append_number_range(struct boldll *bl, int begin_num, int end_num)
{
   int num = begin_num;

   while(num <= end_num)
      bl = append_single_number(bl, num++);

   return(bl);
}

/* ========================================================================= */
struct boldll *append_single_number(struct boldll *bl, int number)
{
   struct boldll *newbl;

   if(NULL == (newbl = (struct boldll *)malloc(sizeof(struct boldll))))
      return(NULL);

   newbl->type = BLT_CPU;
   newbl->next = bl;
   newbl->cpu = number;

   return(newbl);
}

