.PHONY: default clean lean ccbuild ccstart

CC=gcc
CC_OPTS=-Wall -Werror -O3 -m64
LD_OPTS=-O3 -m64
LC_ALL=C
export LC_ALL

default: vcpu

# inputest is just a wrapper around options to debug development
inputest: inputest.o options.o
	@$(CC) -o inputest inputest.o options.o

inputest.o: inputest.c options.h
	@$(CC) $(CC_OPTS) -c inputest.c

ccstart:
	@echo -n "Compiling."

# scatter.o undocumented.o smtchk.o
vcpu: ccstart options.o main.o gather.o munge.o scatter.o
	@echo "Done."
	@echo -n "Linking..."
	@$(CC) $(LD_OPTS) -o vcpu options.o main.o gather.o munge.o scatter.o ../../../common/procperf/libprocperf.a
	@echo "Done."

main.o: main.c options.h munge.h gather.h
	@echo -n "."
	@$(CC) $(CC_OPTS) -c main.c

options.o: options.h options.c
	@echo -n "."
	@$(CC) $(CC_OPTS) -c options.c

munge.o: munge.h munge.c options.h
	@echo -n "."
	@$(CC) $(CC_OPTS) -c munge.c

gather.o: gather.h gather.c options.h
	@echo -n "."
	@$(CC) $(CC_OPTS) -c gather.c

scatter.o: scatter.c gather.h scatter.c options.h
	@echo -n "."
	@$(CC) $(CC_OPTS) -c scatter.c

clean: lean
	@echo -n "Cleaning source tree..."
	@rm -f vcpu
	@echo "Done."

lean:
	@echo -n "Leaning source tree..."
	@rm -rf core *~ *.o
	@echo -n "."
	@rm -f inputest
	@echo -n "."
	@rm -f libprocperf.a
	@echo "Done."
