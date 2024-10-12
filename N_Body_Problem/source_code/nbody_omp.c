#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "nbody.h"

// 计算粒子之间的引力
void compute_forces(Particle particles[], double fx[], double fy[], double fz[], int N) {
    for (int i = 0; i < N; i++) {
        fx[i] = fy[i] = fz[i] = 0.0;
        for (int j = 0; j < N; j++) {
            if (i != j) {
                double dx = particles[j].x - particles[i].x;
                double dy = particles[j].y - particles[i].y;
                double dz = particles[j].z - particles[i].z;
                double dist = sqrt(dx * dx + dy * dy + dz * dz);
                if (dist > 1e-5) { // 避免除以零的错误
                    double F = G * particles[i].mass * particles[j].mass / (dist * dist);
                    fx[i] += F * dx / dist;
                    fy[i] += F * dy / dist;
                    fz[i] += F * dz / dist;
                }
            }
        }
    }
}

// 更新粒子的位置和速度
void update_particles(Particle particles[], double fx[], double fy[], double fz[], int N) {
    for (int i = 0; i < N; i++) {
        particles[i].vx += fx[i] / particles[i].mass * DT;
        particles[i].vy += fy[i] / particles[i].mass * DT;
        particles[i].vz += fz[i] / particles[i].mass * DT;
        particles[i].x += particles[i].vx * DT;
        particles[i].y += particles[i].vy * DT;
        particles[i].z += particles[i].vz * DT;
    }
}

// 计算体系总动量的三个分量
void compute_total_momentum(Particle particles[], double *px, double *py, double *pz, int N) {
    *px = *py = *pz = 0.0;
	double x=0.0,y=0.0,z=0.0;
    for (int i = 0; i < N; i++) {
        x += particles[i].mass * particles[i].vx;
        y += particles[i].mass * particles[i].vy;
        z += particles[i].mass * particles[i].vz;
    }
	*px=x;
	*py=y;
	*pz=z;
}

// 计算系统的总能量（动能和势能）
double compute_total_energy(Particle particles[], int N) {
    double total_kinetic = 0.0;
    double total_potential = 0.0;

    // 计算动能
    for (int i = 0; i < N; i++) {
        double v2 = particles[i].vx * particles[i].vx + particles[i].vy * particles[i].vy + particles[i].vz * particles[i].vz;
        total_kinetic += 0.5 * particles[i].mass * v2;
    }
    // 计算势能
    for(int i = 0; i < N; i++){
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

    return total_kinetic + total_potential;
}
