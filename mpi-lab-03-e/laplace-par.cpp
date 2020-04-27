/*
 * A template for the 2019 MPI lab at the University of Warsaw.
 * Copyright (C) 2016, Konrad Iwanicki.
 * Refactoring 2019, Łukasz Rączkowski
 */

#include <iostream>
#include <iomanip>
#include <cstring>
#include <sys/time.h>
#include <mpi.h>
#include "laplace-common.h"

#define OPTION_VERBOSE "--verbose"

static void printUsage(char const* progName) {
    std::cerr << "Usage:" << std::endl <<
              "    " << progName << " [--verbose] <N>" << std::endl <<
              "Where:" << std::endl <<
              "   <N>         The number of points in each dimension (at least 4)." << std::endl <<
              "   " << OPTION_VERBOSE << "   Prints the input and output systems." << std::endl;
}

static InputOptions parseInput(int argc, char * argv[], int numProcesses) {
    int numPointsPerDimension = 0;
    bool verbose = false;
    int errorCode = 0;

    if (argc < 2) {
        std::cerr << "ERROR: Too few arguments!" << std::endl;
        printUsage(argv[0]);
        errorCode = 1;
        MPI_Finalize();
    } else if (argc > 3) {
        std::cerr << "ERROR: Too many arguments!" << std::endl;
        printUsage(argv[0]);
        errorCode = 2;
        MPI_Finalize();
    } else {
        int argIdx = 1;

        if (argc == 3) {
            if (strncmp(argv[argIdx], OPTION_VERBOSE, strlen(OPTION_VERBOSE)) != 0) {
                std::cerr << "ERROR: Unexpected option '" << argv[argIdx] << "'!" << std::endl;
                printUsage(argv[0]);
                errorCode = 3;
                MPI_Finalize();
            }
            verbose = true;
            ++argIdx;
        }

        numPointsPerDimension = std::strtol(argv[argIdx], nullptr, 10);

        if ((numPointsPerDimension < 4) || (numProcesses > numPointsPerDimension / 2)) {
            /* If we had a smaller grid, we could use the sequential version. */
            std::cerr << "ERROR: The number of points, '"
                << argv[argIdx]
                << "', should be an iteger greater than or equal to 4; and at least 2 points per process!"
                << std::endl;
            printUsage(argv[0]);
            MPI_Finalize();
            errorCode = 4;
        }
    }

    return {numPointsPerDimension, verbose, errorCode};
}

static void sendRecv(int myRank, int numProcesses, double* buffFrom, double* buffTo, int buffSize, bool clockwise, MPI_Request* reqs){
    int rankFrom, rankTo;
    if(clockwise){
        rankTo = (myRank + 1) % numProcesses;
        rankFrom = myRank == 0 ? numProcesses - 1 : myRank - 1;
    } else {
        rankFrom = (myRank + 1) % numProcesses;
        rankTo = myRank == 0 ? numProcesses - 1 : myRank - 1;
    }
    MPI_Isend(buffFrom, buffSize, MPI_DOUBLE, rankTo, clockwise, MPI_COMM_WORLD, reqs);
    MPI_Irecv(buffTo, buffSize, MPI_DOUBLE, rankFrom, clockwise, MPI_COMM_WORLD, reqs + 1);
}

