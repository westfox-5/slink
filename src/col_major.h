#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cassert>

#include "util.h"

class ColMajorMatrix {

private:
    // Store the matrix in a linear vector.
    // since it is a triangular square matrix, we can safely store only half of the matrix, without the diagonal.
    // total size of an NxN matrix that we have to allocate is:
    //      Sum[i;1;N]{N-i}
    // which is
    //      N * (N-1) / 2
    std::vector<double> vec;
    std::string filename;

public:
    long N;

    ColMajorMatrix(std::string inFilename);
    void print();

    int indexOf(int i, int j) {
        int max = i > j ? i : j;
        int min = i < j ? i : j;
        return ((max * (max - 1)) / 2) + min;
    }

    double valueAt(int row, int col) {
        if (row == col) 
            return 0;
        int indexOf = this->indexOf(row, col);
        if (indexOf > -1 && indexOf < this->vec.size())
            return this->vec.at(indexOf);
        
        return -1;
    }
};

ColMajorMatrix::ColMajorMatrix(std::string inFilename)
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
    
    std::string in;
    for (int col=0; col<cols; ++col) {
        for (int row=0; row<rows; ++row) {
            inFile >> in;
            if (row < col) {
                this->vec.push_back( atof(in.c_str()) );
            }
        }
    }
}

void ColMajorMatrix::print() {
//   printVector(this->vec);

    for (int col=0; col<this->N; ++col) {
        for (int row=0; row<this->N; ++row) {
            
            std::cout << "[" << row << "," << col << "]:" << this->valueAt(row, col) << " ";
        }
        std::cout<<std::endl;
    }
    std::cout << "" << std::endl;
}
