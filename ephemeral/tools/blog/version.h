#ifndef VERSION_H
#define VERSION_H
/*
  Version History:
   0.1.0   11/16/11 - Original creation - mostly stubbed
                    - File size will be requested file size plus header.
   0.2.0   11/20/11 - Added usage()
                    - Clarifications / additions to ToDo
   0.3.0   11/25/11 - Added endian determination
                    - Additional work on header code
   0.4.0   11/26/11 - Work on dumping circular file content 
                    - Started on proper structure to writing circular file
   0.5.0   11/28/11 - Added about() / -a / --about
                    - Added -H option
                    - Fixed issue on parsing input
                    - Added macros for HERE, HEAD and TAIL
   0.6.0   11/30/11 - Added RECV to header for total bytes received
   0.7.0    12/3/11 - Added header offsets (defines)
                    - Added check for data type compatibility
                    - The most basic circular logging is working
   0.8.0    12/4/11 - Work on circular wrapping
                    - Moved out and cleaned up header code (major refactoring)
   0.9.0    12/4/11 - Snapped a version (lots of changes)
                    - options.c parsing is now complete
   0.10.0   12/4/11 - More changes (another snap)
                    - Bug smash: circular log (header data) is now correct
   0.11.0   12/6/11 - Minor changes to Makefile
                    - Started writing mmap() code, and then finished
   0.12.0   12/7/11 - Started in on the rfile code.
                    - Inserted foundation for multi-platform (OS) support in
                      the Makefile. Darwin is there, Linux is stubbed. (No
                      plans for a Linux port at this time.)
                    - Getting kind of tired. Snapping a version, going to bed.
                      Some of the rfileops fundamentals are now in place.
   0.13.0   12/7/11 - Minor changes to rfile struct (rfileops.h)
   0.14.0   12/8/11 - Initial threads integration
   0.15.0  12/10/11 - More work on threads
                    - Changed the binary name ==> eternitee
   0.16.0  12/10/11 - Snapped version after some development
                    - Work on -r code
   0.17.0  12/10/11 - Snapped version after some development
   0.18.0  12/11/11 - Testing
                    - Fixed file creation issue
                    - Fixed delete sychronization issue
                    - Fixed two rolls in a second issue
                    - Fixed "sparse" file option (for mmap backed fill)
   0.19.0  12/12/11 - AIX porting issues
                      > Must be 64 bit (for 64bit (s)size_t
                      > Requires a header reference to strings.h
                      > Requires a -lpthreads option to link
   0.20.0  12/13/11 - Fixed error messages on secondary items (-l, -r, & -D)
                    - Cleanup of this file
                    - Put in a movie and went to bed.
   0.21.0  12/14/11 - Code clean up / documentation
   0.22.0  12/14/11 - FreeBSD port
   0.23.0  12/14/11 - Fix of mmap() issue found on FreeBSD.
   0.24.0  12/15/11 - Added new fields to circular log header. Tested good.
                    - Added new file for filefinder.h. Configured for compile
                      but not link at this time. Its primary purpose is to 
                      do directory search and file size determination.
   0.25.0  12/17/11 - Work on append mode.
                    - CLOG header now uses bit-field rather than single byte
                      for endian data. The remaining bits are for error flags.
   0.26.0  12/18/11 - More work on append mode.
                    - clog append works. rlog append "works", provided you
                      "use it right".
   0.26.1  12/19/11 - AIX port / testing
                    - Fixed issue with size of time_t in AIX (64 bit port).
   0.27.0  12/19/11 - New MetaData option - Basic layout
   0.27.1  12/20/11 - clog without the -m is NOT writing the info on exit.
                      A signal handler is required. -m works!
   0.28.0  12/25/11 - Work on rfile metadata dump / file format  
   0.29.0  12/26/11 - More work on rfile / metadata
   0.30.0  12/27/11 - Re-mapping of the -M option
   0.31.0  12/28/11 - Cleanup of metadata dump/load/print
   0.32.0    1/1/12 - Set-up signal handlers
   0.33.0    1/1/12 - Snapped version
                    - Goat hunting
                    - Fixes bytes on disk issue (metadata print)
		              - Implemented a dump for rotating mode
   0.34.0    1/1/12 - Minor code cleanup
   0.35.0    1/1/12 - First Linux port
   0.36.0  04/17/12 - Modification to correctly return name.
   0.37.0   1/11/13 - Fixed issue with naming convention in -r mode
                    - Moved Makefile to use xlc
                    - Worked around some warnings in xlc (not in gcc)
                    - Cantidate for 1.0.0
   0.38.0   11/4/13 - Moved to ephemeral repo (multi-platform ports)
                    - Fixed minor compile warning on Linux
                    - Added Linux compile options
                    - Added Solaris support
   0.39.0    8/5/14 - Fixed the Ctrl-C/Kill problem on the process and the
                      circular mode. (Now, mmap mode is not required.)
                    - Fixed issue with bad bit test and bit check
   0.40.0    8/5/14 - I CHANGED THE HEADER FORMAT BUT NOT THE HEADER VERSION!
                      While working on endian code I converted to standard
                      types. This means that the pointers and the time_t
                      sizes were type sensitive. So these had to be defined
                      independently of the bitness of the binary.
                    - I added in some test cases (that just run some commands
                      that you have to visually inspect).
   0.41.0    8/5/14 - Man page addition
                    - -r mode testcase
*/
#define VERSION_STRING "0.41.0"
/*
  ToDo:
  [_] Both modes need to be tested for robustness. These have to support long
      running processes - without killing the pipe.
  [_] The lock sychronization is broken. A better method is required.
      (This is fixed for now - not a perfect fix, but not horrible)

  Done:
  [X] Cleanup, testing, documentation
  [X] The lseek error is being thrown on what appears to be valid data.
  [X] Reserve space for timestamps in rlog metadata format (to be possbily
      used later)
  [X] -d(ump) is not compatible with -r(otating) log mode
  [X] Depricate all r* functions that were used for appending rlog (before
      metadata file became a requirement).
  [X] Some printf()s in GetNextRFile()
  [X] Total bytes on disk is wrong in PrintMetaData()
  [X] Rotating logs (append) works... but if a full rotation did not happen:
      - Then we roll prematurely at a (sometimes) crazy size
      - Existing files are not put in the rotating list (to be deleted)
      - Old metadata files are not detected (and overwritten/updated)
      - -t mode is not detected, it must be passed.
  [X] Handle append mode for rolling files - A bit less simple. Gotta do a
      search in the directory for files that have basename.<something>.
      Use the largest of the found files (there has to be at least two and
      the largest is the file size).
  [X] sighandler for HUP, and other signals. (This is a low priority because
      the logger should not be getting killed - it would most likely be the
      source that would be killed.)
  [X] eRlogType not handled in discovery (append mode)! (modifed method - 
      handled)
  [X] Bitfield not properly handled in rlog metadata write.
  [X] Holy SHIT - append is much harder than anticipated! (Now simplified)
  [X] metadata to rfile is not quite complete. Some issues in reading in the
      filenames.
  [X] Need to read-in and fill the contents of fullfn when reading the
      metadata file. This is not done at this time (just filenames).
  [X] -M base file name not printed
  [X] -M current files not printed
  [X] Need option to read vs dump metadata file. - Should read, and have
      a separate dump function. (Read is done, dump is not)
  [X] Need to handle the -M option as a dump metadata (to stdout). And have 
      the rlog option *always* write the metadata file.
  [X] Timestamps are 0 values (actually, small negative values - when adjusted
      by localtime()) in the AIX port when dumping a circular file. (time_t is
      64 bits in 64 bit AIX port. I created a int32_t, assigned the time to
      it before write - so this should be portable back to Darwin and other
      32 bit ports.)
  [X] Append on rolling files must handle timestamp files.
  [X] circular append is not writing to the proper location. It is not
      appending. The pointers are not properly initialized on start.
  [X] Handle append mode for circular file - Simple: read header and pick
      up where it last left off.
  [X] Make clog endian byte a bit field. (Note: it is currently not a bit
      field - but acts as though it is - because we only use the first bit
      of the byte (we only store 1 or 0))
  [X] Add the following items to the circular header:
      - size <---- Required for -a(ppend)
      - first write time
      - last write time 
  [X] ./knowndata 20 | ./eternitee -m -D knowndata knowndata.clog <== fails
  [X] On the header ops - if the file is open (when we would normally expect
      it to be closed) then close it and open with the options we want.
  [X] Buffer sizes are not correct - The define is not global
      (It is on the datahandler - dump buffers may be different)
  [X] AIX port
  [X] Validate magic when dumping a file. (Already done)
  [X] "clog -r" is not considered an error - even though no file name is
      specified and no argument is supplied to -r.
  [X] Lots of extra (depricated) code laying about
  [X] Mask on file create has execute bit set for owner
  [X] Create a sparse file in circular/mmap mode - rather than doing a 0 fill
  [X] There are still messages containing CLOG
  [X] Test/Debug -r mode
  [X] Should we watch stderr too? No, if the user wants stderr they should 
      capture it. What does tee do? - (Don't be stupid: How do you
      differentiate input? Man, you are a dunce.)
  [X] Other things the header should save:
      - End of header (start of data) offset [one byte, after magic]
      - Size of offset data types [one byte, after magic]
      - Size of file
      (This is basically all done - or is derivable)
  [X] Need to handle two rolls in a second, with a timestamped file. Or more
      specifically, if the last file name has same timestamp.
  [X] Handle overwrites of an existing file
  [X] Can you make better names for the rfileops.h structures? Not now, I cant.
  [X] -r value must be >= 2. 1 can work provided we stick to the percentage
      filled before deleting the last file. If 1 was used then the user could
      potentially lose data (the liklihood would be higher).
  [X] input validation: -t only with -r
  [X] input validation: -m only with !-r
  [X] input validation: -M only with -r
  [X] Handle iRolling option
  [X] Finish thread sychronization code
  [X] Make sure that all rfile members are initialized
  [X] Make sure that stats are updated in rfile struct
  [X] Add -r sub-option to dump metadata into <basename>.metadata. This should
      include (at least):
      + basename
      + Bytes received
      + Bytes in last file
      + Files rolled
      + Description
      + Any errors
      + Bytes dropped after error
  [X] Change -a to append, use -A for about
  [X] Can we rename clog - coffee (a play on tee)? - eternitee
  [X] Plan a thread strategy for rfileops.c
  [X] -r should have an additional -t option. This timestamps the rolls.
  [X] -t value must be set in options.c (even if it is not currently
      documented or handled (in input parsing))
  [X] rfileops.h - the struct rfilelist needs an used/unused flag.
  [X] Need to handle overwrite. Just make Open default to truncate.
  [X] rfileops.h - the struct rfile needs a current and delete pointer. One
      option may be to create a doubly linked list. (No this is not necessary)
  [X] Implement mmap() option.
  [X] tail is not advanced when there is a description
  [X] A check is required for o->description to be no more than 
      ( 255 - OFFSET_MINDATA ) 
  [X] Parse iRolling option
  [X] File size check in options.c should be 100K bytes not 4K
  [X] Header is not correct (head/tail off by 8 bytes)
  [X] Write the SIZE() function (as GetFileSize() )
  [X] Why on earth are you constantly translating the data area to the file
      area on the head/tail pointers!? This is just too confusing.
  [X] Define header offset names for various header offsets that lseek() may
      need to move to. This should be part of the header documentation in the
      (c) header.h file.
  [X] Write stdout option
  [X] Header needs to capture the endian system this was created on. Because
      you are saving raw data types - this is relevant.
  [X] better define header
  [X] Optional feature: Ability to include a string describing clog contents.
      Even if you do not do this, make a version option in the header to add
      nutty stuff like this later.
  [X] Define / hard-set head/tail pointer size
  [X] Write usage() - Drive functionality from usage() design
  [X] Un-stub options parsing


*/
#endif
