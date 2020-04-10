#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#define MAX_PAYLOAD
#define SAMPLE_SIZE 30

double benchmark(void* buff, int to, int payload){
    double startTime,endTime;

    startTime = MPI_Wtime();
    MPI_Send(buff, payload, MPI_BYTE, to, 0, MPI_COMM_WORLD);
    MPI_Recv(buff, payload, MPI_BYTE, to, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    endTime = MPI_Wtime();
    return endTime - startTime;
}

int main(int argc, char * argv[]){
	MPI_Init(&argc, &argv);
	
	int numProcesses, myRank;

    MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    int node = myRank/(numProcesses/2);
    int partnerNode, partnerRank;
    if(node == 0){
        partnerNode = 1;
        partnerRank = myRank + (numProcesses/2);
    } else {
        partnerNode = 0;
        partnerRank = myRank - (numProcesses/2);
    }
    
    int max_payload = MAX_PAYLOAD;
    void* buff = malloc(max_payload);
    
    if(node == 0){	
		for(int i = 0 ; i < max_payload ; i *= 10){
            MPI_Recv(buff, i, MPI_BYTE, partnerRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(buff, i, MPI_BYTE, partnerRank, 0, MPI_COMM_WORLD);
		}
	} else {
		for(int i = 0 ; i < max_payload ; i *= 10){
			double benchmarkTime = benchmark(buff, partnerRank, i);
			pritnf("%d %d %lf\n", partnerRank, i, benchmarkTime);
		}
	}
	free(buff);
	MPI_Finalize();
	return 0;
}
