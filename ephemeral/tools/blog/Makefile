.PHONY: gmake clean lean default compatchk all toolsbuild toolsclean test

CC=gcc
CCOPT=-Wall -Werror
LDOPT=
OSSUP=false
BINNAME=blog

ifeq "$(shell uname -s)" "Linux"
	CC=gcc
	CCOPT=-Wall -Werror -m64 -O2 -D_XOPEN_SOURCE=500
	LDOPT=-lpthread -m64 -O2
	OSSUP=true
	LANG=C
endif

ifeq "$(shell uname -s)" "SunOS"
	CC=gcc
	CCOPT=-Wall -Werror -m64 -O2
	LDOPT=-lpthread -m64 -O2
	OSSUP=true
	LANG=C
endif

ifeq "$(shell uname -s)" "Darwin"
	CC=gcc
	CCOPT=-Wall -Werror
	LDOPT=
	OSSUP=true
endif

ifeq "$(shell uname -s)" "FreeBSD"
	CC=gcc
	CCOPT=-Wall -Werror
	LDOPT=-lpthread
	OSSUP=true
endif

ifeq "$(shell uname -s)" "AIX"
	CC=/bb/util/version11-012012/usr/vac/bin/xlc_r
	CCOPT=-O2 -q64
	LDOPT=-lpthreads -O2 -q64
	OSSUP=true
endif

default: compatchk start $(BINNAME)
	@printf "Done.\n"

compatchk:
	@$(OSSUP)

start:
	@printf "Compiling..."

$(BINNAME): main.o options.o cfileops.o endian.o datahandler.o cheader.o fileops.o rfileops.o filefinder.o rmetadata.o
	@printf "Done.\nLinking..."
	@$(CC) $(LDOPT) -o $(BINNAME) main.o options.o cfileops.o endian.o datahandler.o cheader.o fileops.o rfileops.o filefinder.o rmetadata.o

main.o: main.c options.h cfileops.h version.h datahandler.h
	@printf "."
	@$(CC) $(CCOPT) -DBIN_NAME=\"$(BINNAME)\" -c main.c

options.o: options.c options.h
	@printf "."
	@$(CC) $(CCOPT) -c options.c

cfileops.o: cfileops.c cfileops.h options.h endian.h cheader.h fileops.h
	@printf "."
	@$(CC) $(CCOPT) -c cfileops.c

rfileops.o: rfileops.c rfileops.h options.h fileops.h
	@printf "."
	@$(CC) $(CCOPT) -c rfileops.c

fileops.o: fileops.c fileops.h 
	@printf "."
	@$(CC) $(CCOPT) -c fileops.c

cheader.o: cheader.c cheader.h options.h endian.h fileops.h
	@printf "."
	@$(CC) $(CCOPT) -c cheader.c

rmetadata.o: rmetadata.c rmetadata.h fileops.h rfileops.h 
	@printf "."
	@$(CC) $(CCOPT) -c rmetadata.c

endian.o: endian.c endian.h
	@printf "."
	@$(CC) $(CCOPT) -c endian.c

filefinder.o: filefinder.c filefinder.h
	@printf "."
	@$(CC) $(CCOPT) -c filefinder.c

datahandler.o: datahandler.h datahandler.c options.h cfileops.h
	@printf "."
	@$(CC) $(CCOPT) -c datahandler.c

all: knowndata $(BINNAME)

knowndata: knowndata.c
	@printf "Building knowndata..."
	@$(CC) $(CCOPT) -o knowndata knowndata.c
	@printf "Done.\n"

lean:
	@printf "Leaning..."
	@rm -f *~ core *.o
	@printf "Done.\n"

clean: lean
	@printf "Cleaning..."
	@rm -f $(BINNAME) knowndata circular
	@printf "Done.\n"

gmake:
	@echo "ERROR: This is a gmake makefile. Use gmake, not make." >&2
	@echo >&2
	@false

test: default knowndata
	@printf "Copying binaries into test directory..."
	@cp knowndata testscripts
	@cp $(BINNAME) testscripts
	@printf "Done.\n"



# These are strictly for the AIX build process
toolsbuild: default
	@echo "Placing blog into USR part"
	@cp blog ../../USR_PART/usr/local/bin/blog
	@cp blog.1 ../../USR_PART/usr/share/man/man1

toolsclean: clean
	@echo "Cleaning blog from USR part"
	@rm -f ../../USR_PART/usr/local/bin/blog
	@rm -f ../../USR_PART/usr/share/man/man1/blog.1
