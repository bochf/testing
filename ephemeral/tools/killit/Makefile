.PHONY: gmake clean lean

CC=gcc
CC_OPTS=-Wall -Werror
LD_OPTS=



killit: ccstart main.o options.o pwhelp.o pshelp.o bbenv.o
	@echo "Done."
	@echo "Linking...\c"
	@$(CC) $(LD_OPTS) -o killit main.o options.o pwhelp.o pshelp.o bbenv.o
	@echo "Done."

ccstart:
	@echo "Compiling...\c"

main.o: main.c version.h version.h pwhelp.h pshelp.h bbenv.h
	@$(CC) $(CC_OPTS) -c main.c
	@echo ".\c"

options.o: options.c options.h
	@$(CC) $(CC_OPTS) -c options.c
	@echo ".\c"

pwhelp.o: pwhelp.c pwhelp.h
	@$(CC) $(CC_OPTS) -c pwhelp.c
	@echo ".\c"

pshelp.o: pshelp.c pshelp.h pwhelp.h
	@$(CC) $(CC_OPTS) -c pshelp.c
	@echo ".\c"

bbenv.o: bbenv.c bbenv.h
	@$(CC) $(CC_OPTS) -c bbenv.c
	@echo ".\c"

clean: lean
	@echo "Cleaning source tree...\c"
	@rm -f killit
	@echo "Done."

lean:
	@echo "Leaning source tree...\c"
	@rm -f *.o
	@rm -f core
	@rm -f *~
	@echo "Done."

gmake:
	@echo "ERROR: This is a gmake makefile. Use gmake, not make."
	@echo
	@false
