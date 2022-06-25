#ifndef SLINK_H
#define SLINK_H

#pragma once

#include <iostream>
#include <vector>
#include <numeric>
#include <cmath>
#include <iterator>
#include <thread>

#include "matrix.h"
#include "util.h"

class Slink
{
private:
    static const Slink *__sequential(const Matrix *matrix);
    static const Slink *__parallel__omp(const Matrix *matrix, int num_threads);
    static const Slink *__sequential__split(const Matrix *matrix, int num_threads);
    static const Slink *__parallel__split(const Matrix *matrix, int num_threads);
    static const Slink *__parallel__split_omp(const Matrix *matrix, int num_threads);

public:
    int id_{-1};
    int start_{-1}, end_{-1};
    std::vector<int> pi_;
    std::vector<double> lambda_;

    enum ExecType
    {
        SEQUENTIAL,
        PARALLEL_OMP,
        SEQUENTIAL_SPLIT,
        PARALLEL_SPLIT,
        PARALLEL_SPLIT_OMP
    };

    Slink(std::vector<int> t_pi, std::vector<double> t_lambda) : pi_(t_pi), lambda_(t_lambda) {}
    static Slink *empty();
    double checkValue() const;
    void print() const;
    int size() const;
};

#endif
