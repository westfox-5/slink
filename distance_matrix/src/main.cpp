#include <iostream>

#include "col_major.h"

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

    ColMajorMatrix<char> cm(argv[1]);

    cm.print();

    return 0;
}

