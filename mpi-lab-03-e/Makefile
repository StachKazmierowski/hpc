#
# A template for the 2016 MPI lab at the University of Warsaw.
# Copyright (C) 2016, Konrad Iwanicki.
#
CC          := mpicc
CFLAGS      := -O3 -c -Wall
LFLAGS      := -O3
ALL         := \
	scatter-gather-max.exe \
	ring-nonblocking.exe \
	laplace-seq.exe \
	laplace-par.exe

all : $(ALL)


%.exe : %.o
	$(CC) $(LFLAGS) -o $@ $< -lm

laplace-%.o : laplace-%.c laplace-common.h laplace-common-impl.h Makefile
	$(CC) $(CFLAGS) $<

%.o : %.c
	$(CC) $(CFLAGS) $<

clean :
	rm -f *.o *.out *.err $(ALL)

