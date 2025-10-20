#pragma once

#include <vector>
#include <cstddef>
#include <random>

class DataLoader 
{
public:
    static std::vector<int> generate_data(size_t size);
};