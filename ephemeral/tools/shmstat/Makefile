.PHONY: gmake clean lean ccstart buildit cleanit


CC=/bb/util/version11-012012/usr/vac/bin/xlc
CC_OPTS=-O2 -q64
LD_OPTS=-O2 -q64




shmstat: ccstart main.o options.o distrogph.o
	@printf "Done.\n"
	@printf "Linking..."
	@$(CC) $(LD_OPTS) -o shmstat main.o options.o distrogph.o
	@printf "Done.\n"

main.o: main.c options.h version.h distrogph.h
	@printf "."
	@$(CC) $(CC_OPTS) -c main.c

options.o: options.c options.h
	@printf "."
	@$(CC) $(CC_OPTS) -c options.c

distrogph.o: distrogph.c distrogph.h
	@printf "."
	@$(CC) $(CC_OPTS) -c distrogph.c

ccstart:
	@printf "Compiling."

clean: lean
	@printf "Cleaning up..."
	@rm -f shmstat
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

buildit: shmstat
	@echo "Placing shmstat into USR part"
	@cp shmstat ../../USR_PART/usr/local/bin/shmstat

cleanit: clean
	@echo "Cleaning shmstat from USR part"
	@rm -f ../../USR_PART/usr/local/bin/shmstat
