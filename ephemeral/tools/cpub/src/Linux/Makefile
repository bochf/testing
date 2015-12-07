.PHONY: GMAKE cclib clean lean

libprovider.a: cclib linproviders.o meminfoprov.o procpidstat.o procstat.o
	@printf "Done.\n"
	@printf "Building library..."
	@$(AR) $(AR_OPTS) libprovider.a linproviders.o meminfoprov.o procpidstat.o procstat.o > libbuild.out 2>&1
	@printf "Done.\n"

cclib:
	@printf "Compiling provider library."


linproviders.o: linproviders.c linproviders.h
	@$(CC) $(CC_OPTS) -c linproviders.c
	@printf "."

meminfoprov.o: meminfoprov.c meminfoprov.h
	@$(CC) $(CC_OPTS) -c meminfoprov.c
	@printf "."

procpidstat.o: procpidstat.c procpidstat.h
	@$(CC) $(CC_OPTS) -c procpidstat.c
	@printf "."

procstat.o: procstat.c procstat.h
	@$(CC) $(CC_OPTS) -c procstat.c
	@printf "."

lean:
	@printf "Leaning OS-specific tree."
	@rm -f *.o
	@printf "."
	@printf "Done.\n"

clean: lean
	@printf "Cleaning OS-scpecific tree."
	@rm -f libprovider.a
	@printf "."
	@printf "Done.\n"

GMAKE:
	@printf "ERROR: This is a gmake file, not a make file. Use gmake." >&2
	@false

