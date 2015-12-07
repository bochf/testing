#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <stdlib.h>

struct ipcitem
{
   int shmi;
   void *shma;
   struct ipcitem *next;
};

int main(int argc, char *argv[])
{
   int i;
   int shmi;
   void *shma;
   int mcnt;
   long msuccess;
   struct ipcitem *ipclist;
   struct ipcitem *ipcthis;

   if ( argc == 2 )
   {
      mcnt = atoi(argv[1]);
   }
   else
   {
      fprintf(stderr, "ERROR: A count paramater is required.\n");
      return(1);
   }

   printf("LDR_CNTRL = %s\n", getenv("LDR_CNTRL"));
   printf("EXTSHM = %s\n", getenv("EXTSHM"));

   ipclist = NULL;
   i = 0;
   msuccess = 0;
   while ( i < mcnt )
   {
      if ( NULL == (ipcthis = (struct ipcitem *)malloc(sizeof(struct ipcitem))) )
      {
         fprintf(stderr, "ERROR: Unable to allocate memory for ipcitem list.\n");
         /* Way to go! Blow up and leave your previous IPC segments all over the place! */
         return(1);
      }

      printf("shmget(IPC_PRIVATE, 4096, IPC_CREAT...) = ");
      shmi = shmget(IPC_PRIVATE, 4096, IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
      printf("%d\n", shmi);

      if ( -1 == shmi )
      {
         perror("shmget()");
         i = mcnt;
         continue;
      }
      
      printf("shmat(%d, NULL, 0) = ", shmi);
      shma = shmat(shmi, NULL, 0);
      printf("0x%lX\n", (unsigned long)shma);

      if ( (void *)-1 != shma )
      {
         msuccess++;

         ipcthis->shmi = shmi;
         ipcthis->shma = shma;
         ipcthis->next = ipclist;
         ipclist = ipcthis;
      }
      else
      {
         printf("shmctl(%d, IPC_RMID, NULL) = %d\n", shmi, shmctl(shmi, IPC_RMID, NULL));
         i = mcnt;
         continue;
      }


      i++;
   }

   printf("shmat()'d %ld memory segments.\n", msuccess);
   printf("=========================================\n");
   printf("Run: svmon -P %ld\n", (long)getpid());
   printf("Run: procmap -S %ld\n", (long)getpid());
   printf("=========================================\n");
   printf("Hit return to exit.\n");
   fflush(stdout);
   getchar();

   i = 1;
   ipcthis = ipclist;
   while(ipcthis)
   {
      printf("Removing item %d.\n", i++);
      printf("shmdt(%lX) = %d\n", (unsigned long)ipcthis->shma, shmdt(ipcthis->shma));
      printf("shmctl(%d, IPC_RMID, NULL) = %d\n", ipcthis->shmi, shmctl(ipcthis->shmi, IPC_RMID, NULL));
      fflush(stdout);

      ipcthis = ipcthis->next;
   }

   /* Leave main() */
   return(0);
}
