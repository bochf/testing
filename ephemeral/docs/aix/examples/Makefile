.PHONY: gmake all clean lean

# Set this to change bitness (of a few commands) to 64 bit
BITNESS=
#-maix64

all: mmap_file shmat_file mmap_shared mmap_file_append shmat_file_append mk_bigdata_file shmat_bigdata malloc_segments malloc_meg shmat_limit malloc_meg_MD

# Example code in the document (mmap()s a file, reads data, exits)
mmap_file: mmap_file.c
	gcc -Wall -Werror -o mmap_file mmap_file.c

# Example code in the document (shmat() variant of mmap_file)
shmat_file: shmat_file.c
	gcc -Wall -Werror -o shmat_file shmat_file.c

mmap_shared: mmap_shared.c
	gcc -Wall -Werror -o mmap_shared mmap_shared.c

mmap_file_append: mmap_file_append.c
	gcc -Wall -Werror -o mmap_file_append mmap_file_append.c

shmat_file_append: shmat_file_append.c
	gcc -Wall -Werror -o shmat_file_append shmat_file_append.c

# This simply builds large (segment sized) files to be used for shmat_bigdata 
mk_bigdata_file: mk_bigdata_file.c
	gcc -Wall -Werror -o mk_bigdata_file mk_bigdata_file.c

# shmat()s a segment sized file into memory, then does it again with a slightly
# larger file. This requires that mk_bigdata_file be called first.
shmat_bigdata: shmat_bigdata.c
	gcc -Wall -Werror -o shmat_bigdata shmat_bigdata.c

# gcc does not accept this as an option - so I use ldedit
malloc_meg_MD: malloc_meg
	cp malloc_meg malloc_meg_MD
	ldedit -bmaxdata:0x20000000 malloc_meg_MD

malloc_segments: malloc_segments.c
	gcc -Wall -Werror $(BITNESS) -o malloc_segments malloc_segments.c

# malloc() 1M requests until we fail - Specify # of meg requests as option
malloc_meg: malloc_meg.c
	gcc -Wall -Werror $(BITNESS) -o malloc_meg malloc_meg.c

# Continue to create/map SysV segments until we fail - Specify desired count as option
shmat_limit: shmat_limit.c
	gcc -Wall -Werror $(BITNESS) -o shmat_limit shmat_limit.c

lean:
	rm -f core

clean: lean
	rm -f mmap_file shmat_file mmap_shared mmap_file_append
	rm -f shmat_file_append mk_bigdata_file shmat_bigdata
	rm -f malloc_segments malloc_meg shmat_limit malloc_meg_MD
	rm -f BIG_DATA
	rm -f BIG_DATA_plus4k

gmake:
	@echo "ERROR: This is not a make file. Use gmake to build."
	@false
