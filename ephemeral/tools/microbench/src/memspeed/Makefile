.PHONY: GMAKE ccstart clean lean

THIS_TREE=memspeed module

ifeq "$(shell uname -s)" "Linux"
   CC_OPTS += -m64 -O2 -g -fopenmp
   LD_OPTS += -m64 -lrt -lnuma -fopenmp
   BITNESS=64
endif

ifeq "$(shell uname -s)" "SunOS"
   CC_OPTS += -m64
   LD_OPTS += -m64
   BITNESS=64
endif

ifeq "$(shell uname -s)" "AIX"
   CC_OPTS += -q64
   LD_OPTS += -q64
   BITNESS=64
endif

memspeed: ccstart memspeed.o
	@printf "Done.\n"
	@printf "Linking $(THIS_TREE)..."
	@$(CC) $(LD_OPTS)  -o memspeed  memspeed.o ../lib/libmb$(BITNESS).a
	@printf "Done.\n"

ccstart:
	@printf "Compiling $(THIS_TREE)..."

memspeed.o: memorybm.c ../version.h
	@$(CC) $(CC_OPTS) -c memorybm.c
	@mv memorybm.o memspeed.o
	@printf "."

lean:
	@printf "Leaning $(THIS_TREE)."
	@rm -f *.o
	@printf "."
	@printf "Done.\n"

clean: lean
	@printf "Cleaning $(THIS_TREE)."
	@rm -f memspeed
	@printf "."
	@printf "Done.\n"

GMAKE:
	@printf "ERROR: This is a gmake file, not a make file. Use gmake." >&2
	@false

