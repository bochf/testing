.PHONY: GMake clean lean ccstart ossup oslib

CC=false
CC_OPTS=
LD=false
LD_OPTS=

OSSUP=false

ifeq "$(shell uname -s)" "Linux"
   CC=gcc
   CC_OPTS=-Wall -Werror -m64 -O2 -DPORT_$(shell uname -s)
   LD=gcc
   LD_OPTS=-m64 -lpthread -m64 -O2
   #-lpthread -m64 -O2 -lrt -lz
   AR=ar
   AR_OPTS=rs
   OSSUP=true
   LANG=C
endif

ifeq "$(shell uname -s)" "SunOS"
   CC=/bb/util/common/SS12_3-20131030/SUNWspro/bin/c99
   CC_OPTS=-xtarget=generic -m64 -xO3 -DPORT_$(shell uname -s) -D_XOPEN_SOURCE=600 -D__EXTENSIONS__
   LD=/bb/util/common/SS12_3-20131030/SUNWspro/bin/c99
   LD_OPTS=-lpthread -lkstat -lrt -lsocket -lnsl -m64 -xO3
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
   #BITNESS=-q64
   OSSUP=true
endif

export CC
export LD
export CC_OPTS
export LD_OPTS
export AR
export AR_OPTS

sysstatd: ossup oslib ccstart main.o options.o cfgfile.o datatree.o support.o coredata.o iobuf.o daemonize.o fileio.o stdioio.o bbenv.o yamlfmtr.o forest.o urlparse.o httpd.o jsonfmtr.o proctree.o acclog.o qcconr.o dyntree.o
	@printf ".Done.\nLinking.."
	@$(LD) $(LD_OPTS) -o sysstatd main.o options.o cfgfile.o datatree.o support.o coredata.o iobuf.o daemonize.o fileio.o stdioio.o bbenv.o yamlfmtr.o forest.o urlparse.o httpd.o jsonfmtr.o proctree.o acclog.o qcconr.o dyntree.o $(shell uname -s)/libosspecific.a
	@printf ".Done.\n"

main.o: main.c version.h options.h cfgfile.h forest.h urlparse.h iobuf.h cfgfile.h stdioio.h support.h httpd.h fileio.h daemonize.h
	@printf "."
	@$(CC) $(CC_OPTS) -c main.c

cfgfile.o: cfgfile.c cfgfile.h support.h defaults.h options.h
	@printf "."
	@$(CC) $(CC_OPTS) -c cfgfile.c

options.o: options.c options.h
	@printf "."
	@$(CC) $(CC_OPTS) -c options.c

datatree.o: datatree.c datatree.h support.h
	@printf "."
	@$(CC) $(CC_OPTS) -c datatree.c

support.o: support.c support.h 
	@printf "."
	@$(CC) $(CC_OPTS) -c support.c

coredata.o: coredata.c coredata.h datatree.h version.h
	@printf "."
	@$(CC) $(CC_OPTS) -c coredata.c

iobuf.o: iobuf.c iobuf.h support.h
	@printf "."
	@$(CC) $(CC_OPTS) -c iobuf.c

daemonize.o: daemonize.c daemonize.h
	@printf "."
	@$(CC) $(CC_OPTS) -c daemonize.c

fileio.o: fileio.c fileio.h support.h
	@printf "."
	@$(CC) $(CC_OPTS) -c fileio.c

acclog.o: acclog.c acclog.h support.h defaults.h
	@printf "."
	@$(CC) $(CC_OPTS) -c acclog.c

stdioio.o: stdioio.c stdioio.h iobuf.h
	@printf "."
	@$(CC) $(CC_OPTS) -c stdioio.c

bbenv.o: bbenv.c bbenv.h support.h datatree.h
	@printf "."
	@$(CC) $(CC_OPTS) -c bbenv.c

forest.o: forest.c forest.h $(shell uname -s)/osspecific.h datatree.h bbenv.h coredata.h support.h iobuf.h urlparse.h yamlfmtr.h jsonfmtr.h proctree.h options.h
	@printf "."
	@$(CC) $(CC_OPTS) -c forest.c

urlparse.o: urlparse.c urlparse.h support.h
	@printf "."
	@$(CC) $(CC_OPTS) -c urlparse.c

httpd.o: httpd.c httpd.h support.h iobuf.h urlparse.h yamlfmtr.h jsonfmtr.h iobuf.h options.h datatree.h proctree.h acclog.h
	@printf "."
	@$(CC) $(CC_OPTS) -c httpd.c

proctree.o: proctree.c proctree.h support.h iobuf.h urlparse.h dyntree.h $(shell uname -s)/osspecific.h
	@printf "."
	@$(CC) $(CC_OPTS) -c proctree.c

yamlfmtr.o: yamlfmtr.c yamlfmtr.h iobuf.h datatree.h
	@printf "."
	@$(CC) $(CC_OPTS) -c yamlfmtr.c

jsonfmtr.o: jsonfmtr.c jsonfmtr.h iobuf.h datatree.h support.h
	@printf "."
	@$(CC) $(CC_OPTS) -c jsonfmtr.c

# STUB - xmlfmtr
# STUB - csvfmtr

qcconr.o: qcconr.c qcconr.h dyntree.h defaults.h support.h options.h
	@printf "."
	@$(CC) $(CC_OPTS) -c qcconr.c

dyntree.o: dyntree.c dyntree.h
	@printf "."
	@$(CC) $(CC_OPTS) -c dyntree.c

oslib:
	@$(MAKE) -s -C $(shell uname -s)


clean: lean
	@$(MAKE) -s -C Linux clean
	@$(MAKE) -s -C AIX  clean
	@printf "Cleaning directory.."
	@rm -f sysstatd
	@printf ".Done.\n"

lean:
	@$(MAKE) -s -C $(shell uname -s) lean
	@printf "Leaning directory.."
	@rm -f *.o
	@printf "."
	@rm -f core
	@printf "."
	@rm -f *~
	@printf ".Done.\n"

ccstart:
	@printf "Compiling.."

GMake:
	@echo "ERROR: This is a gmake file. Use gmake, not make." >&2
	@false

ossup:
	@printf "Testing OS support..."
	@if [[ $(OSSUP) == "false" ]] ; then $(OSSUP) ; fi
	@printf "Good\n"
