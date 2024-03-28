#include <stdio.h>
#include "mpi.h"
#include <chrono>
#include <iostream>

int main(int argc, char** argv)
{
	const int MAX = 1000;
	int a[MAX];
	
	int rank, size, processor_name_length;
	int sum;
	int* b= &sum;
	int n, ibeg, iend;
	char* processor_name = new char[MPI_MAX_PROCESSOR_NAME * sizeof(char)];

	for (int i = 0; i < MAX; i++) {
		a[i] = i+1;
	}

	auto start = std::chrono::system_clock::now();

	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	n = (int) MAX / size;
	ibeg = rank * n;
	iend = (rank + 1) * n;

	
	if (rank == 0){

		sum = 0;
		
		for (int i = ibeg; i < ((iend > MAX) ? MAX : iend); i++)
		{
			sum += a[i];
		}
		printf("Process: %d, %d\n", rank, sum);
		MPI_Send(b,1, MPI_INT, rank+1, 1, MPI_COMM_WORLD);
	}
	else if (rank!=size-1) { 
		MPI_Recv(b, 1, MPI_INT, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status); 
		for (int i = ibeg; i < ((iend > MAX) ? MAX : iend); i++)
		{
			sum += a[i];
		}
		printf("Process: %d, %d\n", rank, sum);
		MPI_Send(b, 1, MPI_INT, rank+1, 1, MPI_COMM_WORLD);
	}
	else {
		MPI_Recv(b, 1, MPI_INT, rank - 1, 1, MPI_COMM_WORLD, &status);
		for (int i = ibeg; i < MAX; i++)
		{
			sum += a[i];
		}
		printf("Process: %d, %d\n", rank, sum);
	}
	
	MPI_Finalize();
	auto end = std::chrono::system_clock::now();

	std::chrono::duration<double> elapsed_seconds = end - start;
	std::cout << "Elapsed Time: " << elapsed_seconds.count() << " sec" << std::endl;
	
	
	return 0;
}
