.PHONY: gmake clean lean ccstart


# Commentary on the build tool:
#  - It is possible to specify a platform-specific build tool. (ie: xlc for AIX,
#    gcc for Linux, etc...)

CC=gcc
CC_OPTS=-Wall -Werror
LD_OPTS=

# Very gmakey - Determine the default system type for build using shell command
UNAME_S=$(shell uname -s)
DEFSYS=-DDEFAULT_SYSTYPE=\"$(UNAME_S)\"


bbhelpq: ccstart main.o slurp.o filefmt.o options.o compile.o query.o endian.o ssearch.o decompile.o
	@printf "Done.\n"
	@printf "Linking..."
	@$(CC) $(LD_OPTS) -o bbhelpq main.o slurp.o filefmt.o options.o compile.o query.o endian.o ssearch.o decompile.o
	@printf "Done.\n"

main.o: main.c version.h filefmt.h options.h
	@printf "."
	@$(CC) $(CC_OPTS) -c main.c

slurp.o: slurp.c slurp.h filefmt.h
	@printf "."
	@$(CC) $(CC_OPTS) -c slurp.c

filefmt.o: filefmt.c filefmt.h
	@printf "."
	@$(CC) $(CC_OPTS) -c filefmt.c

options.o: options.c options.h ssearch.h filefmt.h version.h
	@printf "."
	@$(CC) $(CC_OPTS) $(DEFSYS) -c options.c

compile.o: compile.c compile.h options.h slurp.h
	@printf "."
	@$(CC) $(CC_OPTS) -c compile.c

decompile.o: decompile.c decompile.h options.h query.h
	@printf "."
	@$(CC) $(CC_OPTS) -c decompile.c

query.o: query.c query.h options.h compile.h ssearch.h filefmt.h
	@printf "."
	@$(CC) $(CC_OPTS) -c query.c

endian.o: endian.c endian.h
	@printf "."
	@$(CC) $(CC_OPTS) -c endian.c

ssearch.o: ssearch.c ssearch.h
	@printf "."
	@$(CC) $(CC_OPTS) -c ssearch.c

ccstart:
	@printf "Build system type is \""$(UNAME_S)\"". Setting default system type to the same.\n"
	@printf "Compiling."

clean: lean
	@printf "Cleaning up..."
	@rm -f bbhelpq
	@printf "Done.\n"

lean:
	@printf "Leaning up."
	@rm -f core
	@printf "."
	@rm -f *~
	@printf "."
	@rm -f *.o
	@printf "Done.\n"

gmake:
	@printf "This is a gmake file. Please use gmake, not make.\n"
	@printf "\n"
	@printf "\n"
	@printf "\n"
	@printf "\n"
	@false

# =============================================================================
### --- BB Packages integration

buildit: bbhelpq
	@echo "Placing bbhelpq into USR part"
	@cp bbhelpq ../../USR_PART/usr/local/bin/bbhelpq

cleanit: clean
	@echo "Cleaning bbhelpq from USR part"
	@rm -f ../../USR_PART/usr/local/bin/bbhelpq
