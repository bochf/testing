.PHONY: GMAKE cclib clean lean

libprovider.a: cclib osspecific.o aixbasic.o process.o
	@printf "Done.\n"
	@printf "Building library.."
	@echo $(AR) $(AR_OPTS) libosspecific.a osspecific.o aixbasic.o process.o > libbuild.cmd 2>&1
	@printf "."
	@$(AR) $(AR_OPTS) libosspecific.a osspecific.o aixbasic.o process.o > libbuild.out 2>&1
	@printf "Done.\n"

cclib:
	@printf "Compiling OS-specific library."

aixbasic.o: aixbasic.h aixbasic.c 
	@$(CC) $(CC_OPTS) -c aixbasic.c
	@printf "."

process.o: process.h process.c 
	@$(CC) $(CC_OPTS) -c process.c
	@printf "."

osspecific.o: osspecific.h osspecific.c aixbasic.h process.h
	@$(CC) $(CC_OPTS) -c osspecific.c
	@printf "."

lean:
	@printf "Leaning OS-specific tree."
	@rm -f *.o
	@printf "."
	@printf "Done.\n"

clean: lean
	@printf "Cleaning OS-scpecific tree."
	@rm -f libosspecific.a
	@printf "."
	@printf "Done.\n"

GMAKE:
	@printf "ERROR: This is a gmake file, not a make file. Use gmake." >&2
	@false

