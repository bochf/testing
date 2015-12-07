#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "distrogph.h"

/* ========================================================================= */
struct barray *InitBuckets(int type)
{
   struct barray *ba;

   if (NULL == (ba = (struct barray *)malloc(sizeof(struct barray))))
      return(NULL);

   ba->type = type;

   ba->bucket_N = 0;
   ba->bucket_0 = 0;
   ba->bucket_1 = 0;
   ba->bucket_2 = 0;
   ba->bucket_3 = 0;
   ba->bucket_4 = 0;
   ba->bucket_5 = 0;
   ba->bucket_6 = 0;
   ba->bucket_7 = 0;
   ba->bucket_8 = 0;
   ba->bucket_9 = 0;
   ba->bucket_A = 0;
   ba->bucket_B = 0;
   ba->bucket_C = 0;
   ba->bucket_D = 0;
   ba->bucket_E = 0;
   ba->bucket_F = 0;
   ba->bucket_Z = 0;

   ba->nonzero = 0;

   return(ba);
}


/* ========================================================================= */
int HasNonZeroData(struct barray *ba)
{
   if ( NULL == ba )
      return(0);

   return(ba->nonzero);
}

/* ========================================================================= */
int BucketValue(struct barray *ba, unsigned long long value)
{
   if ( NULL == ba )
      return(1);

   if ( value < 0 ) /* out of range - this should never happen */
      return(1);

   if ( ba->type == BA_BASE2 )
   {
      /* Flag the recipt of a non-zero value (not really possible) */
      if ( value > 0 )
         ba->nonzero = 1;

      /* There is probably a craftier way to do this. */

      if ( value >= 32773 ){ ba->bucket_Z++; return(0); }
      if ( value >= 16387 ){ ba->bucket_F++; return(0); }
      if ( value >= 8193 ) { ba->bucket_E++; return(0); }
      if ( value >= 4097 ) { ba->bucket_D++; return(0); }
      if ( value >= 2049 ) { ba->bucket_C++; return(0); }
      if ( value >= 1025 ) { ba->bucket_B++; return(0); }
      if ( value >= 513 )  { ba->bucket_A++; return(0); }
      if ( value >= 257 )  { ba->bucket_9++; return(0); }
      if ( value >= 129 )  { ba->bucket_8++; return(0); }
      if ( value >= 65 )   { ba->bucket_7++; return(0); }
      if ( value >= 33 )   { ba->bucket_6++; return(0); }
      if ( value >= 17 )   { ba->bucket_5++; return(0); }
      if ( value >= 9 )    { ba->bucket_4++; return(0); }
      if ( value >= 5 )    { ba->bucket_3++; return(0); }
      if ( value >= 3 )    { ba->bucket_2++; return(0); }
      if ( value == 2 )    { ba->bucket_1++; return(0); }
      if ( value == 1 )    { ba->bucket_0++; return(0); }
      if ( value == 0 )    { ba->bucket_N++; return(0); }
   }

   if ( ba->type == BA_BASE10 )
   {
      /* Flag the recipt of a non-zero value */
      if ( value > 0 )
         ba->nonzero = 1;

      /* There is probably a craftier way to do this. */
      if ( value >= 10000000000000000 ) { ba->bucket_Z++; return(0); }
      if ( value >= 1000000000000000 ) { ba->bucket_F++; return(0); }
      if ( value >= 100000000000000 ) { ba->bucket_E++; return(0); }
      if ( value >= 10000000000000 ) { ba->bucket_D++; return(0); }
      if ( value >= 1000000000000 ) { ba->bucket_C++; return(0); }
      if ( value >= 100000000000 ) { ba->bucket_B++; return(0); }
      if ( value >= 10000000000 ) { ba->bucket_A++; return(0); }
      if ( value >= 1000000000 )  { ba->bucket_9++; return(0); }
      if ( value >= 100000000 )   { ba->bucket_8++; return(0); }
      if ( value >= 10000000 )    { ba->bucket_7++; return(0); }
      if ( value >= 1000000 )     { ba->bucket_6++; return(0); }
      if ( value >= 100000 )      { ba->bucket_5++; return(0); }
      if ( value >= 10000 )       { ba->bucket_4++; return(0); }
      if ( value >= 1000 )        { ba->bucket_3++; return(0); }
      if ( value >= 100 )         { ba->bucket_2++; return(0); }
      if ( value >= 10 )          { ba->bucket_1++; return(0); }
      if ( value >= 1 )           { ba->bucket_0++; return(0); }
      if ( value == 0 )           { ba->bucket_N++; return(0); }
   }

   /* Should never get here */
   return(1);
}

