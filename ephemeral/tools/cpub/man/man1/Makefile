.PHONY: GMAKE clean lean solaris linux aix

default:
	@echo "Specify a target: aix, solaris, or linux."

solaris: solaris.man

linux: linux.man

aix: aix.man

solaris.man: cpub.1.head cpub.1.solprov cpub.1.tail
	@printf "Building the Solaris man1 page..."
	@cat cpub.1.head > solaris.man
	@cat cpub.1.solprov >> solaris.man
	@cat cpub.1.tail >> solaris.man
	@cat solaris.man > cpub.1
	@printf "Done.\n"

linux.man: cpub.1.head cpub.1.linprov cpub.1.tail
	@printf "Building the Linux man1 page..."
	@cat cpub.1.head > linux.man
	@cat cpub.1.linprov >> linux.man
	@cat cpub.1.tail >> linux.man
	@cat linux.man > cpub.1
	@printf "Done.\n"

aix.man: cpub.1.head cpub.1.aixprov cpub.1.tail
	@printf "Building the AIX man1 page..."
	@cat cpub.1.head > aix.man
	@cat cpub.1.aixprov >> aix.man
	@cat cpub.1.tail >> aix.man
	@cat aix.man > cpub.1
	@printf "Done.\n"

GMAKE:
	@echo "Whoa cowboy... You should know by now that this needs gmake!" >&2
	@echo
	@echo
	@false

lean:
	@printf "Leaning up the man1 build directory"
	@rm -f solaris.man
	@printf "."
	@rm -f linux.man
	@printf "."
	@rm -f aix.man
	@printf "."
	@rm -f *~
	@printf "Done.\n"

clean: lean
	@printf "Cleaning up the man1 build directory..."
	@rm -f cpub.1
	@printf "Done.\n"

