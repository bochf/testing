.PHONY: GMAKE clean lean ccstart ossup

# Default/sane settings that will work everywhere
CC=gcc
CC_OPTS=-Wall -Werror -O2
LD=gcc
LD_OPTS=-Wall -Werror -lpthread -O2

OSSUP=false

ifeq "$(shell uname -s)" "Linux"
   CC=gcc
   CC_OPTS=-Wall -Werror -m64 -O2 -DPORT_$(shell uname -s)
   LD=gcc
   LD_OPTS=-m64 -lpthread -m64 -O2
   OSSUP=true
   LANG=C
endif

ifeq "$(shell uname -s)" "SunOS"
   CC=/bb/util/common/SS12_3-20131030/SUNWspro/bin/c99
   CC_OPTS=-m64 -xO3 -DPORT_$(shell uname -s)
   #-xtarget=generic -m64 -xO3 -DPORT_$(shell uname -s) -D_XOPEN_SOURCE=600 -D__EXTENSIONS__
   LD=/bb/util/common/SS12_3-20131030/SUNWspro/bin/c99
   LD_OPTS=-lpthread -lkstat -lrt -lsocket -lnsl -m64 -xO3
   OSSUP=true
   LANG=C
endif

ifeq "$(shell uname -s)" "AIX"
   CC=/bb/util/version11-042012/usr/vac/bin/xlc_r
   LD=/bb/util/version11-042012/usr/vac/bin/xlc_r
   CC_OPTS=-q64 -O2 -DPORT_$(shell uname -s)
   LD_OPTS=-q64 -O2 -lperfstat
   OSSUP=true
endif

# =============================================================================
# The core of compiling and linking
ob: ossup ccstart main.o options.o ringbuf.o threads.o events.o
	@printf ".Done.\nLinking..."
	@$(LD) $(LD_OPTS) -o ob main.o options.o ringbuf.o threads.o events.o
	@printf "Done.\n"

main.o: main.c options.h ringbuf.h version.h
	@$(CC) $(CC_OPTS) -c main.c
	@printf "."

options.o: options.c options.h
	@$(CC) $(CC_OPTS) -c options.c
	@printf "."

ringbuf.o: ringbuf.c ringbuf.h options.h
	@$(CC) $(CC_OPTS) -c ringbuf.c
	@printf "."

threads.o: threads.c threads.h ringbuf.h options.h
	@$(CC) $(CC_OPTS) -c threads.c
	@printf "."

events.o: events.c events.h
	@$(CC) $(CC_OPTS) -c events.c
	@printf "."

# =============================================================================
# Support PHONY targets
ossup:
	@printf "Testing OS support..."
	@if [[ $(OSSUP) == "false" ]] ; then $(OSSUP) ; fi
	@printf "Good.\n"

ccstart:
	@printf "Compiling.."

lean:
	@printf "Leaning project directory."
	@rm -f core
	@printf "."
	@rm -f *.o
	@printf "."
	@rm -f *~
	@printf "Done.\n"

clean: lean
	@printf "Cleaning project directory.."
	@rm -f ob
	@printf ".Done.\n"

GMake:
	@echo "ERROR: This is a gmake file. Use gmake, not make." >&2
	@false

