.PHONY: GMAKE ccstart clean lean

THIS_TREE=mallocspeed module

ifeq "$(shell uname -s)" "Linux"
   CC_OPTS += -m64
   LD_OPTS += -m64 -lrt -lpthread
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

mallocspeed: ccstart mallocspeed.o
	@printf "Done.\n"
	@printf "Linking $(THIS_TREE)..."
	@$(CC) $(LD_OPTS) -o mallocspeed mallocspeed.o ../lib/libmb$(BITNESS).a -lpthread
	@printf "Done.\n"

ccstart:
	@printf "Compiling $(THIS_TREE)..."

mallocspeed.o: mallocbm.c ../version.h
	@$(CC) $(CC_OPTS) -c mallocbm.c
	@mv mallocbm.o mallocspeed.o
	@printf "."

lean:
	@printf "Leaning $(THIS_TREE)."
	@rm -f *.o
	@printf "."
	@printf "Done.\n"

clean: lean
	@printf "Cleaning $(THIS_TREE)."
	@rm -f mallocspeed
	@printf "."
	@printf "Done.\n"

GMAKE:
	@printf "ERROR: This is a gmake file, not a make file. Use gmake." >&2
	@false

