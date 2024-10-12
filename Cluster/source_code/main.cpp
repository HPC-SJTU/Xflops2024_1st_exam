#include "solver.hpp"
#include <iostream>
#include <vector>
#include <fstream>
#include <sys/time.h>
#include <random>
#include "omp.h"

const long long MOD = 998244353;
const int thread_num = 128;

double timers[10];
double total_t[10];
#define TIC(i) timers[(i)] = timer()
#define TOC(i) total_t[(i)] += timer() - timers[(i)]
inline double timer() {
  struct timeval tv;
  gettimeofday(&tv, 0);
  return tv.tv_sec + tv.tv_usec * 1.e-6;
}


using namespace std;
int main (int argc, char* argv[]) {
    long long n;
    if (argc!= 2){
        std::cout<<"Usage: "<< argv[0] << " <file_name>"<<endl;
        return 1;
    }
    
    ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cout<< "Unable to open file "<< argv[1] << endl;
        return 1;
    }
    file >> n;
    int random_seed[thread_num];
    for(int i=0; i < thread_num; i++){
        file >> random_seed[i];
    }
    Server* servers = new Server[n];
    int nthreads, tid;

    #pragma omp parallel private(tid)
    { 
        std::mt19937 gen;
        std::uniform_int_distribution<> dis(1, 100000);
        nthreads = omp_get_num_threads();
        tid = omp_get_thread_num();
        gen = std::mt19937(random_seed[tid]);
        for(long long i = n*tid / nthreads; i< n*(tid+1)/ nthreads; ++i){
            servers[i] = Server(dis(gen), dis(gen));
        }
    }

    TIC(0);
    long long ans = solve(servers, n);
    TOC(0);

    cout << "solve Time " << total_t[0] << endl;

    cout <<  ans << endl;
    
    delete [] servers;
    
    return 0;
}