#!/bin/ksh93

print "mymain32 opens without preload:"
/usr/bin/truss -af -t kopen ./mymain32

print "======"
print "mymain32 opens with preload:"
LDR_PRELOAD='./libinterposer.a(opendio32.o)' /usr/bin/truss -af -t kopen ./mymain32

print "======"
print "mymain64 opens without preload:"
/usr/bin/truss -af -t kopen ./mymain64

print "======"
print "mymain64 opens with preload:"
LDR_PRELOAD64='./libinterposer.a(opendio64.o)' /usr/bin/truss -af -t kopen ./mymain64
