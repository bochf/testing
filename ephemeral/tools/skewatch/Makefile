.PHONY: GMAKE clean lean ossup

OSSUP=false

ifeq "$(shell uname -s)" "Linux"
        CC=gcc
        CC_OPTS=-Wall -Werror -m64 -DPORT_$(shell uname -s)
        LD_OPTS=-m64 -lrt
        OSSUP=true
        LANG=C
endif

ifeq "$(shell uname -s)" "SunOS"
        CC=/bb/util/common/SS12_3-20131030/SUNWspro/bin/c99
        CC_OPTS=-xtarget=generic -m64 -xO3 -DPORT_$(shell uname -s) -D_XOPEN_SOURCE=600 -D__EXTENSIONS__
        LD_OPTS=-lrt -m64 -xO3
        OSSUP=true
        LANG=C
endif

ifeq "$(shell uname -s)" "AIX"
        CC=/bb/util/version11-042012/usr/vac/bin/xlc_r
        LD=/bb/util/version11-042012/usr/vac/bin/xlc_r
        CC_OPTS=-q64 -O2 -DPORT_$(shell uname -s)
        LD_OPTS=-q64 -O2 -lperfstat -lz
        AR=ar
        AR_OPTS=-X 32_64 -v -r 
        BITNESS=-q64
        OSSUP=true
endif


skewatch: ossup main.o
	@printf "Linking..."
	@$(CC) $(LD_OPTS) -o skewatch main.o
	@printf "Done.\n"

ossup:
	@printf "Testing OS support..."
	@$(OSSUP)
	@printf "Good\n"

main.o: main.c
	@printf "Compiling..."
	@$(CC) $(CC_OPTS) -c main.c
	@printf "Done.\n"

clean: lean
	@printf "Cleaning the build dir..."
	@rm -f skewatch
	@printf "Done.\n"

lean:
	@printf "Leaning the build dir..."
	@rm -f core *.o *~
	@printf "Done.\n"

GMAKE:
	@printf "This is a Gmakefile not a makefile. Use gmake."
	@echo
	@echo
	@echo
	@echo
	@echo
	@false
