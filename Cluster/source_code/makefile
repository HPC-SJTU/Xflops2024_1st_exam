CC=clang++
CFLAGS=-fopenmp -O3

all: cluster

cluster: main.cpp solver.hpp
	$(CC) $(CFLAGS) -o cluster main.cpp