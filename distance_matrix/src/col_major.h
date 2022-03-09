#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cassert>

#include "util.h"

template <typename T>
class ColMajorMatrix {

private:
    // Store the matrix in a linear vector.
    // since it is a triangular square matrix, we can safely store only half of the matrix, without the diagonal.
    // total size of an NxN matrix that we have to allocate is:
    //      Sum[i;1;N]{N-i}
    // which is
    //      N * (N-1) / 2
    std::vector<T> vec;
    long N;

    std::string filename;

public:
    ColMajorMatrix(std::string inFilename);
    void print();

    int indexOf(int i, int j) {
        if (i <= j)
            return ((j * (j - 1)) / 2) + i;
        return -1;
    }

    T valueAt(int row, int col) {
        int indexOf = this->indexOf(row, col);
        if (indexOf > -1)
            return this->vec.at(indexOf);
        return NULL;
    }
};

template <typename in_type>
ColMajorMatrix<in_type>::ColMajorMatrix(std::string inFilename)
{
    this->filename = inFilename;

    std::ifstream inFile(inFilename);
    std::string line;

    if ( !inFile.is_open() ) { 
        std::cerr << putlocation(inFilename, 0, 0) << ": ERROR: could not open file '" << inFilename << "'." << std::endl;
        exit(1);
    }

    if ( inFile.eof() ) {	  
        std::cerr << putlocation(inFilename, 0, 0) << ": ERROR: expecting first line of the input file '" << inFilename << "' to contain ROW and COL values separated by space" << std::endl;
        exit(1);
    }
    
    int rows, cols;
    inFile >> rows;
    inFile >> cols;

    if ( rows != cols ) {
        std::cerr << putlocation(inFilename, 0, 0) << ": ERROR: expecting a square matrix in input file '" << inFilename << "'." << std::endl;
        exit(1);
    } 

    long size = rows *(rows-1) /2;
    this->N = rows;
    this->vec.reserve(size);
    
    in_type ignore;
    in_type buff;
    for (int col=0; col<cols; ++col) {
        for (int row=0; row<rows; ++row) {
            if (row >= col) {
                inFile >> ignore;
            } else {
                inFile >> buff;
                this->vec.push_back(buff);
            }
        }
    }
}

template <typename in_type>
void ColMajorMatrix<in_type>::print() {
    std::cout.width(this->vec.size() * 2);
    std::cout.fill('-');
    std::cout << "" << std::endl;
    // print indices
    for(int i=0; i<this->vec.size(); ++i){
        std::cout << i << " ";
    }
    std::cout << std::endl;
    // print values
    for(int i=0; i<this->vec.size(); ++i){
        std::cout << this->vec.at(i) << " ";
    }
    std::cout << std::endl;
    std::cout.width(this->vec.size() * 2);
    std::cout.fill('-');
    std::cout << "" << std::endl;
}
