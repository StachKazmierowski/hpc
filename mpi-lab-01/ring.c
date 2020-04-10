#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#define MASTER 0

int main(int argc, char * argv[])
{
	int numProcesses, myRank;
	int token = 1;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	
	// Jeżeli nie jestem rootem, czekam na wiadomość
	if(myRank != 0){
		MPI_Recv(&token, 1, MPI_INT, myRank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("Process %d received token %d form process %d\n", myRank, token, myRank - 1);
	}
	token *= myRank;
	MPI_Send(&token, 1, MPI_INT, (myRank + 1) % numProcesses , 0, MPI_COMM_WORLD);
	
	if(myRank == 0){
	MPI_Recv(&token, 1, MPI_INT, numProcesses - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("MASTER received token %d form process %d\n", token, myRank - 1);
	}
   	MPI_Finalize();
    return 0;
}
