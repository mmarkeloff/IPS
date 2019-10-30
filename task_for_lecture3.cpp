#include <stdio.h>
#include <ctime>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_opadd.h>
#include <chrono>
#include <iostream>


#define GETTIME std::chrono::high_resolution_clock::now()

constexpr int MATRIX_SIZE = 2000;

void InitMatrix( double** matrix )
{
	for ( int i = 0; i < MATRIX_SIZE; ++i )
		matrix[i] = new double[MATRIX_SIZE + 1];

	for ( int i = 0; i < MATRIX_SIZE; ++i ) {
		for ( int j = 0; j <= MATRIX_SIZE; ++j )
			matrix[i][j] = rand() % 2500 + 1;
	}
}

std::chrono::duration<double> SerialGaussMethod(double** matrix, const int rows, double* result)
{
    double koef;
    std::chrono::duration<double> duration;
    std::chrono::high_resolution_clock::time_point t1, t2;

    t1 = GETTIME;

    for (int k = 0; k < rows; ++k) {
        for (int i = k + 1; i < rows; ++i ) {
            koef = -matrix[i][k] / matrix[k][k];

            for (int j = k; j < rows; ++j)
                matrix[i][j] += koef * matrix[k][j];
        }
    }

    t2 = GETTIME;

    duration = t2 - t1;
    std::cout << "Serial duration = " << duration.count() << " sec" << std::endl;

    result[rows - 1] = matrix[rows - 1][rows] / matrix[rows - 1][rows - 1];

    for (int k = rows - 2; k >= 0; --k) {
        result[k] = matrix[k][rows];

        for (int j = k + 1; j < rows; ++j )
            result[k] -= matrix[k][j] * result[j];

        result[k] /= matrix[k][k];
    }

    return duration;
}

std::chrono::duration<double> ParallelGaussMethod( double **matrix, const int rows, double* result )
{
    std::chrono::duration<double> duration;
    std::chrono::high_resolution_clock::time_point t1, t2;

    t1 = GETTIME;

	for (int k = 0; k < rows; ++k ) {
        cilk::reducer_opadd<int> k_reducer(k);
        cilk_for (int i = k_reducer.get_value() + 1; i < rows; ++i) {
            double koef = -matrix[i][k] / matrix[k][k];

            for (int j = k; j < rows; ++j)
                matrix[i][j] += koef * matrix[k][j];
		}
	}

    t2 = GETTIME;

    duration = t2 - t1;
    std::cout << "Parallel duration = " << duration.count() << " sec" << std::endl;

	result[rows - 1] = matrix[rows - 1][rows] / matrix[rows - 1][rows - 1];

	for (int k = rows - 2; k >= 0; --k ) {
		result[k] = matrix[k][rows];

        cilk::reducer_opadd<int> k_reducer(k);
        cilk_for(int j = k_reducer.get_value() + 1; j < rows; ++j) {
            cilk::reducer_opadd<int> res_reducer(result[j]);
            result[k] -= matrix[k][j] * res_reducer.get_value();
        }

		result[k] /= matrix[k][k];
	}

    return duration;
}


int main() {
	srand( (unsigned) time( 0 ) );

    __cilkrts_set_param("nworkers", "4");

    double** matrix = new double* [MATRIX_SIZE];
    for (size_t i = 0; i < MATRIX_SIZE; ++i)
        matrix[i] = new double[MATRIX_SIZE + 1];

    InitMatrix(matrix);

    double* result = new double[MATRIX_SIZE];
    std::chrono::duration<double> dur = ParallelGaussMethod(matrix, MATRIX_SIZE, result );

	for (size_t i = 0; i < MATRIX_SIZE; ++i)
		delete[] matrix[i];

    delete[] matrix;
	delete[] result;

    matrix = new double* [MATRIX_SIZE];
    for (size_t i = 0; i < MATRIX_SIZE; ++i)
        matrix[i] = new double[MATRIX_SIZE + 1];

    InitMatrix(matrix);

    result = new double[MATRIX_SIZE];

    std::chrono::duration<double> _dur = SerialGaussMethod(matrix, MATRIX_SIZE, result);

    for (size_t i = 0; i < MATRIX_SIZE; ++i)
        delete[] matrix[i];

    delete[] matrix;
    delete[] result;

    std::cout << "Acceleration is: " << _dur.count() - dur.count() << std::endl;

	return 0;
}