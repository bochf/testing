Makefile      - A *special* make file - read before use
gather.c      - Gathers data from libperfstat
gather.h      - Public info from gather.c
inputest.c    - Used to debug input parsing on the Mac (debug wrapper)
main.c        - Contains main() for the app
munge.c       - Does calculations - moved out of the display code for cleanliness
munge.h       - Public items from munge.c
options.c     - Code to parse user input
options.h     - Public interfaces for options.c
readme.txt    - This file
scatter.c     - Display code
scatter.h     - Public interface for scatter.c
vcpu.mk       - Bloomberg plink make file
version.h     - Version information


Data is broken into two key structures
 struct options   - Defined in options.h
                  - Used to store user info
 struct cpu_list  - Defined in gather.h
                  - Used to store all cpu data
                  - Contains sub-structs defined in the same file and libperfstat
