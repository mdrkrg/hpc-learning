#include <algorithm>
#include <cstdio>
// #include <omp.h>
#include <vector>
#pragma once

struct Server {
    long long weight, capacity, risk;
    Server(long long w, long long c) : weight(w), capacity(c), risk(0) {}
    Server() {}
};

bool compare_server(const Server &s1, const Server &s2) {
    return s1.weight + s1.capacity < s2.weight + s2.capacity;
}

void merge_sort_recursive(std::vector<Server> &servers, unsigned long long left,
                          unsigned long long right) {
    if (left < right) {
        if (right - left >= 32) {
            unsigned long long mid = (right - left) / 2 + left;
#pragma omp taskgroup
            {
#pragma omp task shared(servers) untied if (right - left >= (1 << 14))
                merge_sort_recursive(servers, left, mid);
#pragma omp task shared(servers) untied if (right - left >= (1 << 14))
                merge_sort_recursive(servers, mid + 1, right);
#pragma omp taskyield
            }
            std::inplace_merge(servers.begin() + left,
                               servers.begin() + mid + 1,
                               servers.begin() + right + 1, compare_server);
        } else {
            std::sort(servers.begin() + left, servers.begin() + right + 1,
                      compare_server);
        }
    }
}

void merge_sort(std::vector<Server> &servers) {
#pragma omp parallel
#pragma omp single
    merge_sort_recursive(servers, 0, servers.size() - 1);
}

long long solve(Server *servers, long long n) {
    static const size_t NUM_THREADS = 24;
    // omp_set_num_threads(NUM_THREADS);
    std::vector<Server> servers_v(servers, servers + n);
    merge_sort(servers_v);

    long long total_weight = servers_v[0].weight;
    servers_v[0].risk = -servers_v[0].capacity;
    long long max_risk = servers_v[0].risk;

    for (long long i = 1; i < n; ++i) {
        servers_v[i].risk = total_weight - servers_v[i].capacity;
        max_risk = std::max(max_risk, servers_v[i].risk);
        total_weight += servers_v[i].weight;
    }

    return max_risk;
}
