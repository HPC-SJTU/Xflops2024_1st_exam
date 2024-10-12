#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include "nbody.h"

int main(int argc, char *argv[]) {
    int N;
    // 处理输入参数，选择问题规模
    if (argc != 2) {
        printf("Usage: %s <file path>\n", argv[0]);
        return -1;
    }

    double ref[35];
    read_ref(ref, argv[1]);
    srand(ref[33]); // 固定初始化随机数，用于正确性检验
    N = (int)ref[34];
    Particle particles[N];
    double fx[N], fy[N], fz[N];
    double initial_momentum_x=0, initial_momentum_y=0, initial_momentum_z=0;
    double current_momentum_x=0, current_momentum_y=0, current_momentum_z=0;
    double initial_energy=0, current_energy=0;
    double start_time=0, end_time=0, total_time = 0.0;
    double total_kinetic = 0.0;
    double total_potential = 0.0;

    // 初始化粒子
    for (int i = 0; i < N; i++) {
        particles[i].x = RAND_NUM * 10000;
        particles[i].y = RAND_NUM * 10000;
        particles[i].z = RAND_NUM *  10000;
        particles[i].vx = RAND_NUM * 1000;
        particles[i].vy = RAND_NUM * 1000;
        particles[i].vz = RAND_NUM * 1000;
        
        particles[i].mass = RAND_NUM * 10000;
    }
    for (int i = 0; i < N; i++) {
        initial_momentum_x += particles[i].mass * particles[i].vx;
        initial_momentum_y += particles[i].mass * particles[i].vy;
        initial_momentum_z += particles[i].mass * particles[i].vz;
    }

    for(int i = 0; i < N; i++){
        double v2 = particles[i].vx * particles[i].vx + particles[i].vy * particles[i].vy + particles[i].vz * particles[i].vz;
        total_kinetic += 0.5 * particles[i].mass * v2;

        for (int j = i + 1; j < N; j++) {
            double dx = particles[j].x - particles[i].x;
            double dy = particles[j].y - particles[i].y;
            double dz = particles[j].z - particles[i].z;
            double dist = sqrt(dx * dx + dy * dy + dz * dz);
            if (dist > 1e-5) {
                total_potential -= G * particles[i].mass * particles[j].mass / dist;
            }
        }
    }
    initial_energy = total_kinetic + total_potential;
    // 迭代仿真部分
    for (int step = 0; step < STEPS; step++) {
        //参与最终计时的部分：计算力，计算位置，计算动量，计算能量四个函数
        start_time = omp_get_wtime();

        compute_forces(particles, fx, fy, fz, N);
        update_particles(particles, fx, fy, fz, N);
        compute_total_momentum(particles, &current_momentum_x, &current_momentum_y, &current_momentum_z, N);
        current_energy = compute_total_energy(particles, N);

        end_time = omp_get_wtime();
        total_time += end_time - start_time;
        //每100步进行一次正确性检验
        if (step % 100 == 1) {
            int i = step / 100;
            printf("step %d finished in %f sec\n", i, total_time);
            printf("Initial Momentum: (%f, %f, %f)\n", initial_momentum_x, initial_momentum_y, initial_momentum_z);
            printf("Current Momentum: (%f, %f, %f)\n", current_momentum_x, current_momentum_y, current_momentum_z);
            printf("Initial Energy: %f\n", initial_energy);
            printf("Current Energy: %f\n", current_energy);
			if(fabs(initial_momentum_x - current_momentum_x) > ERR *3e7
					|| fabs(initial_momentum_y - current_momentum_y) > ERR *3e7
					|| fabs(initial_momentum_z - current_momentum_z) > ERR *3e7
					|| fabs(initial_energy - current_energy) > ERR*3e8
                    || fabs(particles[N / 2 + i].x - ref[i * 3]) > ERR
                    || fabs(particles[N / 2 + i].y - ref[i * 3 + 1]) > ERR
                    || fabs(particles[N / 2 + i].z - ref[i * 3 + 2]) > ERR){
				printf("Validation Fail, Check your Optimization!\n");
				return 1;
			}
			else
				printf("Validation Passed\n");
            printf("\n");
        }
    }
    printf("total simulation ends in %f sec\n", total_time);
    return 0;
}

// 读取正确性检验的参考文件的函数
void read_ref(double* array, char* file_path){
    FILE *fp;
    int i;
    // 打开文件
    fp = fopen(file_path, "r");
    if (fp == NULL) {
        printf("file not found: %s \n", file_path);
        exit(1);
    }

    for (i = 0; i < 35; i++) {
        if (fscanf(fp, "%lf", &array[i]) != 1) {
            printf("read data error, index of error data is %d\n", i);
            fclose(fp);
            exit(1);
        }
    }
    // 关闭文件
    fclose(fp);
}
