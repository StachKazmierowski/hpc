#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#define MAX_POWER 7
#define SAMPLE_SIZE 30

double benchmark(int size, int argc, char* argv[]){
	int token;
	double startTime;
	double endTime;
	double executionTime;
	int numProcesses = 2;
	int myRank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	
	startTime = MPI_Wtime();
	if(myRank == 0){
		MPI_Send(&token, size, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
		MPI_Recv(&token, size, MPI_BYTE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	} else {
		MPI_Recv(&token, size, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Send(&token, size, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
	}
	endTime = MPI_Wtime();
	executionTime = endTime - startTime;
   	MPI_Finalize();
	return executionTime;
}

int main(int argc, char * argv[]){
int size = 1;
double time;
for(int i = 0 ; i < MAX_POWER ; i++){
	size *= 10;
	for(int j = 0; j < SAMPLE_SIZE ; j++){
		time = benchmark(size, argc, argv);
		printf("%d %d %lf\n", j, size, time);
	}
}
return 0;
}
