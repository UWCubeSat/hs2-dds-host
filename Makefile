SDIR=./src
IDIR=./lib/include
ODIR=./obj

CC=gcc
CFLAGS=-g -Wall

INCLUDES=-I$(IDIR) -I/usr/include/hidapi

LIBS=-lhidapi-libusb

SOURCES=dds-host.c

EXE=dds-host

all: $(EXE)

$(EXE): $(SDIR)/$(SOURCES)
	$(CC) $(CFLAGS) -o bin/$(EXE) $(SDIR)/$(SOURCES) $(INCLUDES) $(LIBS)

.PHONY: clean

clean:
	rm -f bin/$(EXE)