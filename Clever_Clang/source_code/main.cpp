#include <iostream>
#include <chrono>
#include <fstream>
#include <string>
#include <iomanip> 

#include "gd.h"


int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file>" << std::endl;
        return 1;
    }

    std::string input_filepath = argv[1];
    std::string output_filepath = argv[2];

    std::ifstream infile(input_filepath);
    if (!infile) {
        std::cerr << "Error: Unable to open input file " << input_filepath << std::endl;
        return 1;
    }

    float a, b, eta;
    uint32_t N, M;
    PolyParams params(0, 0, 0, 0);

    infile >> a >> b >> N >> M >> eta;
    infile >> params.a >> params.b >> params.c >> params.d;
    infile.close();

    // initialize points
    float *points = new float[N];
    float interval = (b - a) / (N - 1);
    for (uint32_t i = 0; i < N; ++i) {
        points[i] = a + i * interval;
    }

    std::cout << "Search params: " << a << " " << b << " " << N << " " << M << " " << eta << std::endl;
    std::cout << "Function params: " << params.a << " " << params.b << " " << params.c << " " << params.d << std::endl;

    auto t1 = std::chrono::steady_clock::now();
    gradient_descent(points, N, M, eta, &params);
    auto t2 = std::chrono::steady_clock::now();
    uint32_t d1 = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    std::cout << "Time: " << d1 << "ms" << std::endl;


    // write to output file
    std::ofstream outfile(output_filepath);
    if (!outfile) {
        std::cerr << "Error: Unable to open output file " << output_filepath << std::endl;
        delete[] points;
        return 1;
    }
    outfile << std::setprecision(std::numeric_limits<float>::max_digits10);
    for (uint32_t i = 0; i < N; ++i) {
        outfile << points[i] << std::endl;
    }
    outfile.close();


    delete[] points;

    return 0;
}
