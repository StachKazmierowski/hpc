/*
 * A template for the 2019 MPI lab at the University of Warsaw.
 * Copyright (C) 2016, Konrad Iwanicki.
 * Refactoring 2019, Łukasz Rączkowski
 */

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

    auto graph = allocateGraphPart(
            numVertices,
            getFirstGraphRowOfProcess(numVertices, numProcesses, myRank),
            getFirstGraphRowOfProcess(numVertices, numProcesses, myRank + 1)
    );

    if (graph == nullptr) {
        return nullptr;
    }

    assert(graph->numVertices > 0 && graph->numVertices == numVertices);
    assert(graph->firstRowIdxIncl >= 0 && graph->lastRowIdxExcl <= graph->numVertices);
    
    if(myRank == 0){
    	auto graphToSend = allocateGraphPart(numVertices, 0, numVertices);

    	if (graphToSend == nullptr) {
    	    return nullptr;
    	}

    	assert(graphToSend->numVertices > 0 && graphToSend->numVertices == numVertices);
    	assert(graphToSend->firstRowIdxIncl == 0 && graphToSend->lastRowIdxExcl == graphToSend->numVertices);

    	for (int i = 0; i < graph->numVertices; ++i) {
    	    initializeGraphRow(graphToSend->data[i], i, graphToSend->numVertices);
    	}
    	
    	int firstRowOfNext = getFirstGraphRowOfProcess(numVertices, numProcesses, 1);
    	for(int i = 0; i < firstRowOfNext; i++){
    		graph->data[i] = graphToSend->data[i];
    	}
    	
    	for(int i = 1; i < numProcesses; i++){
    		int firstRow = getFirstGraphRowOfProcess(numVertices, numProcesses, i);
    		int lastRow = getFirstGraphRowOfProcess(numVertices, numProcesses, i+1);
    		for(int j = 0; j < (lastRow - firstRow); j++){
    			MPI_Send(graphToSend->data[firstRow + j], numVertices, MPI_INTEGER, i, 0, MPI_COMM_WORLD);
    		} 
    	}
    } else {
    		int firstRow = getFirstGraphRowOfProcess(numVertices, numProcesses, myRank);
    		int lastRow = getFirstGraphRowOfProcess(numVertices, numProcesses, myRank + 1);
    		for(int j = 0; j < (lastRow - firstRow); j++){
    			MPI_Recv(graph->data[j], numVertices, MPI_INTEGER, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    		}
    }
    return graph;
}

void collectAndPrintGraph(Graph* graph, int numProcesses, int myRank) {
    assert(numProcesses >= 1 && myRank >= 0 && myRank < numProcesses);
    assert(graph->numVertices > 0);
    assert(graph->firstRowIdxIncl >= 0 && graph->lastRowIdxExcl <= graph->numVertices);
    int numVertices = graph->numVertices;
    
    if(myRank == 0){
    	auto graphToReceive = allocateGraphPart(numVertices, 0, numVertices);

    	if (graph == nullptr) {
    	    return;
    	}

    	assert(graphToReceive->numVertices > 0 && graphToReceive->numVertices == numVertices);
    	assert(graphToReceive->firstRowIdxIncl == 0 && graphToReceive->lastRowIdxExcl == graphToReceive->numVertices);
    	
    	for(int i = 0; i < getFirstGraphRowOfProcess(numVertices, numProcesses, myRank + 1); i++){
   			graphToReceive->data[i] = graph->data[i];
    	}
    	
    	for(int i = 1; i < numProcesses; i++){
    		int firstRow = getFirstGraphRowOfProcess(numVertices, numProcesses, i);
    		int lastRow = getFirstGraphRowOfProcess(numVertices, numProcesses, i+1);
    		for(int j = 0; j < (lastRow - firstRow); j++){
    			MPI_Recv(graphToReceive->data[firstRow + j], numVertices, MPI_INTEGER, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    		}
    	}
    	
        for (int i = 0; i < graph->numVertices; i++) {
        	printGraphRow(graph->data[i], i, graph->numVertices);
    	}
    	
    } else {
    		int firstRow = getFirstGraphRowOfProcess(numVertices, numProcesses, myRank);
    		int lastRow = getFirstGraphRowOfProcess(numVertices, numProcesses, myRank + 1);
    		for(int j = 0; j < (lastRow - firstRow); j++){
    			MPI_Send(graph->data[j], numVertices, MPI_INTEGER, 0, 0, MPI_COMM_WORLD);
    		}
    }
}

void destroyGraph(Graph* graph, int numProcesses, int myRank) {
    freeGraphPart(graph);
}
