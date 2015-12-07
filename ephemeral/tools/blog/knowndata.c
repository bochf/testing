#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
   int i, j, iterations;
  
   if (argc == 2)
      iterations = atoi(argv[1]);
   else
      iterations = 1;
   
   if (iterations == 1)
   {
      i = 0;
      
      while(i < 1000)
      {
         i++;
         printf("[%-3d]ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789\n",
                i);
         fflush(stdout);
      }
   }
   else
   {
      j = 0;
      
      while(j < iterations)
      {
         i = 0;
         while(i < 1000)
         {
            printf("[%3d.%03d]ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789\n",
                   j, i);
            fflush(stdout);
            i++;
         }
         j++;
      }
   }
   
   return(0);
}