/* ========================================================================= */
/* BAR_SIZE is also used in the printf statement in dump_bucket_array() */
#define BAR_SIZE 60
/* BAR_ALLOC is a "round" number that is at least 1 more than BAR_SIZE */
#define BAR_ALLOC 64
char *fix_bar(char *bar, int cnt)
{
   int i;

   i = 0;
   while(i < BAR_SIZE)
   {
      if ( i < cnt )
         bar[i] = '#';
      else
         bar[i] = 0;

      i++;
   }

   bar[i] = 0;

   return(bar);
}

/* ========================================================================= */
int DumpBucketArray(struct barray *ba, long graph_factor)
{
   FILE *f;
   char fullname[1024];
   char bar[BAR_ALLOC];
   char mode[4];
   int divfactor;

   /*** Determine graph scaling factor ***/
   /* The divide by zero test should not be necessary - but I do it anyways. */
   if ( 0 == (divfactor = graph_factor / 60))
      divfactor = 1;
   
   /* Note: We quit displaying the larger numbers if there is no remaining data to be displayed */

   if ( ba->type == BA_BASE2 )
   { 
      if ( ba->bucket_N )
         printf("%9s %-5d [%-60s]\n", "0",      ba->bucket_N, fix_bar(bar, ba->bucket_N / divfactor));
      printf("%9s %-5d [%-60s]\n", "1",         ba->bucket_0, fix_bar(bar, ba->bucket_0 / divfactor));
      printf("%9s %-5d [%-60s]\n", "2",         ba->bucket_1, fix_bar(bar, ba->bucket_1 / divfactor));
      printf("%9s %-5d [%-60s]\n", "3-4",       ba->bucket_2, fix_bar(bar, ba->bucket_2 / divfactor));
      printf("%9s %-5d [%-60s]\n", "5-8",       ba->bucket_3, fix_bar(bar, ba->bucket_3 / divfactor));
      printf("%9s %-5d [%-60s]\n", "9-16",      ba->bucket_4, fix_bar(bar, ba->bucket_4 / divfactor));
      printf("%9s %-5d [%-60s]\n", "17-32",     ba->bucket_5, fix_bar(bar, ba->bucket_5 / divfactor));
      printf("%9s %-5d [%-60s]\n", "33-64",     ba->bucket_6, fix_bar(bar, ba->bucket_6 / divfactor));
      printf("%9s %-5d [%-60s]\n", "65-128",    ba->bucket_7, fix_bar(bar, ba->bucket_7 / divfactor));
      printf("%9s %-5d [%-60s]\n", "127-256",   ba->bucket_8, fix_bar(bar, ba->bucket_8 / divfactor));
      printf("%9s %-5d [%-60s]\n", "257-512",   ba->bucket_9, fix_bar(bar, ba->bucket_9 / divfactor));
      printf("%9s %-5d [%-60s]\n", "513-1024",  ba->bucket_A, fix_bar(bar, ba->bucket_A / divfactor));
      if ( ba->bucket_A + ba->bucket_B + ba->bucket_C + ba->bucket_D + ba->bucket_E + ba->bucket_F + ba->bucket_Z > 0 )
         printf("%9s %-5d [%-60s]\n", "*-2^11", ba->bucket_B, fix_bar(bar, ba->bucket_B / divfactor));
      if ( ba->bucket_B + ba->bucket_C + ba->bucket_D + ba->bucket_E + ba->bucket_F + ba->bucket_Z > 0 )
         printf("%9s %-5d [%-60s]\n", "*-2^12", ba->bucket_C, fix_bar(bar, ba->bucket_C / divfactor));
      if ( ba->bucket_C + ba->bucket_D + ba->bucket_E + ba->bucket_F + ba->bucket_Z > 0 )
         printf("%9s %-5d [%-60s]\n", "*-2^13", ba->bucket_D, fix_bar(bar, ba->bucket_D / divfactor));
      if ( ba->bucket_D + ba->bucket_E + ba->bucket_F + ba->bucket_Z > 0 )
         printf("%9s %-5d [%-60s]\n", "*-2^14", ba->bucket_E, fix_bar(bar, ba->bucket_E / divfactor));
      if ( ba->bucket_E + ba->bucket_F + ba->bucket_Z > 0 )
         printf("%9s %-5d [%-60s]\n", "*-2^15", ba->bucket_F, fix_bar(bar, ba->bucket_F / divfactor));
      if ( ba->bucket_Z > 0 )
         printf("%9s %-5d [%-60s]\n", "Overflow", ba->bucket_Z, fix_bar(bar, ba->bucket_Z / divfactor));
   }

   if ( ba->type == BA_BASE10 )
   { 
      if ( ba->bucket_N )
         printf("%9s %-5d [%-60s]\n", "0",     ba->bucket_N, fix_bar(bar, ba->bucket_N / divfactor));
      printf("%9s %-5d [%-60s]\n", "10^0",     ba->bucket_0, fix_bar(bar, ba->bucket_0 / divfactor));
      printf("%9s %-5d [%-60s]\n", "10^1",     ba->bucket_1, fix_bar(bar, ba->bucket_1 / divfactor));
      printf("%9s %-5d [%-60s]\n", "10^2",     ba->bucket_2, fix_bar(bar, ba->bucket_2 / divfactor));
      printf("%9s %-5d [%-60s]\n", "10^3",     ba->bucket_3, fix_bar(bar, ba->bucket_3 / divfactor));
      printf("%9s %-5d [%-60s]\n", "10^4",     ba->bucket_4, fix_bar(bar, ba->bucket_4 / divfactor));
      printf("%9s %-5d [%-60s]\n", "10^5",     ba->bucket_5, fix_bar(bar, ba->bucket_5 / divfactor));
      printf("%9s %-5d [%-60s]\n", "10^6",     ba->bucket_6, fix_bar(bar, ba->bucket_6 / divfactor));
      printf("%9s %-5d [%-60s]\n", "10^7",     ba->bucket_7, fix_bar(bar, ba->bucket_7 / divfactor));
      printf("%9s %-5d [%-60s]\n", "10^8",     ba->bucket_8, fix_bar(bar, ba->bucket_8 / divfactor));
      printf("%9s %-5d [%-60s]\n", "10^9",     ba->bucket_9, fix_bar(bar, ba->bucket_9 / divfactor));
      printf("%9s %-5d [%-60s]\n", "10^10",    ba->bucket_A, fix_bar(bar, ba->bucket_A / divfactor));
      if ( ba->bucket_A + ba->bucket_B + ba->bucket_C + ba->bucket_D + ba->bucket_E + ba->bucket_F + ba->bucket_Z > 0 )
         printf("%9s %-5d [%-60s]\n", "10^11", ba->bucket_B, fix_bar(bar, ba->bucket_B / divfactor));
      if ( ba->bucket_B + ba->bucket_C + ba->bucket_D + ba->bucket_E + ba->bucket_F + ba->bucket_Z > 0 )
         printf("%9s %-5d [%-60s]\n", "10^12", ba->bucket_C, fix_bar(bar, ba->bucket_C / divfactor));
      if ( ba->bucket_C + ba->bucket_D + ba->bucket_E + ba->bucket_F + ba->bucket_Z > 0 )
         printf("%9s %-5d [%-60s]\n", "10^13", ba->bucket_D, fix_bar(bar, ba->bucket_D / divfactor));
      if ( ba->bucket_D + ba->bucket_E + ba->bucket_F + ba->bucket_Z > 0 )
         printf("%9s %-5d [%-60s]\n", "10^14", ba->bucket_E, fix_bar(bar, ba->bucket_E / divfactor));
      if ( ba->bucket_E + ba->bucket_F + ba->bucket_Z > 0 )
         printf("%9s %-5d [%-60s]\n", "10^15", ba->bucket_F, fix_bar(bar, ba->bucket_F / divfactor));
      if ( ba->bucket_Z > 0 )
         printf("%9s %-5d [%-60s]\n", "Overflow", ba->bucket_Z, fix_bar(bar, ba->bucket_Z / divfactor));
   }

   return(0);
}



