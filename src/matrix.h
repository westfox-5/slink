#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <functional>

#include "util.h"
#include "dist_functions.h"
#include "../lib/rapidcsv.h"

class Matrix
{
public:
    enum Type
    {
        LINEAR,
        COL_MAJOR
    };

    enum FileType
    {
        DIST,
        CSV
    };

    long dimension_;
    std::vector<double> vec_;
    std::string filename_;

    Matrix(std::string t_filename) : filename_(t_filename){};

    static const Matrix *create(
        Matrix::Type t_matrixtype,
        Matrix::FileType t_filetype,
        std::string t_filename
    );

    // for initialization; must be non-pure virtual functions!
    virtual long getSize(int N) const = 0;
    virtual bool store(int row, int col) const = 0;
    virtual int indexOf(int i, int j) const = 0;

    long getDimension() const;
    long getSize() const;
    double valueAt(int row, int col) const;
    void print() const;

private:
    static const Matrix *createFromDist(Matrix::Type type, std::string filename);
    static const Matrix *createFromCsv(Matrix::Type type, std::string filename);
};

// Store the matrix in a linear vector.
// Size: N*N
// No optimizations are performed
class LinearMatrix : public Matrix
{
public:
    LinearMatrix(std::string filename) : Matrix(filename){};
    int indexOf(int i, int j) const override;
    long getSize(int N) const override;
    // store all the values of the matrix
    bool store(int row, int col) const override;
};

// Store the matrix in a linear vector.
// since it is a simmetric square matrix, we can safely store only half of the matrix, without the diagonal.
// Size:
//      Sum[i;1;N]{N-i}
// which is
//      N * (N-1) / 2
class ColMajorMatrix : public Matrix
{
public:
    ColMajorMatrix(std::string filename) : Matrix(filename){};
    int indexOf(int i, int j) const override;
    long getSize(int N) const override;
    // store only a triangle of the matrix
    bool store(int row, int col) const override;
};

#endif