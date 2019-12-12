#include <iostream>
#include <omp.h>
#include <math.h>

namespace consts {
    constexpr uint64_t  num             = 100000000;
    constexpr size_t    num_of_threads  = 3;
    constexpr double    step            = 1.0 / static_cast<double>(num);
};

double par(void) {
    omp_lock_t          writeLock;
    uint64_t            inc             = 0;
    double              S               = 0.0;
    double              t               = omp_get_wtime();

    omp_init_lock(&writeLock);
#pragma omp parallel for reduction(+:S) private (x) shared(consts::step)
    {
        for (uint64_t i = 0; i < consts::num; i++) {
            double x = (i + 0.5) * consts::step;
            S += 4.0 / (1.0 + pow(x, 2)); 
 #pragma omp critical
            {
                omp_set_lock(&writeLock);
                inc++;
                omp_unset_lock(&writeLock);
            }
        }
    }
    omp_destroy_lock(&writeLock);

    t = omp_get_wtime() - t;
    std::cout << "par: pi = " << consts::step * S << std::endl;
    std::cout << "inc = " << inc << std::endl;
    return t;
}

int main() {
#pragma omp parallel
    {
 #pragma omp master
        {
            omp_set_num_threads(consts::num_of_threads);
            std::cout << "OpenMP. number of threads: " << consts::num_of_threads << std::endl;
        }
    }
    std::cout << "elapsed time: " << par() << " sec." << std::endl;
    return 0;
}