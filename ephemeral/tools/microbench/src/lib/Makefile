.PHONY: GMAKE cclibs clean lean

ifeq "$(shell uname -s)" "Linux"
   CC_OPTS += -O2
   CC32=-m32
   CC64=-m64
   AR=ar
   AR_OPTS=rs
endif

ifeq "$(shell uname -s)" "SunOS"
   AR=ar
   CC32=-m32
   CC64=-m64
   AR_OPTS=-r
endif

ifeq "$(shell uname -s)" "AIX"
   CC_OPTS += -O2 -DPORT_$(shell uname -s)
   AR=ar
   CC32=-q32
   CC64=-q64
   AR_OPTS=-v -r
   AR_64=-X 64
endif

default: libmb32.a libmb64.a

libmb32.a: cc32libs optparser32.o verbosity32.o mbenvirons32.o
	@printf "Done.\n"
	@printf "Building 32bit library..."
	@$(AR) $(AR_OPTS) libmb32.a optparser32.o verbosity32.o mbenvirons32.o > libmb32.out 2>&1
	@printf "Done.\n"

libmb64.a: cc64libs optparser64.o verbosity64.o mbenvirons64.o
	@printf "Done.\n"
	@printf "Building 64bit library..."
	@$(AR) $(AR_64) $(AR_OPTS) libmb64.a optparser64.o verbosity64.o mbenvirons64.o > libmb64.out 2>&1
	@printf "Done.\n"

cc32libs:
	@printf "Compiling 32bit micro benchmark library.."

cc64libs:
	@printf "Compiling 64bit micro benchmark library.."

optparser32.o: optparser.c ../version.h
	@$(CC) $(CC_OPTS) $(CC32) -c optparser.c -o optparser32.o
	@printf "."

verbosity32.o: verbosity.c ../version.h
	@$(CC) $(CC_OPTS) $(CC32) -c verbosity.c -o verbosity32.o
	@printf "."

mbenvirons32.o: mbenvirons.c ../version.h
	@$(CC) $(CC_OPTS) $(CC32) -c mbenvirons.c -o mbenvirons32.o
	@printf "."

optparser64.o: optparser.c ../version.h
	@$(CC) $(CC_OPTS) $(CC64) -c optparser.c -o optparser64.o
	@printf "."

verbosity64.o: verbosity.c ../version.h
	@$(CC) $(CC_OPTS) $(CC64) -c verbosity.c -o verbosity64.o
	@printf "."

mbenvirons64.o: mbenvirons.c ../version.h
	@$(CC) $(CC_OPTS) $(CC64) -c mbenvirons.c -o mbenvirons64.o
	@printf "."

lean:
	@printf "Leaning lib tree."
	@rm -f *.o
	@printf "."
	@printf "Done.\n"

clean: lean
	@printf "Cleaning lib tree."
	@rm -f libmb.a libmb32.a libmb64.a
	@printf "."
	@printf "Done.\n"

GMAKE:
	@printf "ERROR: This is a gmake file, not a make file. Use gmake." >&2
	@false

