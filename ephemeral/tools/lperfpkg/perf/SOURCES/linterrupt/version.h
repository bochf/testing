#ifndef VERSION_H
#define VERSION_H
/*
  Version History:
   0.1.0    8/11/15 - Original versioned release
   0.2.0    8/12/15 - Added options parsing
                    - Added timestamps
                    - Moved data structures into dedicated .h file
   0.3.0    8/13/15 - Added munge.c/h to handle derived values
   0.4.0    8/19/15 - CPU names are now cached (preventing the need to
                      repeatedly build the label strings).
   0.5.0    8/19/15 - Moved into git
   0.6.0    8/20/15 - Added multi-color mode (the default) as an improvement
                      on the existing color mode.
   0.7.0    8/24/15 - Added CPU utilization numbers (-c)
   0.8.0    8/26/15 - Code cleanup of basicdiff.c
                    - Code cleanup of main.c
                    - Added -+ option
                    - Turned on -O3 optimization
                    - Code cleanup of munge.c
   0.8.1    8/26/15 - Basic hackery to work on RHEL 6.2 (previously only 5.8)
   0.9.0    8/27/15 - Continued fixes for RHEL 6.2
   0.9.1    8/27/15 - Fixed issues with type and label
                    - Changed data type of Interrupt
   0.9.2    8/27/15 - Fixed memory issue   
   0.9.3    8/27/15 - Turned -g off, turned -O3 back on
*/
#define VERSION_STRING "0.9.3"
/*
  ToDos:
   [ ] Add a -w(ide) or -s(mall) spacing option for output. This is a short-
       cut to providing a proper "wrap" that will display in sections.
   [ ] Use a signal handler to get out of the main while loop
   [ ] Write -N(ames) table either in output or a dump option.
   [_] Implement -n(ame drop) to exclude interrupt names from the regular
       output. A table could be built at the top (start) of output to denote
       what interrupts are for what.
   [ ] Proper commenting on all (external) functions.
   [ ] Add an option for ERR and MIS numbers.
   [Q] Add a #define for the % printf format specifiers for field width.
   [D] Implement -p(ivot)

  Done:
   [X] Validate that unsigned long is appropriate for this data. Perhaps a
       long long is better.
   [X] gather.c::build_data_structures() should check the length of the 
       label and type when copying in the data.
   [X] Long running boxes overflow unsigned long for the interrupts since
       boot value. The data type needs to be modified to long long.
   [X] version.h is not a Makefile dependency on main.c (but it is an actual
       dependency).
   [X] Fix the parsing of "label" and "type" strings on RHEL 6.2.
   [X] Validate input in options.c
   [X] Resolve the data items (primarily) in munge.c. I have S**B comments on
       them.
   [W] Write a better timer than sleep(1) ---> Withdrawn. To go deeper than
       sleep would require setting a timer or a much more complex threaded
       method. I think sleep() is sufficient. The drift is not signficant.
   [X] Calculate per-second stats if using an iteration of > 1
   [X] Add CPU utilization numbers to the header / output. This will require
       reading in another /proc file.
   [X] Implement a -l(OC drop) that does not include all LOC timer interrupts.
   [X] Remove kruft in the munge code
   [X] Write heat-map code for hottest interrupt. Color code the total
       interrupts for that interrupt as well as the name.
   [X] Write a munge function to calculate all derived data
   [X] Create a -s(hort names) option to shorten the dev names.
   [X] Handle all stubs in gather.c
   [X] Implement -m(onochrome)
   [X] The header (CPUx) is expensive and not properly justified. While it is
       not likely to be an immediate problem, at some point there will be
       three digits of CPUs and it will break. The string should be pre-created
       and justified printed instead of creating the string each time.
   [X] Implement -c(ollapse)
   [X] fflush(stdout) on each dump iteration
   [X] Insert timestamp at top of output
   [X] Parse command line options
   [X] Create a version file
*/



#endif
