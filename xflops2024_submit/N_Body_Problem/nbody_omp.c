#include "nbody.h"
// #include <arm_neon.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <stdint.h> // uint32_t

void print_particle_0(Particle particles[], double fx[], double fy[],
                      double fz[]);

float Q_rsqrt(float number) {
    union {
        float f;
        uint32_t i;
    } conv = {.f = number};
    conv.i = 0x5f3759df - (conv.i >> 1);
    conv.f *= 1.5F - (number * 0.5F * conv.f * conv.f);
    return conv.f;
}

const size_t VECTOR_SIZE = 2;

// first attempt: vectorize
// second: loop unrolling

/* void compute_forces(Particle particles[], double fx[], double fy[], double
fz[], int N) { for (int i = 0; i < N / VECTOR_SIZE; i++) {
        // Each force is independent of each other

        float64x2_t vec_fx_1, vec_fy_1, vec_fz_1; // fx[i] = fy[i] = fz[i] = 0.0
        vec_fx_1 = vdupq_n_f64(0.0);
        vec_fy_1 = vdupq_n_f64(0.0);
        vec_fz_1 = vdupq_n_f64(0.0);

        float64x2_t vec_outer_x_1, vec_outer_x_rev_1;
        vec_outer_x_1 = vmovq_n_f64(particles[i].x);
        vec_outer_x_rev_1 = vmovq_n_f64(particles[i + 1].x);
        vec_outer_x_1 = vsetq_lane_f64(particles[i + 1].x, vec_outer_x_head_1,
1); vec_outer_x_rev_1 = vsetq_lane_f64(particles[i].x, vec_outer_x_head_rev_1,
1);

        float64x2_t vec_outer_y_1, vec_outer_y_rev_1;
        vec_outer_y_1 = vmovq_n_f64(particles[i].y);
        vec_outer_y_rev_1 = vmovq_n_f64(particles[i + 1].y);
        vec_outer_y_1 = vsetq_lane_f64(particles[i + 1].y, vec_outer_y_head_1,
1); vec_outer_y_rev_1 = vsetq_lane_f64(particles[i].y, vec_outer_y_head_rev_1,
1);

        float64x2_t vec_outer_z_1, vec_outer_z_rev_1;
        vec_outer_z_1 = vmovq_n_f64(particles[i].z);
        vec_outer_z_rev_1 = vmovq_n_f64(particles[i + 1].z);
        vec_outer_z_1 = vsetq_lane_f64(particles[i + 1].z, vec_outer_z_head_1,
1); vec_outer_z_rev_1 = vsetq_lane_f64(particles[i].z, vec_outer_z_head_rev_1,
1);

        for (int j = 0; j < N / VECTOR_SIZE; j++) {
            // a vector inner should be calculated together
            if (i != j) {
                float64x2_t vec_inner_x_1;
                vec_inner_x_1 = vmovq_n_f64(particles[j]);
                vec_inner_x_1 = vsetq_lane_f64(particles[j + 1], vec_inner_x_1,
1); float64x2_t vec_inner_y_1; vec_inner_y_1 = vmovq_n_f64(particles[j]);
                vec_inner_y_1 = vsetq_lane_f64(particles[j + 1], vec_inner_y_1,
1); float64x2_t vec_inner_z_1; vec_inner_z_1 = vmovq_n_f64(particles[j]);
                vec_inner_z_1 = vsetq_lane_f64(particles[j + 1], vec_inner_z_1,
1);

                float64x2_t vec_dx_1, vec_dy_1, vec_dz_1;
                vec_dx_1 = vsubq_f64(vec_inner_x_1, vec_outer_x_1); // double dx
= particles[j].x - particles[i].x; vec_dx_1 = vsubq_f64(vec_inner_x_1,
vec_outer_x_1); // double dx = particles[j].x - particles[i].x; double dx =
particles[j].x - particles[i].x; double dy = particles[j].y - particles[i].y;
                double dz = particles[j].z - particles[i].z;
                double dist = sqrt(dx * dx + dy * dy + dz * dz);
                if (dist > 1e-5) { // 避免除以零的错误
                    double F = G * particles[i].mass * particles[j].mass /
                               (dist * dist);
                    fx[i] += F * dx / dist;
                    fy[i] += F * dy / dist;
                    fz[i] += F * dz / dist;
                }
            }
        }
    }
} */

void compute_force(Particle from, Particle to, double *fx_to, double *fy_to,
                   double *fz_to, int i) {
    double dx = from.x - to.x;
    double dy = from.y - to.y;
    double dz = from.z - to.z;
    double square_dist = dx * dx + dy * dy + dz * dz;
    // self compute will end here
    if (square_dist > 1e-10) { // 避免除以零的错误
        double numeral = G * to.mass * from.mass;
        // double F = G * particles[i].mass * particles[j].mass / (dx * dx + dy
        // * dy + dz * dz);
        double other = pow((double)square_dist, (double)-1.5);
        *fx_to += dx * numeral * other;
        *fy_to += dy * numeral * other;
        *fz_to += dz * numeral * other;
        // fz[i] += F * dz / dist;
    }
}

