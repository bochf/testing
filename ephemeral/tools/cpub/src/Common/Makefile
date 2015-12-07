.PHONY: GMAKE cclib clean lean

libcmnprov.a: cclib cmnproviders.o filestatp.o fileopsp.o mountdf.o
	@printf "Done.\n"
	@printf "Building common provider library..."
	@$(AR) $(AR_OPTS) libcmnprov.a cmnproviders.o filestatp.o fileopsp.o mountdf.o > libbuild.out 2>&1
	@printf "Done.\n"

cclib:
	@printf "Compiling common provider library."

lean:
	@printf "Leaning common provider tree."
	@rm -f *.o
	@printf "."
	@printf "Done.\n"

clean: lean
	@printf "Cleaning common provider tree."
	@rm -f libcmnprov.a
	@printf "."
	@printf "Done.\n"

GMAKE:
	@printf "ERROR: This is a gmake file, not a make file. Use gmake." >&2
	@false

cmnproviders.o: cmnproviders.c cmnproviders.h  ../providers.h filestatp.h
	@$(CC) $(CC_OPTS) -c cmnproviders.c
	@printf "."

filestatp.o: filestatp.c filestatp.h ../providers.h
	@$(CC) $(CC_OPTS) -c filestatp.c
	@printf "."

fileopsp.o: fileopsp.c fileopsp.h ../providers.h
	@$(CC) $(CC_OPTS) -c fileopsp.c
	@printf "."

mountdf.o: mountdf.c mountdf.h ../providers.h
	@$(CC) $(CC_OPTS) -c mountdf.c
	@printf "."


