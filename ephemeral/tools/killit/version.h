#ifndef VERSION_H
#define VERSION_H

/*
  Version History:
   0.1.0   10/28/13 - Original creation
   0.2.0   10/29/13 - Implemented (a brute force) rake functionality.
   0.3.0   10/30/13 - Added (largely stubbed) options.c::validate_options()
                    - Parsing optarg is really a problem!
                    - Ham-handed parsing of command line options "fix".
*/

#define VERSION_STRING "0.3.0"

/*
  ToDo:
   [_] Handle bad options
   [ ] Handle different kill options
   [ ] Unstub the kill options

  Done:
   [X] -k with no "argument" is not processed correctly
   [X] No kill for user op. No kill for user root. These items need to be
       tested for.
   [X] The program should validate that it is on a BB system.
   [X] Implement the rake option
   [X] Base functionality
*/

#endif
