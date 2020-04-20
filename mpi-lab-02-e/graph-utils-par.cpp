/*
 * A template for the 2019 MPI lab at the University of Warsaw.
 * Copyright (C) 2016, Konrad Iwanicki.
 * Refactoring 2019, Łukasz Rączkowski
 */

#include <iostream>
#include <cassert>
#include <mpi.h>
#include "graph-base.h"
#include "graph-utils.h"

int getFirstGraphRowOfProcess(int numVertices, int numProcesses, int myRank) {
	int perProcess = numVertices / numProcesses;
	int unevenNumber = numVertices - perProcess * numProcesses;
	if(unevenNumber >= myRank){
		return myRank * perProcess + myRank;
	} else {
		return myRank * perProcess + unevenNumber;
	}
}

Graph* createAndDistributeGraph(int numVertices, int numProcesses, int myRank) {
    assert(numProcesses >= 1 && myRank >= 0 && myRank < numProcesses);

	int start = getFirstGraphRowOfProcess(numVertices, numProcesses, myRank);
    int end = getFirstGraphRowOfProcess(numVertices, numProcesses, myRank + 1);
    auto graph = allocateGraphPart(
            numVertices,
            start,
            end
    );

    if (graph == nullptr) {
        return nullptr;
    }

    assert(graph->numVertices > 0 && graph->numVertices == numVertices);
    assert(graph->firstRowIdxIncl >= 0 && graph->lastRowIdxExcl <= graph->numVertices);
    
    if(myRank == 0){
//    	auto graphToSend = allocateGraphPart(numVertices, 0, numVertices);

        int recipientRank = 1;
        int partStart = getFirstGraphRowOfProcess(numVertices, numProcesses, recipientRank);
        int partEnd = getFirstGraphRowOfProcess(numVertices, numProcesses, recipientRank + 1) - 1;

        int *row = new int[numVertices];
        
        for (int i = 0; i < graph->numVertices; ++i) {
            if(i < partStart){
                initializeGraphRow(graph->data[i], i, graph->numVertices);
            } else {
                initializeGraphRow(row, i, numVertices);
//                MPI_Request request;
                MPI_Send(row,
                        numVertices,
                        MPI_INT,
                        recipientRank,
                        0,
                        MPI_COMM_WORLD
//                        ,&request
                        );
            }
            if(i == partEnd){
                recipientRank++;
                partStart = getFirstGraphRowOfProcess(numVertices, numProcesses, recipientRank);
                partEnd = getFirstGraphRowOfProcess(numVertices, numProcesses, recipientRank + 1) - 1;
            }
        }
//    	freeGraphPart(graphToSend);
    	MPI_Barrier(MPI_COMM_WORLD);
    	delete[] row;
    } else {
    	int rowNumber = end - start;
   		for(int j = 0; j < rowNumber; j++){
   			MPI_Recv(graph->data[j], numVertices, MPI_INTEGER, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
   		}
   		MPI_Barrier(MPI_COMM_WORLD);
    }
    return graph;
}

void collectAndPrintGraph(Graph* graph, int numProcesses, int myRank) {
    assert(numProcesses >= 1 && myRank >= 0 && myRank < numProcesses);
    assert(graph->numVertices > 0);
    assert(graph->firstRowIdxIncl >= 0 && graph->lastRowIdxExcl <= graph->numVertices);
    int numVertices = graph->numVertices;
    int firstRow = getFirstGraphRowOfProcess(numVertices, numProcesses, myRank);
    int lastRow = getFirstGraphRowOfProcess(numVertices, numProcesses, myRank + 1);
    int rowsNumber = lastRow - firstRow;
    
    int maxRowsNumber = (numVertices + numProcesses - 1)/numProcesses;
    
    int* recv_data ;
    int* send_data ;

    send_data = new int[numVertices*maxRowsNumber];
    
    for(int i=0; i<rowsNumber; i++){
        for(int j=0; j < numVertices; j++){
            send_data[i*numVertices + j] = graph->data[i][j];
        }
    }
    
    if(myRank == 0){
        recv_data = new int[numVertices*maxRowsNumber*numProcesses];
    }
    
    MPI_Gather(
            send_data,
            numVertices*maxRowsNumber,
            MPI_INT,
            recv_data,
            numVertices*maxRowsNumber,
            MPI_INT,
            0,
            MPI_COMM_WORLD);
    
    if(myRank == 0){
    	for(int i = 0; i < numProcesses; i++){
    		int firstRow = getFirstGraphRowOfProcess(numVertices, numProcesses, i);
    		int lastRow = getFirstGraphRowOfProcess(numVertices, numProcesses, i + 1);
   			for(int j = 0; j < (lastRow - firstRow); j++){
            	printGraphRow(recv_data + numVertices * (i * maxRowsNumber + j),0, numVertices );
   			}
   		}
        delete[] recv_data;
    }

    delete[] send_data;
}

void destroyGraph(Graph* graph, int numProcesses, int myRank) {
    freeGraphPart(graph);
}
