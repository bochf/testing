.PHONY: GMAKE cclib clean lean

libprovider.a: cclib kstatprov.o solproviders.o
	@printf "Done.\n"
	@printf "Building library..."
	@$(AR) $(AR_OPTS) libprovider.a kstatprov.o solproviders.o > libbuild.out 2>&1
	@printf "Done.\n"


cclib:
	@printf "Compiling provider library."

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

solproviders.o: solproviders.c solproviders.h kstatprov.h
	@$(CC) $(CC_OPTS) -c solproviders.c
	@printf "."

kstatprov.o: kstatprov.c kstatprov.h
	@$(CC) $(CC_OPTS) -c kstatprov.c
	@printf "."


