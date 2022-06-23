#pragma once

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

    static inline const Matrix *create(Matrix::Type type, Matrix::InputType fileType, std::string filename, std::function<double(const rapidcsv::Document *, int r1, int r2)> dist_fn)
    {
        switch (fileType)
        {
        case DIST:
            return createFromDist(type, filename);
        case CSV:
            return createFromCsv(type, filename, dist_fn);
        default:
            std::throw_with_nested(std::invalid_argument("Input Type not managed."));
        }
    };

private:
    static const Matrix *createFromDist(Matrix::Type type, std::string filename);
    static const Matrix *createFromCsv(Matrix::Type type, std::string filename, std::function<double(const rapidcsv::Document *, int r1, int r2)> dist_fn);

public:
    long dimension;
    std::vector<double> vec;
    std::string _filename;

    Matrix(std::string filename) : _filename(filename){};

    // for initialization; must be non-pure virtual functions!
    virtual long getSize(int N) const = 0;
    virtual bool store(int row, int col) const = 0;
    virtual int indexOf(int i, int j) const = 0;

    long getDimension() const
    {
        return dimension;
    }

    long getSize() const
    {
        return dimension * dimension;
    }

    double valueAt(int row, int col) const
    {
        if (row == col)
            return 0;
        int indexOf = this->indexOf(row, col);
        if (indexOf > -1 && indexOf < this->vec.size())
            return this->vec.at(indexOf);

        return -1;
    }

    void print() const
    {
        //   printVector(this->vec);
        std::cout << "Dimension: " << this->dimension << std::endl;
        for (int col = 0; col < this->dimension; ++col)
        {
            for (int row = 0; row < this->dimension; ++row)
            {

                std::cout << "[" << row << "," << col << "]:" << this->valueAt(row, col) << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "" << std::endl;
    }
};

// Store the matrix in a linear vector.
// Size: N*N
// No optimizations are performed
class LinearMatrix : public Matrix
{
public:
    LinearMatrix(std::string filename) : Matrix(filename){};

    int indexOf(int i, int j) const override
    {
        return i * dimension + j;
    }

    long getSize(int N) const override
    {
        return N * N;
    }

    // store all the values of the matrix
    bool store(int row, int col) const override
    {
        return true;
    }
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

    int indexOf(int i, int j) const override
    {
        int max = i > j ? i : j;
        int min = i < j ? i : j;
        return ((max * (max - 1)) / 2) + min;
    }

    long getSize(int N) const override
    {
        return N * (N - 1) / 2;
    }

    // store only a triangle of the matrix
    bool store(int row, int col) const override
    {
        return (row < col);
    }
};

const Matrix *Matrix::createFromCsv(Matrix::Type type, std::string filename, std::function<double(const rapidcsv::Document *, int r1, int r2)> dist_fn)
{
    Matrix *matrix = nullptr;
    if (type == Matrix::Type::LINEAR)
    {
        matrix = new LinearMatrix(filename);
    }
    else if (type == Matrix::Type::COL_MAJOR)
    {
        matrix = new ColMajorMatrix(filename);
    }
    else
    {
        std::throw_with_nested(std::invalid_argument("Matrix Type not managed."));
    }

    rapidcsv::Document doc(filename, rapidcsv::LabelParams(-1, -1));

    long N = doc.GetRowCount();

    matrix->dimension = N;
    long size = matrix->getSize(N);
    matrix->vec.reserve(size);

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {

            if (i != j)
            {
                double dist = dist_fn(&doc, i, j);

                if (matrix->store(i, j))
                {
                    matrix->vec.push_back(dist);
                }
            }
        }
    }

    return matrix;
}

const Matrix *Matrix::createFromDist(Matrix::Type type, std::string filename)
{
    Matrix *matrix = nullptr;
    if (type == Matrix::Type::LINEAR)
    {
        matrix = new LinearMatrix(filename);
    }
    else if (type == Matrix::Type::COL_MAJOR)
    {
        matrix = new ColMajorMatrix(filename);
    }
    else
    {
        std::throw_with_nested(std::invalid_argument("MAtrix Type not managed."));
    }

    std::ifstream inFile(filename);
    std::string line;

    if (!inFile.is_open())
    {
        std::cerr << putlocation(filename, 0, 0) << ": ERROR: could not open file." << std::endl;
        exit(1);
    }

    if (inFile.eof())
    {
        std::cerr << putlocation(filename, 0, 0) << ": ERROR: expecting first line to contain dimension of the matrix." << std::endl;
        exit(1);
    }

    int N;
    inFile >> N;

    if (N < 0)
    {
        std::cerr << putlocation(filename, 0, 0) << ": ERROR: expecting a square matrix." << std::endl;
        exit(1);
    }

    matrix->dimension = N;
    long size = matrix->getSize(N);
    matrix->vec.reserve(size);

    std::string in;
    for (int col = 0; col < N; ++col)
    {
        for (int row = 0; row < N; ++row)
        {
            // check unexpected EOF
            if (inFile.eof())
            {
                std::cerr << putlocation(filename, row, col) << ": ERROR: unexpected EOF." << std::endl;
                exit(1);
            }

            // read next token as string
            inFile >> in;
            double value = atof(in.c_str());

            if (matrix->store(row, col))
            {
                matrix->vec.push_back(value);
            }
        }
    }

    return matrix;
}