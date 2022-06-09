#pragma once

#include <iostream>
#include <vector>
#include <numeric>
#include <cmath>

#include "matrix.h"
#include "util.h"

class Slink
{
public:
    static Slink execute_sequential(Matrix *matrix);
    static Slink execute_parallel(Matrix *matrix, int num_threads);

    std::vector<double> getPi() { return _pi; }
    std::vector<double> getLambda() { return _lambda; }

    double checkValue()
    {
        auto remove_inf = [](double a, double b)
        {
            if (a != INF && b != INF)
                return a + b;
            if (a == INF)
                return b;
            if (b == INF)
                return a;
            return 0.;
        };

        double piSum = std::accumulate(_pi.begin(), _pi.end(), 0., remove_inf);
        double lambdaSum = std::accumulate(_lambda.begin(), _lambda.end(), 0., remove_inf);

        return piSum + lambdaSum;
    }

private:
    std::vector<double> _pi;
    std::vector<double> _lambda;
};

Slink Slink::execute_sequential(Matrix *matrix)
{
    Slink slink;
    std::vector<double> M;

    for (int n = 0; n < matrix->getDimension(); ++n)
    {

        // INIT
        slink._pi.push_back(n);
        slink._lambda.push_back(INF);
        M.clear();
        M.reserve(n);
        for (int i = 0; i < n; ++i)
        {
            M.push_back(matrix->valueAt(i, n));
        }

        // UPDATE
        for (int i = 0; i < n; ++i)
        {
            if (slink._lambda[i] >= M[i])
            {
                M[slink._pi[i]] = std::min(M[slink._pi[i]], slink._lambda[i]);
                slink._lambda[i] = M[i];
                slink._pi[i] = n;
            }

            if (slink._lambda[i] < M[i])
            {
                M[slink._pi[i]] = std::min(M[slink._pi[i]], M[i]);
            }
        }

        // FINALIZE
        for (int i = 0; i < n; ++i)
        {
            if (slink._lambda[i] >= slink._lambda[slink._pi[i]])
            {
                slink._pi[i] = n;
            }
        }
    }

    return slink;
}

Slink Slink::execute_parallel(Matrix *matrix, int num_threads)
{
    Slink slink;
    std::vector<double> M;
    int dim = matrix->getDimension();

    //#pragma omp parallel for num_threads(num_threads) private(M)
    for (int n = 0; n < dim; ++n)
    {

        // INIT
        slink._pi.push_back(n);
        slink._lambda.push_back(INF);
        M.clear();
        M.reserve(n);

#pragma omp parallel for num_threads(num_threads)
        for (int i = 0; i < n; ++i)
        {
            M[i] = matrix->valueAt(i, n);
        }

        // UPDATE
        for (int i = 0; i < n; ++i)
        {
            if (slink._lambda[i] >= M[i])
            {
                M[slink._pi[i]] = std::min(M[slink._pi[i]], slink._lambda[i]);
                slink._lambda[i] = M[i];
                slink._pi[i] = n;
            }
            else
            { // slink._lambda[i] < M[i]
                M[slink._pi[i]] = std::min(M[slink._pi[i]], M[i]);
            }
        }

// FINALIZE
#pragma omp parallel for num_threads(num_threads)
        for (int i = 0; i < n; ++i)
        {
            if (slink._lambda[i] >= slink._lambda[slink._pi[i]])
            {
                slink._pi[i] = n;
            }
        }
    }

    return slink;
}
