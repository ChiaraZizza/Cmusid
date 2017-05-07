SOURCES = main.c fingerprinting.c
HEADERS = fingerprinting.h

all:

check:

clean:
	rm -rf *.o

distclean: clean
	rm -f cmusid

cmusid: main.c cmusid_fingerprinting
	clang main.c -g -lcurl -lpthread -o cmusid

cmusid_fingerprinting: fingerprinting.c
	clang fingerprinting.c -g -L chromaprint-1.4.2/src -I chromaprint-1.4.2/src/ -lchromaprint -o cmusid_fp.o

