.PHONY: GMAKE cclib clean lean



libprovider.a: cclib vminfoprov.o aixproviders.o lpcputprov.o processprov.o lpdadapter.o
	@printf "Done.\n"
	@printf "Building library..."
	@$(AR) $(AR_OPTS) libprovider.a vminfoprov.o aixproviders.o lpcputprov.o processprov.o lpdadapter.o > libbuild.out 2>&1
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

aixproviders.o: aixproviders.c aixproviders.h  ../providers.h vminfoprov.h lpcputprov.h lpdadapter.h
	@$(CC) $(CC_OPTS) -c aixproviders.c
	@printf "."

vminfoprov.o: vminfoprov.c vminfoprov.h ../providers.h
	@$(CC) $(CC_OPTS) -c vminfoprov.c
	@printf "."

lpcputprov.o: lpcputprov.c lpcputprov.h ../providers.h
	@$(CC) $(CC_OPTS) -c lpcputprov.c
	@printf "."

processprov.o: processprov.c processprov.h ../providers.h
	@$(CC) $(CC_OPTS) -c processprov.c
	@printf "."

lpdadapter.o: lpdadapter.c lpdadapter.h ../providers.h
	@$(CC) $(CC_OPTS) -c lpdadapter.c
	@printf "."
