#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <functional>

#include "util.h"
#include "../lib/rapidcsv.h"

class Matrix
{
public:
    enum Type
    {
        LINEAR,
        COL_MAJOR
    };

    enum InputType
    {
        DIST,
        CSV
    };

    long dimension;
    std::vector<double> vec;
    std::string _filename;

    Matrix(std::string filename) : _filename(filename){};

    static const Matrix *create(
        Matrix::Type type,
        Matrix::InputType fileType,
        std::string filename,
        std::function<double(const rapidcsv::Document *, int r1, int r2)> dist_fn
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
    static const Matrix *createFromCsv(Matrix::Type type, std::string filename, std::function<double(const rapidcsv::Document *, int r1, int r2)> dist_fn);
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