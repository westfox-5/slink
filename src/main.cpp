#include <iostream>
#include <chrono>

#include "matrix.h"
#include "slink.h"

void usage(std::string name)
{
    std::cout << "Usage: " << name << " <input> [<matrix-type> <num-threads>]" << std::endl;
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

    auto begin = std::chrono::high_resolution_clock::now();
    Matrix *matrix = Matrix::create(matrix_type, input_file);
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

    std::cout << "Reading file: " << input_file << std::endl;
    std::cout << "Time: " << elapsed.count() << " ms." << std::endl;
    std::cout << "Matrix size (MB): " << matrix->getSize() * sizeof(double) / 1000.0 / 1000.0 << std::endl;

    begin = std::chrono::high_resolution_clock::now();
    Slink slink_sequential = Slink::execute_sequential(matrix);
    end = std::chrono::high_resolution_clock::now();
    elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    
    std::cout << "-------------------------------" << std::endl;
    std::cout << "Slink Sequential execution" << std::endl;
    std::cout << "Time: " << elapsed.count() << " ms." << std::endl;

    auto check_sequential = slink_sequential.checkValue();

    if (argc > 3)
    {
        auto num_threads = atoi(argv[3]);
        if (num_threads > -1)
        {
            begin = std::chrono::high_resolution_clock::now();

            Slink slink_parallel = Slink::execute_parallel(matrix, num_threads);

            end = std::chrono::high_resolution_clock::now();
            elapsed =
                std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
            std::cout << "-------------------------------" << std::endl;
            std::cout << "Slink Parallel execution (" << num_threads << " threads)" << std::endl;
            std::cout << "Time: " << elapsed.count() << " ms." << std::endl;

            auto check_parallel = slink_parallel.checkValue();

            std::cout << "-------------------------------" << std::endl;
            if (check_sequential == check_parallel)
            {
                std::cout << std::fixed << "Check values are EQUAL (" << check_sequential << ")" << std::endl;
            }
            else
            {
                std::cout << std::fixed << "Check values are NOT EQUAL (s: " << check_sequential << ", p: " << check_parallel << ")" << std::endl;
            }
        }
    }
    else
    {
        std::cout << "-------------------------------" << std::endl;
        std::cout << std::fixed << "Check value (s: " << check_sequential << ")" << std::endl;
    }

    delete matrix;

    return 0;
}
