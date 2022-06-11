#pragma once

#include <iostream>
#include <vector>
#include <numeric>
#include <cmath>
#include <thread>

#include "matrix.h"
#include "util.h"

class Slink
{
public:
    static Slink execute_sequential(Matrix *matrix);
    static Slink execute_parallel(Matrix *matrix, int num_threads);
    static Slink execute_parallel_threads(Matrix *matrix, int num_threads);
    static void aux(Matrix *matrix, Slink *slink, int n, int num_threads);
    static void aux_initialize(Matrix *matrix, std::vector<double> *M, Slink *slink, int n);
    static void aux_update(std::vector<double> *M, Slink *slink, int n, int i);
    static void aux_finalize(Slink *slink, int n, int i);

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
            else
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

std::mutex shared_update_mutex;
std::mutex shared_finalize_mutex;

Slink Slink::execute_parallel_threads(Matrix *matrix, int num_threads)
{
    Slink slink;
    std::vector<double> M;
    int dim =  matrix->getDimension();
    
    for (int n = 0; n < dim; n+=num_threads) {
        // threads.clear();
        for (int t = 0; t < num_threads; t++) {
            if ((n + t) < dim)
                Slink::aux(matrix, &slink, n + t, num_threads);
        }
    }

    return slink;
}

void Slink::aux(Matrix *matrix, Slink *slink, int n, int num_threads)
{
    std::vector<double> M;
    std::vector<std::thread> threads(num_threads);
    // INIT
    Slink::aux_initialize(matrix, &M, slink, n);

    // UPDATE
    for (int i = 0; i < n; i += num_threads) {
        threads.clear();
        threads.reserve(num_threads);
        for (int t = 0; t < num_threads; t++) {
            if ((i + t) < n) {
                threads[t] = std::thread(Slink::aux_update, &M, slink, n, i);
                threads[t].join();
            }
        }
    }

    // FINALIZE
    for (int i = 0; i < n; ++i)
    {
        Slink::aux_finalize(slink, n, i);
    }
}

void Slink::aux_initialize(Matrix *matrix, std::vector<double> *M, Slink *slink, int n)
{
    M->clear();
    M->reserve(n);
    for (int i = 0; i < n; ++i)
    {
        M->push_back(matrix->valueAt(i, n));
    }
    slink->_pi.push_back(n);
    slink->_lambda.push_back(INF);
}

void Slink::aux_update(std::vector<double> *M, Slink *slink, int n, int i)
{
    if (slink->_lambda[i] >= M->at(i))
    {
        M->at(slink->_pi[i]) = std::min(M->at(slink->_pi[i]), slink->_lambda[i]);
        slink->_lambda[i] = M->at(i);
        slink->_pi[i] = n;
    } else
    {
        M->at(slink->_pi[i]) = std::min(M->at(slink->_pi[i]), M->at(i));
    }
}

void Slink::aux_finalize(Slink *slink, int n, int i)
{
    if (slink->_lambda[i] >= slink->_lambda[slink->_pi[i]])
    {
        slink->_pi[i] = n;
    }
}

Slink Slink::execute_parallel(Matrix *matrix, int num_threads)
{
    Slink slink;
    std::vector<double> M;
    int dim = matrix->getDimension();

    #pragma omp parallel for num_threads(num_threads) private(M) ordered
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
        #pragma omp parallel for schedule(static) num_threads(num_threads)
        for (int i = 0; i < n; ++i)
        {
            double _pi_value = slink._pi[i];
            double min_1 = std::min(M[_pi_value], slink._lambda[i]);
            double min_2 = std::min(M[_pi_value], M[i]);

            if (slink._lambda[i] >= M[i])
            {
                M[_pi_value] = min_1;
                slink._lambda[i] = M[i];
                slink._pi[i] = n;
            }
            else
            {
                M[_pi_value] = min_2;
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
