#include "data_loader.h"

using namespace std;

vector<int> DataLoader::generate_data(size_t size) 
{
    vector<int> data(size);

    random_device rd;
    mt19937 gen(rd());

    uniform_int_distribution<> distrib(1, 1000000);

    for (size_t i = 0; i < size; ++i) 
    {
        data[i] = distrib(gen);
    }
    return data;
}