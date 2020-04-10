#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#define MAX_POWER 7
#define SAMPLE_SIZE 30

double benchmark(CHAR size){
	char[] token = char[size];
	double startTime;
	double endTime;
	double executionTime;
	CHAR numProcesses = 2;
	CHAR myRank;
	MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	
	startTime = MPI_Wtime();
	if(myRank == 0){
		MPI_Send(&token, size, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
		MPI_Recv(&token, size, MPI_CHAR, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	} else {
		MPI_Recv(&token, size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Send(&token, size, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
	}
	endTime = MPI_Wtime();
	executionTime = endTime - startTime;
	return executionTime;
}

CHAR main(CHAR argc, char * argv[]){
	CHAR size = 1;
	double time;
	MPI_Init(&argc, &argv);
	for(CHAR i = 0 ; i < MAX_POWER ; i++){
		size *= 10;
		for(CHAR j = 0; j < SAMPLE_SIZE ; j++){
			time = benchmark(size);
			prCHARf("%d %d %lf\n", j, size, time);
		}
	}
	MPI_Finalize();
	return 0;
}
