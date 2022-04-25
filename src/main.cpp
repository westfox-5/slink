#include <iostream>
#include <chrono>

#include "matrix.h"
#include "slink.h"

void usage(std::string name) 
{
    std::cout << "Usage: " << name << " <input>" << std::endl;
}

int main(int argc, char* const argv[]) {
    if (argc == 1) {
       usage(argv[0]);
       std::cerr << "ERROR: no input file provided" << std::endl;
       exit(1);
    }
    auto input_file = argv[1];

    auto begin = std::chrono::high_resolution_clock::now();
    Matrix *matrix = Matrix::create(Matrix::Type::simple, input_file);

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = 
      std::chrono::duration_cast<std::chrono::milliseconds>
      (end - begin);

    std::cout << "Reading file: " << input_file << std::endl;
    std::cout << "Time: " << elapsed.count() << " ms." << std::endl;
    
    // matrix->print();

    begin = std::chrono::high_resolution_clock::now();
    
    Slink slink = Slink::execute(matrix);

    end = std::chrono::high_resolution_clock::now();
    elapsed = 
      std::chrono::duration_cast<std::chrono::milliseconds>
      (end - begin);
    std::cout << "-------------------------------" << std::endl;
    std::cout << "Slink execution" << std::endl;
    std::cout << "Time: " << elapsed.count() << " ms." << std::endl;
    std::cout << std::fixed << "Check value: " << slink.checkValue() << std::endl;

    // std::cout << "PI:" << std::endl;
    // printVector(slink.getPi());
    // std::cout << "------------" << std::endl;
    // std::cout << "LAMBDA:" << std::endl;
    // printVector(slink.getLambda());

    delete matrix;

    return 0;
}
