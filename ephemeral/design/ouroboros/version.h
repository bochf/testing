#ifndef VERSION_H
#define VERSION_H

/* Version History:
    0.1.0  11/11/15 - Laying the keel
    0.2.0  11/12/15 - Flushing out todos
                    - Re-designed the ring Q methods
    0.3.0  11/13/15 - Wrapping up core functionality
    0.4.0  11/13/15 - Moved into git repo (now two copies - the old will be 
                      deleted/archived at a later point)
                    - Successful compile on AIX & Solaris (Using gcc)
                    - Killed compiler warning on AIX with xlc.
                    - Added timeout for slow write (when the Q does not flush
                      sufficiently to write the EOF marker).
                    - Fixed problem in read() processing.
*/
#define VERSION_STRING "0.4.0"
/*
  ToDo:
   [ ] The write() functionality does not handle short writes. Perhaps we need
       a Stevens Write().
   [ ] Direct I/O does not work on Linux.
   [ ] Add verbosity support.
   [_] Add support for direct I/O reads and writes.
   [_] Flush out about() and help() code.
   [_] Check port on AIX and Solaris.
   [ ] This should call pathconf() or stat() on both file systems to determine
       the optimal block size.
   [ ] Set the mode based on the previous file settings(?) This should mimic
       what cp does perhaps?
   [ ] Would it be worth while to keep the file offset so that the reader
       can retry if it fails or is interrupted?

  Done:
   [X] Handle the read() and write() failures properly.
   [X] Add support for the new (slow write) events in main.c.
   [X] Yank all the debug / $tub code out of main.c.
   [X] Add in -r X and -b X options (I used slightly different options)
   [X] Add the actual read() and write() calls.
   [X] Functionalize the message passing into a straightfoward API.
   [X] The source and dest files need to be specificed in the ringbuf struct,
       and need to be opened *prior* to launching the threads.
   [X] Move all Q management to the ringbuf source files. (Ideally this code
       can be used on other projects.)
   [X] Add in handling of source/dest options.
   [X] Define ring buffer structure
   [X] Write the ring buffer initialization function(s)
   [X] Merge in options files
*/

#endif
