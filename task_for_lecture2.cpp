#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_max.h>
#include <cilk/reducer_min.h>
#include <cilk/reducer_vector.h>
#include <chrono>
#include <iostream>

void ReducerMaxTest(int* mass_pointer, const long size) {
	cilk::reducer<cilk::op_max_index<long, int>> maximum;
	cilk_for(long i = 0; i < size; ++i)
		maximum->calc_max(i, mass_pointer[i]);

	std::cout << "Maximal element = " << maximum->get_reference()
		<< " has index = " << maximum->get_index_reference() << std::endl;
}

void ReducerMinTest(int* mass_pointer, const long size) {
	cilk::reducer<cilk::op_min_index<long, int>> minimum;
	cilk_for(long i = 0; i < size; ++i)
		minimum->calc_min(i, mass_pointer[i]);

	std::cout << "Minimum element = " << minimum->get_reference()
		<< " has index = " << minimum->get_index_reference() << std::endl;
}

void ParallelSort(int* begin, int* end) {
	if (begin != end) {
		--end;
		int* middle = std::partition(begin, end, std::bind2nd(std::less<int>(), *end));
		std::swap(*end, *middle);
		cilk_spawn ParallelSort(begin, middle);
		ParallelSort(++middle, ++end);
		cilk_sync;
	}
}

void CompareForAndCilk_For(size_t sz) {
	std::chrono::duration<double> duration;
	std::chrono::high_resolution_clock::time_point t1, t2;
	
	t1 = std::chrono::high_resolution_clock::now();

	std::vector<int> vec;
	for (long i; i < sz; i++)
		vec.push_back(rand() % 20000 + 1);

	t2 = std::chrono::high_resolution_clock::now();

	duration = (t2 - t1);
	std::cout << "'For' duration is: " << duration.count() 
		<< " seconds with sz = " << sz << std::endl;

	t1 = std::chrono::high_resolution_clock::now();

	cilk::reducer<cilk::op_vector<int>>red_vec;
	cilk_for(long i = 0; i < sz; ++i)
		red_vec->push_back(rand() % 20000 + 1);

	t2 = std::chrono::high_resolution_clock::now();

	duration = (t2 - t1);
	std::cout << "'Cilk for' duration is: " << duration.count() 
		<< " seconds with sz = " << sz << std::endl;
}


int main()
{
	srand((unsigned)time(0));

	__cilkrts_set_param("nworkers", "4");

	long i;
	const long mass_size = 1000000;
	int* mass_begin, * mass_end;
	int* mass = new int[mass_size];
	std::chrono::high_resolution_clock::time_point t1, t2;

	for (i = 0; i < mass_size; ++i)
		mass[i] = (rand() % 25000) + 1;

	std::cout << "mass_size = " << mass_size << std::endl;

	mass_begin = mass;
	mass_end = mass_begin + mass_size;
	ReducerMaxTest(mass, mass_size);
	ReducerMinTest(mass, mass_size);

	t1 = std::chrono::high_resolution_clock::now();
	ParallelSort(mass_begin, mass_end);
	t2 = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double> duration = (t2 - t1);
	std::cout << "Duration of sort is: " << duration.count() << " seconds" << std::endl;

	ReducerMaxTest(mass, mass_size);
	ReducerMinTest(mass, mass_size);
	
	std::vector<int> sz = { 1000000, 100000, 10000, 1000, 500, 100, 50, 1 };
	for (size_t i = 0; i < sz.size(); ++i)
		CompareForAndCilk_For(sz[i]);

	delete[]mass;
	return 0;
}