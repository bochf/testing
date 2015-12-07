#ifndef THREADSUPT_H
#define THREADSUPT_H

#include <pthread.h>
#include "providers.h"

/* STUB - These should not be here */
#include "configfile.h"


/* How the locks / sychronization works
   - Each worker has a condition variable and an associated mutex. This is
     used primarily to signal (wakeup) the workers, but also to protect the
     data members and more importantly to keep the writer from getting a jump
     on writing before the workers are done. These mutexes/cond variables are
     in the worker_arg struct (array - one for each worker).
   - The worker lock is called "datalock" - this is because it protects the
     cond variable (runcond) but also the data. So it protects the data but
     it also potects the "data_ready" int. This is a boolean that is set (to
     1) by the worker when the data is ready, and then (re)set (to 0) by the
     writer once it has comitted the data.
   - The writer will "rake" the worker data_ready variables (taking locks as
     it does so) and counting the number of data_ready's set to 1. When the
     sum is equal to the expected (woken threads) value, then all the data
     is ready to be written. The "rake" is *sort* of a spin - but it will only
     spin/rake/walk through the worker values once or twice. The code to do
     this is in the writer() function in threadsupt.c.
   - Why this rake?
     This is because while the workers are woken first, they *sometimes* do
     not get out of bed until *after* the writer has started. So the data_ready
     value is not set when the writer and workers are woken, and there is
     no chance of the writer getting ahead of the workers.
   - The worker only holds the datalock mutex when collecting data and when
     it goes into and out of the condition variable blocking code. So, the
     majority of the time it is not holding this lock (although reading the
     code might suggest otherwise).
   - The writer has TWO mutexes. The first protects the cond variable and
     the write *operation*. The cond variable protection is fairly clear,
     but the write operation must be protected because we do not want the
     user ctrl-C-ing out of main() and having that exit in the middle of a
     (series of) write operation(s). I was unable to simulate this case (even
     in a write-heavy scenario) - but the protection is there none the less.
   - The second writer mutex is called "readygo" and it denotes exactly that.
     The mutex is locked while the writer is in operation - or more
     specifically: locked at the top of an iteration and only freed once the
     iteration has completed (and data has been written to disk). So, the
     main loop takes this lock before it wakes up any workers and the writer.
     It will not take the lock if the writer is still finishing up the previous
     iteration.
   - Here is a "graph" of how the flow (basically) works:

     main               worker1            worker2            writer

     START              block(runcond)     block(runcond)     block(writecond)
     take(readygo)
     signal(worker1)    take(datalock)
     signal(worker2)    get data           take(datalock)
     signal(writer)                        get data           take(writelock)
     sleep(N)                                                 rake data_ready
     [GOTO TOP]         data_ready=1       data_ready=1       (**NOTE1)
                        free(datalock)     free(datalock)     leave rake spin
                        [GOTO TOP]         [GOTO TOP]         write (**NOTE2)
                                                              free(readygo)
                                                              [GOTO TOP]

     Note 0: The block() on condition variables here implies the proper lock
        of the related mutexes.
     Note 1: The rake code must take a lock on the worker mutex before it can
        access the data_ready variables. So the majority of the "spin" is
        really sitting in a lock. It may/can/will beat the worker into play,
        but will immediately free the lock for that worker once it evaluates
        the data_ready variable (for that worker). The full implementation of
        the rake code is really simple - but not fully demonstrated here.
     Note 2: Still under review... BUT... there are two thoughts here:
        1) We must protect the data during the write from being updated by
           the providers! So let's take a lock and make a copy of the 
           provider data. This will be a "writer safe" copy that was created
           (copied) under a lock and for exclusive use of the writer. This
           was (partially) implemented prior to the introduction of the 
           readygo lock.
        2) Whoa cowboy! The introduction of the readygo lock along with the
           rake code properly protects the data. We do not begin the write
           until the rake is passed - this means that all workers (providers)
           are done modifying the data. And the providers cannot be signaled
           (started) again until we free the READYgo lock. So, in effect, the
           data IS protected by a series of lock/sychronizations.
        This is a VERY IMPORTANT issue because:
          > Locks are per-provider
          > Data is accessed per-item (NOT per provider) 
           See the bullet point on provider vs data locking
   - On provider vs data locking:
     The providers have locks on their data. The providers are called in a 
     "lockable" / orderable manner. The data from the providers is referenced
     in a different "order". To the point: There is a linked list of data items
     that are seen by the writer. The items are not "ordered" by the provider,
     but are ordered by the sequence in which they are written to output.
     So providers look like:
     proot -> provider1 -> provider2 -> provider3 -> NULL
     And data (to the writer) looks like:
     proot -> data1(p1)-> data2(p1)-> data3(p3)->data4(p1)-> data5(p3)-> NULL
     To walk the writer list one would have to take individual locks on the
     each of the providers (p1, p1, p3, p1, p3) as it walks the list. This is
     annoying and expensive. The first approach was to walk the provider list
     and take a provider lock and COPY all data for that provider. (This was
     a small copy, but still a copy.) This "writer safe" copy then could be
     utilized by the writer in any order as it was safe from worker/provider
     interference. The alternative method is to use the "readygo" lock that
     the writer signals READY and the main (timer) loop does the GO.
   - The condition variables do not "loop" on the condition evaluation. By
     "convention" condition variables are typically used with a "flag" of
     sorts. I read this as a convention, not a requirement. Each thread has
     its own condition variable, and signals "effectively" happen at the same
     time - The condition here is just a method to block wating for the top
     of the iteration to start.
 */

