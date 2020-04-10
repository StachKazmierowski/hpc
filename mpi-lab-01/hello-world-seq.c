/*
 * A template for the 2016 MPI lab at the University of Warsaw.
 * Copyright (C) 2016, Konrad Iwanicki
 * Further modifications by Krzysztof Rzadca 2018
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#define MASTER 0

int main(int argc, char * argv[])
{
	int numProcesses, myRank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    
    struct timespec spec;
//    clock_gettime(CLOCK_REALTIME, &spec);
//    srand(spec.tv_nsec); // use nsec to have a different value across different processes
    
//    unsigned t = rand() % 5;
//    sleep(t);
    printf("Hello from thread number %d", myRank);
    if(myRank == MASTER)
    	printf("MASTER: number of tasks is: %d", numProcesses);
    return 0;
}
