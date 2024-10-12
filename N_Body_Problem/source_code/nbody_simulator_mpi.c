#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define G 6.67430e-11 // 引力常数
#define DT 1       // 时间步长
#define STEPS 1002    // 时间步数
#define RAND_NUM ((double)rand() / RAND_MAX)
#define ERR 5e-7

typedef struct {
    double x, y, z;
    double vx, vy, vz;
    double mass;
} Particle;
void read_ref(double* array, int N);

void compute_forces(Particle local_particles[], Particle global_particles[], double fx[], double fy[], double fz[], int local_n, int global_n) {
    for (int i = 0; i < local_n; i++) {
        fx[i] = fy[i] = fz[i] = 0.0;
        for (int j = 0; j < global_n; j++) {
            double dx = global_particles[j].x - local_particles[i].x;
            double dy = global_particles[j].y - local_particles[i].y;
            double dz = global_particles[j].z - local_particles[i].z;
            double dist = sqrt(dx * dx + dy * dy + dz * dz);
            if (dist > 1e-5) {
                double F = G * local_particles[i].mass * global_particles[j].mass / (dist * dist);
                fx[i] += F * dx / dist;
                fy[i] += F * dy / dist;
                fz[i] += F * dz / dist;
            }
        }
    }
}

void update_particles(Particle particles[], double fx[], double fy[], double fz[], int n_particles) {
    for (int i = 0; i < n_particles; i++) {
        particles[i].vx += fx[i] / particles[i].mass * DT;
        particles[i].vy += fy[i] / particles[i].mass * DT;
        particles[i].vz += fz[i] / particles[i].mass * DT;
        particles[i].x += particles[i].vx * DT;
        particles[i].y += particles[i].vy * DT;
        particles[i].z += particles[i].vz * DT;
    }
}

void compute_total_momentum(Particle particles[], double *px, double *py, double *pz, int n_particles) {
    *px = *py = *pz = 0.0;
    for (int i = 0; i < n_particles; i++) {
        *px += particles[i].mass * particles[i].vx;
        *py += particles[i].mass * particles[i].vy;
        *pz += particles[i].mass * particles[i].vz;
    }
}

double compute_total_energy(Particle local_particles[], int local_n, Particle global_particles[], int global_n) {
    double total_kinetic = 0.0;
    double total_potential = 0.0;

    // 计算动能
    for (int i = 0; i < local_n; i++) {
        double v2 = local_particles[i].vx * local_particles[i].vx + local_particles[i].vy * local_particles[i].vy + local_particles[i].vz * local_particles[i].vz;
        total_kinetic += 0.5 * local_particles[i].mass * v2;
    }

    // 计算势能
    for (int i = 0; i < local_n; i++) {
        for (int j = 0; j < global_n; j++) {
            double dx = global_particles[j].x - local_particles[i].x;
            double dy = global_particles[j].y - local_particles[i].y;
            double dz = global_particles[j].z - local_particles[i].z;
            double dist = sqrt(dx * dx + dy * dy + dz * dz);
            if (dist > 1e-5) { // 避免除零错误
                total_potential -= G * local_particles[i].mass * global_particles[j].mass / dist;
            }
        }
    }

    return total_kinetic + total_potential;
}

void read_ref(double* array, int N){
    FILE *fp;
    int i;
    // 打开文件
    char filename[128];  
    sprintf(filename, "ref_data/%d_1.ref", N);

    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("无法打开文件！\n");
        return;
    }

    for (i = 0; i < 33; i++) {
        if (fscanf(fp, "%lf", &array[i]) != 1) {
            printf("读取数据时出错，位置：%d\n", i);
            fclose(fp);
            return;
        }
    }

    // 关闭文件
    fclose(fp);
}

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    const int N = 4096;
    double ref[33];
    if(rank == 0)
        read_ref(ref, N);
    int pass=0;
    int step = 0;
    const int local_n = N / size;
    Particle particles[N];
	Particle local_particles[local_n];
    double fx[local_n], fy[local_n], fz[local_n];
    double local_momentum_x, local_momentum_y, local_momentum_z;
    double local_energy, global_momentum_x, global_momentum_y, global_momentum_z, global_energy;
    double initial_momentum_x=0, initial_momentum_y=0, initial_momentum_z=0, initial_energy=0.0;
    double total_kinetic = 0.0;
    double total_potential = 0.0;

    // 初始化粒子数据
    if (rank == 0) {
		srand(1);
        for (int i = 0; i < N; i++) {
            particles[i].x = RAND_NUM * 10000;
            particles[i].y = RAND_NUM * 10000;
            particles[i].z = RAND_NUM *  10000;
            particles[i].vx = RAND_NUM * 1000;
            particles[i].vy = RAND_NUM * 1000;
            particles[i].vz = RAND_NUM * 1000;
    
            particles[i].mass = RAND_NUM * 10000;

		}

        // 计算初始的总动量和总能量
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
    }

    // TODO:广播粒子数据到所有进程

    memcpy(local_particles, particles + rank * local_n, local_n * sizeof(Particle));
    // 模拟
    for (step = 0; step < STEPS; step++) {
		compute_forces(local_particles, particles, fx, fy, fz, local_n, N);
		update_particles(local_particles, fx, fy, fz, local_n);
        compute_total_momentum(local_particles, &local_momentum_x, &local_momentum_y, &local_momentum_z, local_n);
        local_energy = compute_total_energy(local_particles, local_n, particles, N);

        // TODO: 同步所有进程模拟的信息

	   	if(step % 100 == 1){
			// TODO: 归约所有进程计算的局部动量分量和能量（4次归约操作，到0号进程）




        	if (rank == 0) {
                int i = step / 100;
                printf("Initial Momentum: (%f, %f, %f)\n", initial_momentum_x, initial_momentum_y, initial_momentum_z);
                printf("Current Momentum: (%f, %f, %f)\n", global_momentum_x, global_momentum_y, global_momentum_z);
                printf("Initial Energy: %f\n", initial_energy);
                printf("Current Energy: %f\n", global_energy);
                if(fabs(initial_momentum_x - global_momentum_x) > ERR *3e7
                        || fabs(initial_momentum_y - global_momentum_y) > ERR *3e7
                        || fabs(initial_momentum_z - global_momentum_z) > ERR *3e7
                        || fabs(initial_energy - global_energy) > ERR*3e8
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
        
    }
    if(step == STEPS) pass=1;
    if(rank == 0)
        if(pass)
        printf("CONGRATULATIONS, YOU DID IT!\n");
    MPI_Finalize();
    return 0;
}

