#ifndef NBODY_H
#define NBODY_H

// 宏定义
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

// 函数声明
void update_particles(Particle particles[], double fx[], double fy[], double fz[], int N);
void compute_forces(Particle particles[], double fx[], double fy[], double fz[], int N);
void compute_total_momentum(Particle particles[], double *px, double *py, double *pz, int N);
double compute_total_energy(Particle particles[], int N);
void read_ref(double* array, char* file_path);

#endif 
