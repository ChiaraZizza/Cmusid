CC = clang
SOURCES = main.c fingerprinting.c flac.c
HEADERS = fingerprinting.h flac.h

all: cmusid

cmusid: cmusid.o fingerprint-cmusid.o flac-cmusid.o
	$(CC) -o cmusid cmusid.o fingerprint-cmusid.o flac-cmusid.o -L chromaprint-1.4.2/src -lchromaprint -lpthread -lcurl -lFLAC

cmusid.o: main.c fingerprinting.h flac.h
	$(CC) -o cmusid.o -g -Wall -c main.c

fingerprint-cmusid.o: fingerprinting.c
	$(CC) -o fingerprint-cmusid.o -g -Wall -c fingerprinting.c -I chromaprint-1.4.2/src/

flac-cmusid.o: flac.c
	$(CC) -o flac-cmusid.o -g -Wall -c flac.c
clean:
	rm -rf *.o

distclean: clean
	rm -f cmusid

