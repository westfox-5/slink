#include <iostream>
#include <chrono>
#include <vector>
#include <string>

#include "dist_functions.h"
#include "slink_executors.h"

inline void usage(std::string name)
{
    std::cout << "Usage: " << name << " <file-type> <file-input> [<matrix-type> <num-threads>]" << std::endl;
}

void execute_wrapper(const Matrix *matrix, int num_threads, SlinkExecutors::type executionType)
{
    std::cout << SlinkExecutors::execution_type_to_string(executionType) << std::endl;

    auto begin = std::chrono::high_resolution_clock::now();
    const Slink *slink = SlinkExecutors::execute(matrix, num_threads, executionType);
    auto end = std::chrono::high_resolution_clock::now();

    if (matrix->getDimension() < 20)
        slink->print();

    std::cout << "\tTime: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << " μs." << std::endl;
    std::cout << std::fixed << "\tCheck value: " << std::setprecision(3) << slink->checkValue() << std::endl;

    delete (slink);
}

int main(int argc, char *const argv[])
{
    if (argc < 2)
    {
        usage(argv[0]);
        std::cerr << "ERROR: no input filename and type provided" << std::endl;
        exit(1);
    }
    std::string in_filename_str = argv[2];

    Matrix::FileType in_filetype;
    {
        std::string in_filetype_str = argv[1];
        if (in_filetype_str.compare("dist") == 0)
            in_filetype = Matrix::FileType::DIST;
        else if (in_filetype_str.compare("csv") == 0)
            in_filetype = Matrix::FileType::CSV;
        else
        {
            usage(argv[0]);
            std::cerr << "ERROR: invalid file input type: " << in_filetype_str << std::endl;
            exit(1);
        }
    }

    Matrix::Type matrix_type = Matrix::Type::LINEAR;
    if (argc > 3)
    {
        std::string matrix_type_in(argv[3]);
        if (matrix_type_in.compare("linear") == 0)
            matrix_type = Matrix::Type::LINEAR;
        else if (matrix_type_in.compare("col_major") == 0)
            matrix_type = Matrix::Type::COL_MAJOR;
        else
        {
            usage(argv[0]);
            std::cerr << "ERROR: invalid matrix type: " << matrix_type_in << std::endl;
            exit(1);
        }
    }

    int num_threads = 1;
    if (argc > 4)
    {
        num_threads = atoi(argv[4]);
    }

    auto begin = std::chrono::high_resolution_clock::now();
    const Matrix *matrix = Matrix::create(matrix_type, in_filetype, in_filename_str);
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << std::endl
            << "Parsing of file '" << in_filename_str << "' took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms." << std::endl
            << "Matrix size is " << matrix->getSize() * sizeof(double) / 1000.0 / 1000.0 << " MB." << std::endl;
    if (argc > 5)
    {
        std::vector<std::string> in_exectypes_str = split(std::string(argv[5]), ',');
        for (std::string exectype_str: in_exectypes_str) {
            int execution_type = std::stoi(exectype_str);
            if (execution_type <= SlinkExecutors::type::PARALLEL_SPLIT_OMP && execution_type >= SlinkExecutors::type::SEQUENTIAL)
            {
                execute_wrapper(matrix, num_threads, static_cast<SlinkExecutors::type>(execution_type));
            }
            else if (execution_type == -1) 
            {
                for (int type = SlinkExecutors::type::SEQUENTIAL;  type <= SlinkExecutors::type::PARALLEL_SPLIT_OMP; type++) {
                    execute_wrapper(matrix, num_threads, static_cast<SlinkExecutors::type>(type));
                }
            }
            else 
            {
                usage(argv[0]);
                std::cerr << "ERROR: invalid execution type. Execution type must be an integer between " << SlinkExecutors::type::SEQUENTIAL << " and " << SlinkExecutors::type::PARALLEL_SPLIT_OMP << std::endl;
                exit(1);
            }


        }
    }

    delete matrix;

    return 0;
}
