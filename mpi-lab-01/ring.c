#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#define MASTER 0

int main(int argc, char * argv[])
{
	int numProcesses, myRank;
	unsigned long token;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	
	// Jeżeli nie jestem rootem, czekam na wiadomość
	
	if(myRank == 0){
		token = 1;
		MPI_Send(&token, 1, MPI_LONG, (myRank + 1) % numProcesses , 0, MPI_COMM_WORLD);
		MPI_Recv(&token, 1, MPI_LONG, numProcesses - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("MASTER received token %li form process %d\n", token, numProcesses - 1);
	} else {
		MPI_Recv(&token, 1, MPI_LONG, myRank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("Process %d received token %li form process %d\n", myRank, token, myRank - 1);
		
		token *= myRank;
		
		MPI_Send(&token, 1, MPI_LONG, (myRank + 1) % numProcesses , 0, MPI_COMM_WORLD);
		printf("Process %d send token %li to process %d\n", myRank, token, (myRank + 1) % numProcesses);
	}
	
	
   	MPI_Finalize();
    return 0;
}
