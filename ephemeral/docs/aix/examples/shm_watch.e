#!/bin/probevue

/*
   This should be called thusly:
   probevue -X <command_to_probe> shm_watch.e

   I used syscallx - strictly to be consistent. There is not a probe for
   munmap() in the syscall provider.

   print_args() could have been used... but I was unsure of its portability.
*/

/* Function prototypes */
int shmdt(void *);
void *mmap(void *, unsigned long, int, int, int, unsigned long);
int munmap(void *, unsigned long);
void *shmat(int, void*, int);
int shmctl(int, int, void *);


/* ---------- mmap() ------------------------------------------------------- */
@@syscallx:$__CPID:mmap:entry
{
   printf("mmap(0x%X, %ld, %d, %d, %d, %lu) = ", __arg1, __arg2, __arg3, __arg4, __arg5, __arg6);
}

@@syscallx:$__CPID:mmap:exit
{
   printf("0x%X\n", __rv);
}

/* ---------- munmap() ----------------------------------------------------- */
@@syscallx:$__CPID:munmap:entry
{
   printf("munmap(0x%X, %lu) = ", __arg1, __arg2);
}

@@syscallx:$__CPID:munmap:exit
{
   printf("%d\n", __rv);
}

/* ---------- shmat() ------------------------------------------------------ */
@@syscallx:$__CPID:shmat:entry
{
   printf("shmat(%d, %X, %d) = ", __arg1, __arg2, __arg3);
}

@@syscallx:$__CPID:shmat:exit
{
   printf("0x%X\n", __rv);
}

/* ---------- shmctl() ----------------------------------------------------- */
@@syscallx:$__CPID:shmctl:entry
{
   printf("shmctl(%d, %d, --) = ", __arg1, __arg2);
}

@@syscallx:$__CPID:shmctl:exit
{
   printf("%d\n", __rv);
}

/* ---------- shmdt() ------------------------------------------------------ */
@@syscallx:$__CPID:shmdt:entry
{
   printf("shmdt(0x%X) = ", __arg1);
}

@@syscallx:$__CPID:shmdt:exit
{
   printf("%d\n", __rv);
}

/* Leave when the watched program exits */
@@syscall:$__CPID:exit:entry
{
   exit();
}
