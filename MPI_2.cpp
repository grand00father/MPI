#include <stdio.h>
#include "mpi.h"
#include <chrono>
#include <iostream>

int main(int argc, char** argv)
{
	const int MAX_1 = 10;
	const int MAX_2 = 1000;
	const int MAX_3 = 1000000;
	int max_sizes[] = { MAX_1, MAX_2, MAX_3 };

	int rank, size, processor_name_length;
	int sum;
	int* b = &sum;
	int n, ibeg, iend;
	char* processor_name = new char[MPI_MAX_PROCESSOR_NAME * sizeof(char)];

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	MPI_Status status;

	double total_elapsed_time = 0.0;

	for (int max_size : max_sizes) {
		int* a = new int[max_size];

		for (int i = 0; i < max_size; i++) {
			a[i] = i + 1;
		}

		auto start = std::chrono::system_clock::now();

		n = (int)max_size / size;
		ibeg = rank * n;
		iend = (rank + 1) * n;

		if (rank == 0) {
			sum = 0;

			for (int i = ibeg; i < ((iend > max_size) ? max_size : iend); i++) {
				sum += a[i];
			}
			MPI_Send(b, 1, MPI_INT, rank + 1, 1, MPI_COMM_WORLD);
		}
		else if (rank != size - 1) {
			MPI_Recv(b, 1, MPI_INT, rank - 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			sum = 0;
			for (int i = ibeg; i < ((iend > max_size) ? max_size : iend); i++) {
				sum += a[i];
			}
			MPI_Send(b, 1, MPI_INT, rank + 1, 1, MPI_COMM_WORLD);
		}
		else {
			MPI_Recv(b, 1, MPI_INT, rank - 1, 1, MPI_COMM_WORLD, &status);
			sum = 0;
			for (int i = ibeg; i < max_size; i++) {
				sum += a[i];
			}
		}

		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = end - start;
		total_elapsed_time += elapsed_seconds.count();

		if (rank == size - 1) {
			std::cout << "Total Elapsed Time for "<< max_size << " elements: " << elapsed_seconds.count() << " sec" << std::endl;
		}

		delete[] a;
	}

	if (rank == size - 1) {
		std::cout << "Total Elapsed Time for all sizes: " << total_elapsed_time << " sec" << std::endl;
	}

	MPI_Finalize();

	delete[] processor_name;
	return 0;
}
