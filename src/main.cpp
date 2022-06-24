#include <iostream>
#include <chrono>
#include <vector>
#include <string>

#include "matrix.h"
#include "slink.h"

/// TO OVERRIDE
double distance_function(const rapidcsv::Document *doc, int row1, int row2) 
{
    double l1 = doc->GetCell<double>(10, row1);
    double w1 = doc->GetCell<double>(11, row1);
    double h1 = doc->GetCell<double>(12, row1);


    double l2 = doc->GetCell<double>(10, row2);
    double w2 = doc->GetCell<double>(11, row2);
    double h2 = doc->GetCell<double>(12, row2);

    return std::sqrt((l1-l2)*(l1-l2) + (w1-w2)*(w1-w2) + (h1-h2)*(h1-h2));
}


inline void usage(std::string name)
{
    std::cout << "Usage: " << name << " <file-type> <file-input> [<matrix-type> <num-threads>]" << std::endl;
}

void execute(const Matrix *matrix, int num_threads, Slink::ExecType executionType)
{
    std::cout << std::endl;
    std::cout << Slink::executionTypeToString(executionType) << std::endl;

    auto begin = std::chrono::high_resolution_clock::now();
    const Slink *slink = Slink::execute(matrix, num_threads, executionType);
    auto end = std::chrono::high_resolution_clock::now();

    if (matrix->getDimension() < 20)
        slink->print();

    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << " μs." << std::endl;
    std::cout << std::fixed << "Check value: " << std::setprecision(3) << slink->checkValue() << std::endl;

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
    std::string input_file = argv[2];

    Matrix::InputType input_file_type;
    std::string input_file_type_in = argv[1];
    if (input_file_type_in.compare("dist") == 0)
        input_file_type = Matrix::InputType::DIST;
    else if (input_file_type_in.compare("csv") == 0)
        input_file_type = Matrix::InputType::CSV;
    else
    {
        usage(argv[0]);
        std::cerr << "ERROR: invalid file input type: " << input_file_type << std::endl;
        exit(1);
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
    const Matrix *matrix = Matrix::create(matrix_type, input_file_type, input_file, distance_function);
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Parsing of file '" << input_file << "' took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms." << std::endl
              << "Matrix size is " << matrix->getSize() * sizeof(double) / 1000.0 / 1000.0 << " MB." << std::endl;

    execute(matrix, num_threads, Slink::ExecType::SEQUENTIAL);
    //execute(matrix, num_threads, Slink::ExecType::PARALLEL_OMP);

    execute(matrix, num_threads, Slink::ExecType::SEQUENTIAL_SPLIT);
    // execute(matrix, num_threads, Slink::ExecType::PARALLEL_SPLIT);

    delete matrix;

    return 0;
}
