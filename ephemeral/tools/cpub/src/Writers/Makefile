.PHONY: GMAKE cclib clean lean

libwriters.a: cclib wmstdout.o wmfile.o wmcommon.o wmalarm.o wmbcd.o
	@printf "Done.\n"
	@printf "Building library..."
	@$(AR) $(AR_OPTS) libwriters.a wmstdout.o wmfile.o wmalarm.o wmcommon.o wmbcd.o > libbuild.out 2>&1
	@printf "Done.\n"

cclib:
	@printf "Compiling writer library."

lean:
	@printf "Leaning writer tree."
	@rm -f *.o
	@printf "."
	@rm -f libbuild.out
	@printf "Done.\n"

clean: lean
	@printf "Cleaning writer tree."
	@rm -f libwriters.a
	@printf "."
	@printf "Done.\n"

GMAKE:
	@printf "ERROR: This is a gmake file, not a make file. Use gmake." >&2
	@false

wmstdout.o: wmstdout.c wmstdout.h ../providers.h ../configfile.h wmcommon.h
	@$(CC) $(CC_OPTS) -c wmstdout.c
	@printf "."

wmfile.o: wmfile.c wmfile.h ../providers.h ../configfile.h wmcommon.h
	@$(CC) $(CC_OPTS) -c wmfile.c
	@printf "."

wmcommon.o: wmcommon.c wmcommon.h ../providers.h ../configfile.h
	@$(CC) $(CC_OPTS) -c wmcommon.c
	@printf "."

wmalarm.o: wmalarm.c wmalarm.h ../providers.h ../configfile.h wmcommon.h
	@$(CC) $(CC_OPTS) -c wmalarm.c
	@printf "."

wmbcd.o: wmbcd.c wmbcd.h ../providers.h ../configfile.h wmcommon.h
	@$(CC) $(CC_OPTS) -c wmbcd.c
	@printf "."
