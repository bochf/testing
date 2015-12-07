.PHONY: GMAKE ccstart clean lean ossup cccore oslib wmlib cmnlib

OSSUP=false

ifeq "$(shell uname -s)" "Linux"
        CC=gcc
        CC_OPTS=-Wall -Werror -m64 -O2 -DPORT_$(shell uname -s)
        LD_OPTS=-lpthread -m64 -O2 -lrt -lz
        AR=ar
        AR_OPTS=rs
        OSSUP=true
        LANG=C
endif

ifeq "$(shell uname -s)" "SunOS"
        CC=/bb/util/common/SS12_3-20131030/SUNWspro/bin/c99
        CC_OPTS=-xtarget=generic -m64 -xO3 -DPORT_$(shell uname -s) -D_XOPEN_SOURCE=600 -D__EXTENSIONS__
        LD_OPTS=-lpthread -lkstat -lrt -lz -m64 -xO3
        OSSUP=true
        AR=ar
        AR_OPTS=-rs
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

export CC
export LD
export CC_OPTS
export LD_OPTS
export AR
export AR_OPTS


cpub: ossup oslib wmlib cmnlib cccore main.o providers.o coreprov.o options.o configfile.o threadsupt.o mainhelp.o Stevens.o
	@printf "Done.\n"
	@printf "Linking..."
	@$(CC) $(LD_OPTS) -o cpub main.o providers.o coreprov.o options.o configfile.o threadsupt.o mainhelp.o Stevens.o $(shell uname -s)/libprovider.a Writers/libwriters.a Common/libcmnprov.a
	@printf "Done.\n"

ossup:
	@printf "Testing OS support..."
	@$(OSSUP)
	@printf "Good\n"

cccore:
	@printf "Compiling core."

oslib:
	@$(MAKE) -s -C $(shell uname -s)

wmlib:
	@$(MAKE) -s -C Writers

cmnlib:
	@$(MAKE) -s -C Common

main.o: main.c version.h providers.h coreprov.h configfile.h threadsupt.h
	@$(CC) $(CC_OPTS) -c main.c
	@printf "."

options.o: options.c options.h mainhelp.h
	@$(CC) $(CC_OPTS) -c options.c
	@printf "."

providers.o: providers.c providers.h mainhelp.h
	@$(CC) $(CC_OPTS) -c providers.c
	@printf "."

coreprov.o: coreprov.c providers.h coreprov.h
	@$(CC) $(CC_OPTS) -c coreprov.c
	@printf "."

configfile.o: configfile.c providers.h configfile.h
	@$(CC) $(CC_OPTS) -c configfile.c
	@printf "."

threadsupt.o: threadsupt.c threadsupt.h providers.h
	@$(CC) $(CC_OPTS) -c threadsupt.c
	@printf "."

mainhelp.o: mainhelp.c mainhelp.h
	@$(CC) $(CC_OPTS) -c mainhelp.c
	@printf "."

# I do not like this here - but I am at a bit of a loss to find a better home
Stevens.o: Stevens.c Stevens.h
	@$(CC) $(CC_OPTS) -c Stevens.c
	@printf "."

lean:
	@$(MAKE) -s -C $(shell uname -s) lean
	@$(MAKE) -s -C Writers lean
	@$(MAKE) -s -C Common lean
	@printf "Leaning source tree."
	@rm -f *.o
	@printf "."
	@printf "Done.\n"

clean: lean
	@$(MAKE) -s -C $(shell uname -s) clean
	@$(MAKE) -s -C Writers clean
	@$(MAKE) -s -C Common clean
	@printf "Cleaning source tree."
	@rm -f cpub
	@printf "."
	@printf "Done.\n"

GMAKE:
	@printf "ERROR: This is a gmake file, not a make file. Use gmake." >&2
	@false
