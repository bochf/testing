.PHONY: clean lean libprocperf default libcc all testharness thstart

CC=/usr/bin/gcc
LD_OPTS=-O3
CC_OPTS=-Wall -Werror -O3
LC_ALL=C
export LC_ALL

default: libprocperf.a

libprocperf: libprocperf.a

all: testperf vcpu

testharness: libprocperf.a thstart testperf cputree
	@printf "Done.\n"

thstart:
	@printf "Creating test harness..."

testperf: libprocperf.a testperf.c procperf.h
	@$(CC) -o testperf testperf.c libprocperf.a
	@printf "."

cputree: libprocperf.a cputree.c procperf.h
	@$(CC) -o cputree cputree.c libprocperf.a
	@printf "."

vcpu: libprocperf.a vcpu.c procperf.h
	@printf "Creating vcpu..."
	@$(CC) -o vcpu vcpu.c libprocperf.a
	@printf "Done.\n"

libprocperf.a: libcc cpustat.o cpuinfo.o proccmn.o
	@printf "Done.\n"
	@printf "Building the library..."
	@ar rs libprocperf.a cpustat.o cpuinfo.o proccmn.o > lastlink.out 2>&1
	@printf "Done.\n"

libcc:
	@printf "Building library objects."

cpustat.o: cpustat.c cpustat.h
	@printf "."
	@$(CC) $(CC_OPTS) -c cpustat.c

cpuinfo.o: cpuinfo.c cpuinfo.h
	@printf "."
	@$(CC) $(CC_OPTS) -c cpuinfo.c

proccmn.o: proccmn.c proccmn.h
	@printf "."
	@$(CC) $(CC_OPTS) -c proccmn.c

clean: lean
	@printf "Cleaning..."
	@rm -rf libprocperf.a
	@printf "."
	@rm -f vcpu
	@printf "."
	@rm -f cputree
	@printf "."
	@rm -f testperf
	@printf "Done.\n"

lean:
	@printf "Leaning..."
	@rm -rf *~ *.o core
	@printf "Done.\n"