/* ========================================================================= */
struct worker_arg
{
   pthread_mutex_t datalock;  /* The mutex protecting data and runcond       */
   pthread_cond_t runcond;    /* Cond variable for worker to run             */
   int tnum;                  /* Index of the thread, not the TID            */
   int sinterval;             /* Divisor for averaging per-second            */
   struct provider *prov;

#ifdef PROPOSED_CHANGE
   int data_ready;            /* 1=ready to print, 0=not ready, -1=error     */
#else
   int data_ready;            /* 1 = data is ready to print. 0 = not ready   */
#endif
};


/* ========================================================================= */
struct writer_arg
{
   pthread_mutex_t readygo;
   pthread_mutex_t writelock;
   pthread_cond_t writecond;
   unsigned long wcount;      /* count of writes */
   int wtcount;               /* count of worker threads */

   void *writer_data;
   int (*do_write)(void *data_handle);
   int (*writer_finish)(void *data_handle);

   struct worker_arg *worker_list; /* How writer knows about the workers */
};

/* ========================================================================= */
struct sighandler_arg
{
   unsigned long ms_sleept;     /* Sleep time - in ms  */
   pthread_cond_t cond;         
   pthread_mutex_t mutex;
};

/* ========================================================================= */
struct thread_root
{
   pthread_t *tlist;      /* A list of all threads (0 = writer, !0 = worker) */
   int wkrcount;          /* How many workers we have                        */

   int *slot;             /* Keep track of what slots are in use */
   
   struct worker_arg *wkalist; /* worker arg list */
   struct writer_arg *wrarg;   /* writer arg (not a list - only one)         */


};

/* ========================================================================= */
/* How the indexes work
                             0          1          2          n 
    thread_root->slot        Writer     Worker     Worker     Worker
    thread_root->tlist       Writer     Worker     Worker     Worker
    thread_root->wkalist                Worker-1   Worker-1   Worker-1
    thread_root->wrarg       Writer

    slot is indexed with 0 = Writer, Workers starting at 1
    tlist is indexed with 0 = Writer, Workers starting at 1
    wkalist is indexed with 0 = the first Worker
    wrarg is a reference to the only writer
 */

/* =========================================================================
 * Name: InitThreadRoot
 * Description: Set up all thread mutexes, cond vars, and data structs
 * Paramaters: Count of the number of workers
 *             second interval - divisor if averaging per-second stats
 * Returns: Pointer to all thread data, NULL on error
 * Side Effects: Allocates memory, initializes thread mutexes/conds
 *               It does NOT spawn the threads.
 * Notes: Call this first.
 */
struct thread_root *InitThreadRoot(int wcount, int sinterval);


/* =========================================================================
 * Name: SpawnWorkerThread
 * Description: 
 * Paramaters: 
 * Returns: 
 * Side Effects: 
 * Notes:
 */
int SpawnWorkerThread(struct thread_root *tr, struct provider *prov);


/* =========================================================================
 * Name: SpawnWriterThread
 * Description: Start the writer() thread
 * Paramaters: thread_root as returned by InitThreadRoot()
 * Returns: 0 on success, non-0 on failure
 * Side Effects: Spawns a thread to do writer() work.
 * Notes: Call SetWriterData() First!
 */
int SpawnWriterThread(struct thread_root *tr);

/* =========================================================================
 * Name: SetWriterData
 * Description: Register the writer implementation with thread framework
 * Paramaters: thread_root (as returned from InitThreadRoot())
 *             void *data - See notes below
 *             do_write() - Function pointer to writer() implementation
 * Returns: 1 on failure, 0 on success
 * Side Effects: Just assigns values - but read notes!
 * Notes: Call it thusly:
 *
 *  switch ( writer_type )
 *  {
 *  case MY_WRITER_TYPE:
 *     if(SetWriterData(tr, MyWriterInit(p, c), MyWriterWrite))
 *        exit(1);
 *     break;
 *  case OTHER_WRITER_TYPE:
 *  ...
 *
 *  In the above example:
 *    MyWriterInit() is the initializing function for the writer. It can
 *       follow any convention for input - but generally takes arguments for
 *       struct proot *, and struct config *. It must return a void *.
 *    MyWriterWrite() is the "output a line" function for the writer
 *       implementation. It has a distinct prototype. Discussion on this
 *       can be found in WritingAWriter.txt.
 *
 *  This function will expect a potential failure of the init() function
 *  as the startup for the writer module implementation. So just call it
 *  inline if you like. So you can call the init function inline for the
 *  second paramater.
 *
 *  Q: But why not just have one Set/Spawn for writer like you have for 
 *     the provider (worker) threads?
 *  A: Because 
 *      1. the writer init() function will validate user input
 *      2. user input needs to be validated before we spawn threads
 *      3. worker threads must start before the writer
 *     So - do this:
 *      1. Init the thread data
 *      2. Call SetWriterData() with the writer init function to validate input
 *      3. Spawn the worker threads
 *      4. Spawn the writer thread
 *
 */
int SetWriterData(struct thread_root *tr,
                  void *data,
                  int (*do_write)(void *data_handle),
                  int (*writer_finish)(void *data_handle));

/* =========================================================================
 * Name: SpawnTimerThread
 * Description: Set up signal handling, launch signal handler thread
 * Paramaters: ms_sleep - Time in ms between iterations
 * Returns: pointer to sighandler_arg on success, NULL on failure
 * Side Effects: Allocates memory, masks signals, spawns thread
 * Notes: This MUST be called before the other threads are spawned because
 *        all the other threads will inherit the changes this makes. If
 *        the order needs to be changed, then the signal handling part
 *        should be divorced from the rest.
 */
struct sighandler_arg *SpawnTimerThread(unsigned long ms_sleep);


#endif
