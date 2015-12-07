.PHONY: GMAKE ccstart clean lean

THIS_TREE=netmirror module

ifeq "$(shell uname -s)" "Linux"
   CC_OPTS += -m64 -O2
   LD_OPTS += -m64 -O2
endif

ifeq "$(shell uname -s)" "SunOS"
   CC_OPTS += -m64
   LD_OPTS += -m64 -lsocket -lnsl
endif

ifeq "$(shell uname -s)" "AIX"
   CC_OPTS += -q64 -O2 -DPORT_$(shell uname -s)
   LD_OPTS += -q64 -O2
endif


netmirror: ccstart main.o sender.o receiver.o sink.o nethelp.o
	@printf "Done.\n"
	@printf "Linking $(THIS_TREE)..."
	@$(CC) $(LD_OPTS) -o netmirror main.o sender.o receiver.o sink.o nethelp.o ../lib/libmb64.a
	@printf "Done.\n"

ccstart:
	@printf "Compiling $(THIS_TREE)..."

main.o: main.c sender.h receiver.h sink.h nethelp.h ../version.h
	@$(CC) $(CC_OPTS) -c main.c
	@printf "."

sender.o: sender.c sender.h nethelp.h
	@$(CC) $(CC_OPTS) -c sender.c
	@printf "."

receiver.o: receiver.c receiver.h nethelp.h
	@$(CC) $(CC_OPTS) -c receiver.c
	@printf "."

sink.o: sink.c sink.h nethelp.h
	@$(CC) $(CC_OPTS) -c sink.c
	@printf "."

nethelp.o: nethelp.c nethelp.h
	@$(CC) $(CC_OPTS) -c nethelp.c
	@printf "."

lean:
	@printf "Leaning $(THIS_TREE)."
	@rm -f *.o
	@printf "."
	@rm -f *~
	@printf "."
	@rm -f core
	@printf "."
	@printf "Done.\n"

clean: lean
	@printf "Cleaning $(THIS_TREE)."
	@rm -f boomerang
	@rm -f netbounce
	@printf "."
	@printf "Done.\n"