int GetMaxCount(struct barray *ba)
{
   int max = 0;

   if ( ba->bucket_N > max )
      max = ba->bucket_N;
   if ( ba->bucket_0 > max )
      max = ba->bucket_0;
   if ( ba->bucket_1 > max )
      max = ba->bucket_1;
   if ( ba->bucket_2 > max )
      max = ba->bucket_2;
   if ( ba->bucket_3 > max )
      max = ba->bucket_3;
   if ( ba->bucket_4 > max )
      max = ba->bucket_4;
   if ( ba->bucket_5 > max )
      max = ba->bucket_5;
   if ( ba->bucket_6 > max )
      max = ba->bucket_6;
   if ( ba->bucket_7 > max )
      max = ba->bucket_7;
   if ( ba->bucket_8 > max )
      max = ba->bucket_8;
   if ( ba->bucket_9 > max )
      max = ba->bucket_9;
   if ( ba->bucket_A > max )
      max = ba->bucket_A;
   if ( ba->bucket_B > max )
      max = ba->bucket_B;
   if ( ba->bucket_C > max )
      max = ba->bucket_C;
   if ( ba->bucket_D > max )
      max = ba->bucket_D;
   if ( ba->bucket_E > max )
      max = ba->bucket_E;
   if ( ba->bucket_F > max )
      max = ba->bucket_F;
   if ( ba->bucket_Z > max )
      max = ba->bucket_Z;

   return(max);
}
