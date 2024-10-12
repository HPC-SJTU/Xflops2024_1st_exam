# Makefile for N-body simulation
CC = clang  # 使用的编译器
MPICC = mpicc
CFLAGS = -O2 -Wall -lm -fopenmp

# 可执行文件名称
TARGET = nbody_simulator

# 源代码文件
SRC_serial = nbody_serial.c main.c
SRC_omp = nbody_omp.c main.c
SRC_mpi = nbody_simulator_mpi.c

# 默认目标，编译串行版本
all: serial

# 编译串行版本
serial: $(SRC_serial)
	$(CC) $(CFLAGS) -o $(TARGET)_serial $(SRC_serial)

# 编译 OpenMP 版本
omp: $(SRC_omp)
	$(CC) $(CFLAGS) -o $(TARGET)_omp $(SRC_omp)

# 编译 MPI 版本
mpi: $(SRC_mpi)
	$(MPICC) $(CFLAGS) -o $(TARGET)_mpi $(SRC_mpi)

# 清理编译生成的文件
clean:
	rm -f $(TARGET)_serial $(TARGET)_omp $(TARGET)_mpi

