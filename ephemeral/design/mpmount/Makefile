.PHONY: GMAKE ccstart clean lean ossup

OSSUP=false

ifeq "$(shell uname -s)" "Linux"
        CC=gcc
        CC_OPTS=-Wall -Werror -m64 -O2 -DPORT_$(shell uname -s)
        LD_OPTS=-lpthread -m64 -O2 -lrt
        AR=ar
        AR_OPTS=rs
        OSSUP=true
        LANG=C
endif

ifeq "$(shell uname -s)" "SunOS"
        CC=/bb/util/common/SS12_3-20131030/SUNWspro/bin/c99
        CC_OPTS=-xtarget=generic -m64 -xO3 -DPORT_$(shell uname -s) -D_XOPEN_SOURCE=600
        LD_OPTS=-lpthread -lkstat -lrt -m64 -xO3
        OSSUP=true
        AR=ar
        AR_OPTS=-rs
        LANG=C
endif

ifeq "$(shell uname -s)" "AIX"
        CC=/bb/util/version11-042012/usr/vac/bin/xlc_r
        LD=/bb/util/version11-042012/usr/vac/bin/xlc_r
        CC_OPTS=-q64 -O2 -DPORT_$(shell uname -s)
        LD_OPTS=-q64 -O2 -lperfstat
        AR=ar
        AR_OPTS=-X 32_64 -v -r 
        BITNESS=-q64
        OSSUP=true
endif

mpmount: ossup ccstart main.o mntlist.o
	@printf "Done.\n"
	@printf "Linking..."
	@$(CC) $(LD_OPTS) -o mpmount main.o mntlist.o
	@printf "Done.\n"


main.o: main.c
	@$(CC) $(CC_OPTS) -c main.c
	@printf "."

mntlist.o: mntlist.c
	@$(CC) $(CC_OPTS) -c mntlist.c
	@printf "."


ossup:
	@printf "Testing OS support..."
	@$(OSSUP)
	@printf "Good\n"

lean:
	@printf "Leaning source tree."
	@rm -f *.o
	@printf "."
	@printf "Done.\n"

clean: lean
	@printf "Cleaning source tree."
	@rm -f cpub
	@printf "."
	@printf "Done.\n"

GMAKE:
	@printf "ERROR: This is a gmake file, not a make file. Use gmake." >&2
	@false

ccstart:
	@printf "Compiling."

