#include <iostream>

#include "col_major.h"
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

    ColMajorMatrix matrix(argv[1]);
    //matrix.print();
    //std::cout << "------------" << std::endl;

    Slink slink;
    slink.execute(&matrix);
    
    std::cout << "PI:" << std::endl;
    printVector(slink.getPi());
    std::cout << "------------" << std::endl;

    std::cout << "LAMBDA:" << std::endl;
    printVector(slink.getLambda());
    std::cout << "------------" << std::endl;

    return 0;
}
