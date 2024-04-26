#include <iostream>
#include <vector>
#include <mpi.h>

using namespace std;

void solve_gaussian_elimination(vector < vector < double>>& A, vector<double>& b, MPI_Comm comm) {
	int rank, size;
	MPI_Comm_rank(comm, &rank);
	MPI_Comm_size(comm, &size);
	int n = b.size();

	// Forward elimination
	for (int i = 0; i < n; ++i) {
		if (i % size == rank) {
			vector<double> pivot_row = A[i];
			double pivot_val = b[i] / A[i][i];
			MPI_Bcast(pivot_row.data(), n, MPI_DOUBLE, rank, comm);
			MPI_Bcast(&pivot_val, 1, MPI_DOUBLE, rank, comm);
			for (int j = i + 1; j < n; ++j) {
				if (j % size == rank) {
					double ratio = A[j][i] / A[i][i];
					for (int k = 0; k < n; ++k) {
						A[j][k] -= ratio * pivot_row[k];
					}
					b[j] -= ratio * pivot_val;
				}
			}
		}
		else {
			vector<double> pivot_row(n);
			double pivot_val;
			MPI_Bcast(pivot_row.data(), n, MPI_DOUBLE, i % size, comm);
			MPI_Bcast(&pivot_val, 1, MPI_DOUBLE, i % size, comm);
			for (int j = i + 1; j < n; ++j) {
				if (j % size == rank) {
					double ratio = A[j][i] / A[i][i];
					for (int k = 0; k < n; ++k) {
						A[j][k] -= ratio * pivot_row[k];
					}
					b[j] -= ratio * pivot_val;
				}
			}
		}
	}

	// Backward substitution
	vector<double> x(n);
	for (int i = n - 1; i >= 0; --i) {
		if (i % size == rank) {
			x[i] = b[i] / A[i][i];
			for (int j = 0; j < i; ++j) {
				if (j % size == rank) {
					b[j] -= A[j][i] * x[i];
				}
			}
		}
	}

	// Broadcast solution
	for (int i = 0; i < n; ++i) {
		MPI_Bcast(&x[i], 1, MPI_DOUBLE, i % size, comm);
	}
}

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);
	MPI_Comm comm = MPI_COMM_WORLD;
	int rank;
	MPI_Comm_rank(comm, &rank);

	vector<int> n_values = { 10, 100, 500, 1000};

	for (int n : n_values) {
		vector < vector < double>> A(n, vector<double>(n));
		vector<double> b(n);
		if (rank == 0) {
			// Create a random system of linear equations
			srand(time(NULL));
			for (int i = 0; i < n; ++i) {
				for (int j = 0; j < n; ++j) {
					A[i][j] = rand() % 100; // Random values between 0 and 99
				}
				b[i] = rand() % 100; // Random values between 0 and 99
			}
		}

		// Broadcast matrix A and vector b
		for (int i = 0; i < n; ++i) {
			MPI_Bcast(A[i].data(), n, MPI_DOUBLE, 0, comm);
		}
		MPI_Bcast(b.data(), n, MPI_DOUBLE, 0, comm);

		double start_time = MPI_Wtime();
		solve_gaussian_elimination(A, b, comm);
		double end_time = MPI_Wtime();

		if (rank == 0) {
			cout << "System size: " << n << endl;
			cout << "Solving time: " << end_time - start_time << " seconds" << endl << endl;
		}
	}

	MPI_Finalize();
	return 0;
}