// 计算粒子之间的引力
void compute_forces(Particle particles[], double fx[], double fy[], double fz[],
                    int N) {
    size_t outer_iteration = N / 4 * 4;
    omp_set_num_threads(128);
#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < outer_iteration; i += 4) {
        fx[i] = fy[i] = fz[i] = 0.0;
        fx[i + 1] = fy[i + 1] = fz[i + 1] = 0.0;
        fx[i + 2] = fy[i + 2] = fz[i + 2] = 0.0;
        fx[i + 3] = fy[i + 3] = fz[i + 3] = 0.0;
#pragma omp simd
        for (int j = 0; j < N; j++) {
            compute_force(particles[j], particles[i], (fx + i), (fy + i),
                          (fz + i), i);
            compute_force(particles[j], particles[i + 1], (fx + i + 1),
                          (fy + i + 1), (fz + i + 1), i + 1);
            compute_force(particles[j], particles[i + 2], (fx + i + 2),
                          (fy + i + 2), (fz + i + 2), i + 2);
            compute_force(particles[j], particles[i + 3], (fx + i + 3),
                          (fy + i + 3), (fz + i + 3), i + 3);
        }
    }
    size_t remain = N % 4;
#pragma omp parallel for schedule(dynamic)
    for (int i = N - remain; i < N; i++) {
        fx[i] = fy[i] = fz[i] = 0.0;
#pragma omp simd
        for (int j = 0; j < N; j++) {
            compute_force(particles[j], particles[i], (fx + i), (fy + i),
                          (fz + i), i);
        }
    }
}

void print_particle_0(Particle particles[], double fx[], double fy[],
                      double fz[]) {
    printf("Particle 0: x - %f, y - %f, z - %f, vx - %f, vy - %f, vz - %f\n",
           particles[0].x, particles[0].y, particles[0].z, particles[0].vx,
           particles[0].vy, particles[0].vz);
    printf("fx - %f, fy - %f, fz - %f\n", fx[0], fy[0], fz[0]);
}

// 更新粒子的位置和速度
void update_particles(Particle particles[], double fx[], double fy[],
                      double fz[], int N) {
    omp_set_num_threads(128);
#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < N; i++) {
        particles[i].vx += fx[i] / particles[i].mass * DT;
        particles[i].vy += fy[i] / particles[i].mass * DT;
        particles[i].vz += fz[i] / particles[i].mass * DT;
        particles[i].x += particles[i].vx * DT;
        particles[i].y += particles[i].vy * DT;
        particles[i].z += particles[i].vz * DT;
    }
    // printf("In update_particles\n");
    // print_particle_0(particles, fx, fy, fz);
}

// 计算体系总动量的三个分量
void compute_total_momentum(Particle particles[], double *px, double *py,
                            double *pz, int N) {
    *px = *py = *pz = 0.0;
    double x = 0.0, y = 0.0, z = 0.0;
    omp_set_num_threads(128);
#pragma omp parallel for reduction(+ : x, y, z) schedule(dynamic)
    for (int i = 0; i < N; i++) {
        x += particles[i].mass * particles[i].vx;
        y += particles[i].mass * particles[i].vy;
        z += particles[i].mass * particles[i].vz;
    }
    *px = x;
    *py = y;
    *pz = z;
}

// 计算系统的总能量（动能和势能）
double compute_total_energy(Particle particles[], int N) {
    double total_kinetic = 0.0;
    double total_potential = 0.0;

    // 计算动能
    omp_set_num_threads(128);
#pragma omp parallel for reduction(+ : total_kinetic) schedule(dynamic)
    for (int i = 0; i < N; i++) {
        double v2 = particles[i].vx * particles[i].vx +
                    particles[i].vy * particles[i].vy +
                    particles[i].vz * particles[i].vz;
        total_kinetic += 0.5 * particles[i].mass * v2;
    }
    // 计算势能
#pragma omp parallel for reduction(- : total_potential) schedule(dynamic)
    for (int i = 0; i < N; i++) {
#pragma omp simd
        for (int j = i + 1; j < N; j++) {
            double dx = particles[j].x - particles[i].x;
            double dy = particles[j].y - particles[i].y;
            double dz = particles[j].z - particles[i].z;
            double square_dist = dx * dx + dy * dy + dz * dz;
            if (square_dist > 1e-10) {
                total_potential -= G * particles[i].mass * particles[j].mass *
                                   Q_rsqrt(square_dist);
            }
        }
    }

    return total_kinetic + total_potential;
}
