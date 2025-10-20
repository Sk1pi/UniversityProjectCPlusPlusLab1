#pragma once

#include <vector>
#include <string>
#include <map>
#include <chrono>
#include <cstddef>

using Duration = std::chrono::microseconds;

struct MeasurementResult 
{
    std::string name;
    long long time_us;
    size_t data_size;
};

struct CustomParallelReport 
{
    std::map<int, long long> times_by_K; 
    int best_K = 0;
    long long best_time_us = -1;
    int hardware_concurrency = 0;
};

class MinExperiments 
{
private:
    const std::vector<int>& data_;
    size_t data_size_;
    std::vector<MeasurementResult> library_results_;
    CustomParallelReport custom_report_;

    template<typename Func>
    long long measure_time(Func func) const 
    {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<Duration>(end - start).count();
    }

    int custom_parallel_min_element(int K) const;

public:
    MinExperiments(const std::vector<int>& data);

    void run_library_tests();

    void run_custom_parallel_tests();

    void print_report() const;
};