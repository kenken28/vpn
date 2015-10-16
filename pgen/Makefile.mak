# Makefile for Windows MSVC 5.0:  nmake -f Makefile.mak

cflags = -nologo -c -W3 -DWIN32 -D_CONSOLE -D_MBCS -Z7 -Od /FD /ML
lflags = /NODEFAULTLIB:libc
gflags = -align:0x1000 -subsystem:console
glibs  = libcmt.lib user32.lib gdi32.lib comdlg32.lib wsock32.lib vfw32.lib
objs   = bigint.obj hexdump.obj skein.obj skein_block.obj \
	 base64.obj skrand.obj crc.obj avlc.obj winstring.obj \
	 getopt2.obj

all: pgen.exe pmerge.exe proots.exe ptest.exe pseq.exe pgenp.exe \
	bigint_test.exe exchange_test.exe

avlc.obj: avlc.c avlc.h
    cl $(cflags) avlc.c

crc.obj: crc.c crc.h
    cl $(cflags) crc.c

winstring.obj: winstring.c winstring.h
    cl $(cflags) winstring.c

getopt2.obj: getopt2.c getopt2.h
    cl $(cflags) getopt2.c

bigint_test.obj: bigint_test.c bigint.h
    cl $(cflags) bigint_test.c

hexdump.obj: hexdump.c hexdump.h
    cl $(cflags) hexdump.c

skrand.obj: skrand.c skrand.h
    cl $(cflags) skrand.c

bigint.obj: bigint.c bigint.h
    cl $(cflags) bigint.c

pgen.obj: pgen.c
    cl $(cflags) pgen.c

pgenp.obj: pgenp.c
    cl $(cflags) pgenp.c

pmerge.obj: pmerge.c
    cl $(cflags) pmerge.c

proots.obj: proots.c
    cl $(cflags) proots.c

exchange_test.obj: exchange_test.c
    cl $(cflags) exchange_test.c

pgen.exe: pgen.obj $(objs)
    link $(lflags) $(gflags) pgen.obj $(objs) $(glibs) -out:pgen.exe

pgenp.exe: pgenp.obj $(objs)
    link $(lflags) $(gflags) pgenp.obj $(objs) $(glibs) -out:pgenp.exe

pmerge.exe: pmerge.obj $(objs)
    link $(lflags) $(gflags) pmerge.obj $(objs) $(glibs) -out:pmerge.exe

proots.exe: proots.obj $(objs)
    link $(lflags) $(gflags) proots.obj $(objs) $(glibs) -out:proots.exe

ptest.exe: ptest.obj $(objs)
    link $(lflags) $(gflags) ptest.obj $(objs) $(glibs) -out:ptest.exe

pseq.exe: pseq.obj $(objs)
    link $(lflags) $(gflags) pseq.obj $(objs) $(glibs) -out:pseq.exe

bigint_test.exe: bigint_test.obj $(objs)
    link $(lflags) $(gflags) bigint_test.obj $(objs) $(glibs) -out:bigint_test.exe

exchange_test.exe: exchange_test.obj $(objs)
    link $(lflags) $(gflags) exchange_test.obj $(objs) $(glibs) -out:exchange_test.exe
