#
#       FILE NAME: Makefile
#
#       FILE DESCRIPTION: Make description file for building example programs
#
#	PROJECT NAME: Linux Software (aDIO)
#
#       PROJECT VERSION: (Defined in README.TXT)
#
#       Copyright 2004 RTD Embedded Technologies, Inc.  All Rights Reserved.
#

CC=gcc
DEBUG_FLAGS=-g
INCLUDE_FLAGS=-I../include
LIBRARY_FLAGS=-L../lib -lpthread
OPTIMIZE_FLAGS=
WARNING_FLAGS=-Wall
CFLAGS:=$(DEBUG_FLAGS) $(INCLUDE_FLAGS) $(OPTIMIZE_FLAGS) $(WARNING_FLAGS)

ADIO_LIBRARY=-lrtd-aDIO

EXAMPLES=ports \
	ints

all:	$(EXAMPLES)

ports:	ports.c
	$(CC) $(CFLAGS) -o ports -lpthread ports.c $(LIBRARY_FLAGS) $(ADIO_LIBRARY)

ints:	ints.c
	$(CC) $(CFLAGS) -o ints -lpthread ints.c $(LIBRARY_FLAGS) $(ADIO_LIBRARY)

clean:
	rm -f *.o *~ $(EXAMPLES)
