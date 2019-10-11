#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_opadd.h>
#include <chrono>
#include <iostream>

#define GETTIME std::chrono::high_resolution_clock::now()
constexpr size_t n = 5000000;

double f(double x) {
	return 4.0 / (pow(1.0 + x * x, 2));
}

int main() {
	double a = -1.0;
	double b = 1.0;
	double h = fabs(a - b) / n;
	std::chrono::high_resolution_clock::time_point t1, t2;
	std::chrono::duration<double> duration;

	__cilkrts_set_param("nworkers", "4");

	std::cout << "Integral calculator" << std::endl;
	std::cout << "a = " << a << std::endl;
	std::cout << "b = " << b << std::endl;
	std::cout << "h = " << h << std::endl;
	std::cout << "Number of divisions = " << n << std::endl;

	double* x = new double[n];
	for (int i = 0; i < n; i++)
		x[i] = a + i * h;

	t1 = GETTIME;

	cilk::reducer_opadd<double> result;
	cilk_for(long i = 1; i < n; i++)
		result += h * f((x[i - 1] + x[i]) / 2);

	t2 = GETTIME;

	duration = t2 - t1;
	std::cout << "Duration = " << duration.count() << " sec" << std::endl;
	std::cout << "Rusult = " << result.get_value();

	delete[] x;
	return 0;
}