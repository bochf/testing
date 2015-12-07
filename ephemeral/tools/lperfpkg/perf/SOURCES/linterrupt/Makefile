.PHONY: default clean lean ccstart

# Where the procperf library references can be found:
#  * Here - look for libprocperf.a references They will be
#           relative pathed and tend to stick out.
#  * main.c - #includes and API calls
#  * datastruct.h - Part of the core data structure for the app


CC=gcc
LD=gcc
CC_OPTS=-Wall -Werror -O3 -m64
LD_OPTS=-O3 -m64
BIN_NAME=linterrupt
DBINNAME=-DBIN_NAME=\"$(BIN_NAME)\"

default: $(BIN_NAME)

# Link target - Uses a variable name (not comitted to current name)
$(BIN_NAME): ccstart main.o gather.o basicdiff.o options.o support.o munge.o datastruct.o
	@printf "Done.\n"
	@printf "Linking $(BIN_NAME)..."
	@$(LD) $(LD_OPTS) -o $(BIN_NAME)                           \
	            main.o gather.o basicdiff.o options.o          \
	            support.o munge.o datastruct.o                 \
               ../../../common/procperf/libprocperf.a 
	@printf "Done.\n"


# Compile targets
main.o: main.c gather.h options.h basicdiff.h munge.h version.h
	@printf "."
	@$(CC) $(CC_OPTS) $(DBINNAME) -c main.c

gather.o: gather.c gather.h datastruct.h support.h
	@printf "."
	@$(CC) $(CC_OPTS) -c gather.c

basicdiff.o: basicdiff.c basicdiff.h datastruct.h
	@printf "."
	@$(CC) $(CC_OPTS) -c basicdiff.c

options.o: options.c options.h support.h
	@printf "."
	@$(CC) $(CC_OPTS) -c options.c

support.o: support.c support.h
	@printf "."
	@$(CC) $(CC_OPTS) -c support.c

munge.o: munge.c munge.h datastruct.h
	@printf "."
	@$(CC) $(CC_OPTS) -c munge.c

datastruct.o: datastruct.c datastruct.h
	@printf "."
	@$(CC) $(CC_OPTS) -c datastruct.c


# "Support" targets
clean: lean
	@printf "Cleaning build directory."
	@rm -f $(BIN_NAME)
	@printf "."
	@printf "Done.\n"

lean:
	@printf "Leaning build directory."
	@rm -f *.o
	@printf "."
	@rm -f *~
	@printf "."
	@rm -f core
	@printf "Done.\n"

ccstart:
	@printf "Compiling $(BIN_NAME).."