static std::tuple<int, double> performAlgorithm(int myRank, int numProcesses, GridFragment *frag, double omega, double epsilon) {
    int startRowIncl = frag->firstRowIdxIncl + (myRank == 0 ? 1 : 0);
    int endRowExcl = frag->lastRowIdxExcl - (myRank == numProcesses - 1 ? 1 : 0);

    double maxDiff = 0;
    int numIterations = 0;

    /* TODO: change the following code fragment */
    /* Implement asynchronous communication of neighboring elements */
    /* and computation of the grid */
    /* the following code just recomputes the appropriate grid fragment */
    /* but does not communicate the partial results */
    
    
    auto reqs = new MPI_Request[4];
    
    
    do {
        maxDiff = 0.0;

        for (int color = 0; color < 2; ++color) {
        	
        	int otherColor = (color + 1) % 2;
        	int rowsNumber = frag->lastRowIdxExcl - frag->firstRowIdxIncl;
        	double* sharedRowTop = frag->data[otherColor][0];
        	double* myRowTop = frag->data[otherColor][1];
        	double* myRowBottom = frag->data[otherColor][rowsNumber];
        	double* sharedRowBottom = frag->data[otherColor][rowsNumber + 1];
        	
        	int buffSize = (frag->gridDimension +1) / 2;
        	sendRecv(myRank, numProcesses, myRowTop, sharedRowTop, buffSize, false, reqs);
        	sendRecv(myRank, numProcesses, myRowBottom, sharedRowBottom, buffSize, true, reqs + 2);
        	
        	
        	
            for (int rowIdx = startRowIncl; rowIdx < endRowExcl; ++rowIdx) {
                for (int colIdx = 1 + (rowIdx % 2 == color ? 1 : 0); colIdx < frag->gridDimension - 1; colIdx += 2) {
                    double tmp =
                            (GP(frag, rowIdx - 1, colIdx) +
                             GP(frag, rowIdx + 1, colIdx) +
                             GP(frag, rowIdx, colIdx - 1) +
                             GP(frag, rowIdx, colIdx + 1)
                            ) / 4.0;
                    double diff = GP(frag, rowIdx, colIdx);
                    GP(frag, rowIdx, colIdx) = (1.0 - omega) * diff + omega * tmp;
                    diff = fabs(diff - GP(frag, rowIdx, colIdx));

                    if (diff > maxDiff) {
                        maxDiff = diff;
                    }
                }
            }
            MPI_Waitall(4, reqs, MPI_STATUSES_IGNORE);
        }

        ++numIterations;
        double globalMaxDiff = maxDiff;

        MPI_Allreduce(
                &maxDiff,
                &globalMaxDiff,
                1,
                MPI_DOUBLE,
                MPI_MAX,
                MPI_COMM_WORLD);


        maxDiff = globalMaxDiff;
    } while (maxDiff > epsilon);
    
    delete[] reqs;
    /* no code changes beyond this point should be needed */

    return std::make_tuple(numIterations, maxDiff);
}

int main(int argc, char *argv[]) {
    int numProcesses;
    int myRank;
    struct timeval startTime {};
    struct timeval endTime {};

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

    auto inputOptions = parseInput(argc, argv, numProcesses);

    if (inputOptions.getErrorCode() != 0) {
        return inputOptions.getErrorCode();
    }
    auto isVerbose = inputOptions.isVerbose();
    
    for(int i = 100; i < 2000; i += 100){

    auto numPointsPerDimension = i;

    double omega = Utils::getRelaxationFactor(numPointsPerDimension);
    double epsilon = Utils::getToleranceValue(numPointsPerDimension);

    auto gridFragment = new GridFragment(numPointsPerDimension, numProcesses, myRank);
    gridFragment->initialize();

    if (gettimeofday(&startTime, nullptr)) {
        gridFragment->free();
        std::cerr << "ERROR: Gettimeofday failed!" << std::endl;
        MPI_Finalize();
        return 6;
    }

    /* Start of computations. */

    auto result = performAlgorithm(myRank, numProcesses, gridFragment, omega, epsilon);

    /* End of computations. */

    if (gettimeofday(&endTime, nullptr)) {
        gridFragment->free();
        std::cerr << "ERROR: Gettimeofday failed!" << std::endl;
        MPI_Finalize();
        return 7;
    }

    double duration =
            ((double) endTime.tv_sec + ((double) endTime.tv_usec / 1000000.0)) -
            ((double) startTime.tv_sec + ((double) startTime.tv_usec / 1000000.0));

    std::cerr << "Statistics: duration(s)="
              << std::fixed
              << std::setprecision(10)
              << duration << " #iters="
              << std::get<0>(result)
              << " diff="
              << std::get<1>(result)
              << " epsilon="
              << epsilon
              << std::endl;
    std::cout << i << " " << duration << " " << epsilon << std::endl;
    if (isVerbose) {
        gridFragment->printEntireGrid(myRank, numProcesses);
    }

    gridFragment->free();   
    }


    MPI_Finalize();
    return 0;
}




