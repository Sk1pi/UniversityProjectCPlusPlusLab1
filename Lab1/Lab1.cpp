#include <iostream>
#include <vector>
#include <iomanip>
#include "data_loader.h"
#include "min_experiments.h"

using namespace std;

// Compiler: GCC 13.2.0 

int main() 
{
    cout << fixed << setprecision(3);

    cout << "=====================================================" << endl;
    cout << "RESEARCH ON THE EFFICIENCY OF THE MINIMUM FINDING OPERATION" << endl;
    cout << "COMPILER: C++20/23 (ÐÅÆÈÌ RELEASE)" << endl;
    cout << "=====================================================" << endl;

    vector<size_t> data_sizes = { 10000000, 50000000 }; 

    for (size_t size : data_sizes) 
    {
        cout << "\n\n*** START OF TESTING (N = " << size << " elements) ***" << endl;

        vector<int> data = DataLoader::generate_data(size);
        cout << "Data generated. First element: " << data[0] << endl;

        MinExperiments exp(data);

        exp.run_library_tests();

        exp.run_custom_parallel_tests();

        exp.print_report();

        cout << "\n*** END OF TESTING (N = " << size << ") ***" << endl;
        cout << "-----------------------------------------------------" << endl;
    }

    return 0;
}