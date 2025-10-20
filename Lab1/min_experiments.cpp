#include "min_experiments.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <thread>
#include <future>
#include <numeric>
#include <execution>

using namespace std;

MinExperiments::MinExperiments(const vector<int>& data)
    : data_(data), data_size_(data.size())
{
    custom_report_.hardware_concurrency = (int)thread::hardware_concurrency();
}


void MinExperiments::run_library_tests() 
{
    long long time_default = measure_time([&]() {(void)*min_element(data_.begin(), data_.end());});
    library_results_.push_back({ "Default/Sequential", time_default, data_size_ });

    long long time_seq = measure_time([&]() {(void)*min_element(execution::seq, data_.begin(), data_.end());});
    library_results_.push_back({ "std::execution::seq", time_seq, data_size_ });

    long long time_par = measure_time([&]() {(void)*min_element(execution::par, data_.begin(), data_.end());});
    library_results_.push_back({ "std::execution::par", time_par, data_size_ });

    long long time_par_unseq = measure_time([&]() {(void)*min_element(execution::par_unseq, data_.begin(), data_.end());});
    library_results_.push_back({ "std::execution::par_unseq", time_par_unseq, data_size_ });
}

int MinExperiments::custom_parallel_min_element(int K) const 
{
    size_t size = data_.size();
    if (size == 0) return 0;

    int num_threads = min((int)size, K);
    size_t chunk_size = size / num_threads;
    size_t remainder = size % num_threads;

    vector<future<int>> results;
    size_t current_start = 0;

    auto worker_task = [](const vector<int>::const_iterator begin, const vector<int>::const_iterator end) 
    {
        return *std::min_element(begin, end);
    };

    for (int i = 0; i < num_threads; ++i) {
        size_t current_length = chunk_size + (i < remainder ? 1 : 0);
        size_t current_end = current_start + current_length;

        results.push_back(async(launch::async, worker_task, data_.begin() + current_start, data_.begin() + current_end));

        current_start = current_end;
    }

    int overall_min = numeric_limits<int>::max();
    for (auto& res_future : results) 
    {
        overall_min = min(overall_min, res_future.get());
    }

    return overall_min;
}

void MinExperiments::run_custom_parallel_tests() 
{
    int hardware_concurrency = custom_report_.hardware_concurrency;
    int max_k = max(2, hardware_concurrency * 4);
    if (max_k > 64) max_k = 64;

    vector<int> K_values;
    K_values.push_back(1);
    int k_pow2 = 2;
    while (k_pow2 <= max_k) 
    {
        K_values.push_back(k_pow2);
        k_pow2 *= 2;
    }

    bool concurrency_present = any_of(K_values.begin(), K_values.end(),
        [hardware_concurrency](int k) { return k == hardware_concurrency; });
    if (hardware_concurrency > 1 && !concurrency_present) 
    {
        K_values.push_back(hardware_concurrency);
        sort(K_values.begin(), K_values.end());
        K_values.erase(unique(K_values.begin(), K_values.end()), K_values.end());
    }

    const int num_runs = 3;

    for (int K : K_values) 
    {
        long long total_time = 0;
        for (int i = 0; i < num_runs; ++i) 
        {
            total_time += measure_time([&]() {(void)custom_parallel_min_element(K);});
        }
        long long avg_time = total_time / num_runs;
        custom_report_.times_by_K[K] = avg_time;

        if (custom_report_.best_time_us == -1 || avg_time < custom_report_.best_time_us) 
        {
            custom_report_.best_time_us = avg_time;
            custom_report_.best_K = K;
        }
    }
}

void MinExperiments::print_report() const 
{
    cout << "\n=====================================================" << endl;
    cout << "EXPERIMENT REPORT (Data size:  " << data_size_ << ")" << endl;
    cout << "=====================================================" << endl;

    cout << "\n### 1 & 2. Бібліотечний std::min_element with/without policies ###" << endl;
    cout << "+------------------------+-----------------------+" << endl;
    cout << "| Algorithm             | Execution time (?s)    |" << endl;
    cout << "+------------------------+-----------------------+" << endl;
    for (const auto& res : library_results_) 
    {
        cout << "| " << setw(22) << left << res.name << " | " << setw(19) << right << res.time_us << " |" << endl;
    }
    cout << "+------------------------+-----------------------+" << endl;

    cout << "\n### 3. Own parallel algorithm (Dependence on K) ###" << endl;
    cout << "Number of processor threads (hardware_concurrency): " << custom_report_.hardware_concurrency << endl;
    cout << "+------+-----------------------+" << endl;
    cout << "|   K   | Average time (?s)    |" << endl;
    cout << "+------+-----------------------+" << endl;
    for (const auto& pair : custom_report_.times_by_K) 
    {
        cout << "| " << setw(4) << pair.first << " | " << setw(19) << pair.second << " |" << endl;
    }
    cout << "+------+-----------------------+" << endl;

    cout << "\n### Conclusions regarding our own algorithm ###" << endl;
    cout << "- **The best speed** is achieved when **K = " << custom_report_.best_K << "**, час: "
        << custom_report_.best_time_us << " мкс." << endl;

    double ratio = (double)custom_report_.best_K / custom_report_.hardware_concurrency;
    cout << "- Ratio of the best K to hardware_concurrency (" << custom_report_.hardware_concurrency << "): **"<< ratio << "**" << endl;

    cout << "\n**The law of increasing time with increasing K:**" << endl;
}