#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "util.h"

class Matrix {
protected:
    long dimension;

public:
    virtual int indexOf(int i, int j) = 0;
    virtual double valueAt(int row, int col) = 0;

    virtual void print() = 0;

    long getDimension() { return dimension; }
};

class ColMajorMatrix : public Matrix {
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
        std::cerr << putlocation(inFilename, 0, 0) << ": ERROR: could not open file." << std::endl;
        exit(1);
    }

    if ( inFile.eof() ) {	  
        std::cerr << putlocation(inFilename, 0, 0) << ": ERROR: expecting first line to contain dimension of the matrix." << std::endl;
        exit(1);
    }
    
    int N;
    inFile >> N;

    if ( N < 0 ) {
        std::cerr << putlocation(inFilename, 0, 0) << ": ERROR: expecting a square matrix." << std::endl;
        exit(1);
    } 

    long size = N*(N-1)/2;
    this->dimension = N;
    this->vec.reserve(size);

    std::string in;
    for (int col=0; col<N; ++col) {
        for (int row=0; row<N; ++row) {
            
            // check unexpected EOF
            if ( inFile.eof() ) {
                std::cerr << putlocation(inFilename, row, col) << ": ERROR: unexpected EOF." << std::endl;
                exit(1);
            } 

            // read next token as string
            inFile >> in; 

            // only store the half-top of the matrix, diagonal excluded
            if (row < col) {
                this->vec.push_back( atof(in.c_str()) );
            }
        }
    }
}

void ColMajorMatrix::print() {
//   printVector(this->vec);

    std::cout << "Dimension: " << this->dimension << std::endl;
    for (int col=0; col<this->dimension; ++col) {
        for (int row=0; row<this->dimension; ++row) {
            
            std::cout << "[" << row << "," << col << "]:" << this->valueAt(row, col) << " ";
        }
        std::cout<<std::endl;
    }
    std::cout << "" << std::endl;
}
