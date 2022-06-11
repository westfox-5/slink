#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <functional>

#include "matrix.h"
#include "slink.h"

inline void usage(std::string name)
{
    std::cout << "Usage: " << name << " <input> [<matrix-type> <num-threads>]" << std::endl;
}

void execute(Matrix* matrix, int num_threads, Slink::ExecType executionType) 
{
    
    auto begin = std::chrono::high_resolution_clock::now();
    Slink slink = Slink::execute(matrix, num_threads, executionType);
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << std::endl << Slink::executionTypeToString(executionType) << std::endl;
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << " μs." << std::endl;
    std::cout << std::fixed << "Check value: " << slink.checkValue() << std::endl;
}

int main(int argc, char *const argv[])
{
    if (argc == 1)
    {
        usage(argv[0]);
        std::cerr << "ERROR: no input file provided" << std::endl;
        exit(1);
    }
    std::string input_file = argv[1];

    Matrix::Type matrix_type = Matrix::Type::column_major;
    if (argc > 2)
    {
        std::string matrix_type_in(argv[2]);
        if (matrix_type_in.compare("simple") == 0)
            matrix_type = Matrix::Type::simple;
        else if (matrix_type_in.compare("column_major") == 0)
            matrix_type = Matrix::Type::column_major;
        else
        {
            usage(argv[0]);
            std::cerr << "ERROR: invalid matrix type: " << matrix_type_in << std::endl;
            exit(1);
        }
    }

    int num_threads = 1;
    if (argc > 3)
    {
        num_threads = atoi(argv[3]);
    }


    auto begin = std::chrono::high_resolution_clock::now();
    Matrix *matrix = Matrix::create(matrix_type, input_file);
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Reading file '" << input_file << "'"<<
        " took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms." <<
        " Matrix size is " << matrix->getSize() * sizeof(double) / 1000.0 / 1000.0 << " MB." << std::endl;

    // execute(matrix, num_threads, Slink::ExecType::SEQUENTIAL);
    // execute(matrix, num_threads, Slink::ExecType::PARALLEL_OMP);

    execute(matrix, num_threads, Slink::ExecType::SEQUENTIAL_SPLIT);
    execute(matrix, num_threads, Slink::ExecType::PARALLEL_SPLIT);

    delete matrix;

    return 0;
}
