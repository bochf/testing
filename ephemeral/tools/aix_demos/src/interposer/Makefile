CC=/bb/util/version12-102014/usr/vac/bin/xlc

.PHONY: all clean test

all: mymain32 mymain64 libinterposer.a

mymain32: mymain32.o
	$(CC) -q32 -g -o $@ $<

mymain64: mymain64.o
	$(CC) -q64 -g -o $@ $<

libinterposer.a: opendio32.o opendio64.o
	rm -f $@
	ar -Xany qv $@ $^
	#ranlib $@

opendio32.o:  interposer-open.c opendio32.exp
	$(CC) -q32 -qmkshrobj -bE:opendio32.exp -g -o $@ $<

opendio64.o:  interposer-open.c opendio64.exp
	$(CC) -q64 -qmkshrobj -bE:opendio64.exp -g -o $@ $<

#opendio32.o: interposer-open32.o
#	ld -b32 -o $@ $< -bM:SRE -bE:opendio32.exp -bnoentry -lc
#
#opendio64.o: interposer-open64.o
#	ld -b64 -o $@ $< -bM:SRE -bE:opendio64.exp -bnoentry -lc
#
#interposer-open32.o:  interposer-open.c 
#	$(CC) -q32 -g -o $@ -c $<
#
#interposer-open64.o:  interposer-open.c 
#	$(CC) -q64 -g -o $@ -c $<

mymain32.o:  mymain.c
	$(CC) -q32 -g -o $@ -c $<

mymain64.o:  mymain.c
	$(CC) -q64 -g -o $@ -c $<

clean: 
	rm -f mymain32 mymain64 libinterposer.a *.o

test:
	./test.sh
