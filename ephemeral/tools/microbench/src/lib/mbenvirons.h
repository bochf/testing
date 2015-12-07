#ifndef MBENVIRONS_H
#define MBENVIRONS_H

/* =========================================================================
 * Name: ZombieRepellent
 * Description: Disable the need to wait() for your children - just let them
 *              die without becoming zombies
 * Paramaters: None
 * Returns: results of the sigaction call
 * Side Effects: Will change how SIGCHLD signals are handled
 * Notes: The system() call will re-enable this in the parent.
 *        If you are using the system() call, your results may not be
 *        consistent. Consider replacing with fork()/exec*() to bypass that
 *        behaviour. (This is the world as I understand it, not necessarily
 *        as tested.)
 *
 *        Portability has not (yet) been tested on all platforms.
 */
int ZombieRepellent(void);

#endif
