PROJECT = cmusid
VERSION = 0.1

CC = clang #-fsanitize=address

HEADERS = fingerprinting.h flac.h metadata.h
SOURCES = main.c fingerprinting.c flac.c metadata.h
MORESRC = Makefile chromaprint-1.4.2/ input/
PRJDIST = $(SOURCES) $(HEADERS) $(MORESRC)

PROJDIR = $(PROJECT)-$(VERSION)
TARBALL = $(PROJDIR).tgz

all: cmusid

dist: $(PRJDIST)
	rm -rf $(PROJDIR)
	mkdir $(PROJDIR)
	cp -r $^ $(PROJDIR)
	tar cvzf $(TARBALL) $(PROJDIR)
	rm -rf $(PROJDIR)

cmusid: cmusid.o fingerprint-cmusid.o flac-cmusid.o
	$(CC) -o cmusid cmusid.o fingerprint-cmusid.o flac-cmusid.o -L chromaprint-1.4.2/src -lchromaprint -lpthread -lcurl -lFLAC

cmusid.o: main.c  $(HEADERS)
	$(CC) -o cmusid.o -g -Wall -c main.c

fingerprint-cmusid.o: fingerprinting.c
	$(CC) -o fingerprint-cmusid.o -g -Wall -c fingerprinting.c -I chromaprint-1.4.2/src/

metadata-cmusid.o: metadata.c
	$(CC) -o metadata-cmusid.o -g -Wall -c metadata.c

flac-cmusid.o: flac.c
	$(CC) -o flac-cmusid.o -g -Wall -c flac.c

clean:
	rm -rf *.o

distclean: clean
	rm -f cmusid
	rm -f $(TARBALL)

