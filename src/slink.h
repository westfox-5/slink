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
public:
    enum ExecType
    {
        SEQUENTIAL,
        PARALLEL_OMP,
        SEQUENTIAL_SPLIT,
        PARALLEL_SPLIT
    };
    static inline const Slink *execute(const Matrix *matrix, int num_threads, ExecType et)
    {
        switch (et)
        {
        case SEQUENTIAL:
            return Slink::__sequential(matrix);
        case PARALLEL_OMP:
            return Slink::__parallel__omp(matrix, num_threads);
        case SEQUENTIAL_SPLIT:
            return Slink::__sequential__split(matrix, num_threads);
        case PARALLEL_SPLIT:
            return Slink::__parallel__split(matrix, num_threads);
        default:
            std::throw_with_nested(std::invalid_argument("Execution Type not managed."));
        }
    }

    static inline const std::string executionTypeToString(ExecType et)
    {
        switch (et)
        {
        case SEQUENTIAL:
            return "Sequential";
        case PARALLEL_OMP:
            return "Parallel (OMP)";
        case SEQUENTIAL_SPLIT:
            return "Sequential (SPLIT)";
        case PARALLEL_SPLIT:
            return "Parallel (SPLIT)";
        default:
            std::throw_with_nested(std::invalid_argument("Execution Type not managed."));
        }
    }

    static inline Slink *empty()
    {
        return new Slink({}, {});
    }

private:
    static const Slink *__sequential(const Matrix *matrix);
    static const Slink *__parallel__omp(const Matrix *matrix, int num_threads);
    static const Slink *__sequential__split(const Matrix *matrix, int num_threads);
    static const Slink *__parallel__split(const Matrix *matrix, int num_threads);

public:
    Slink(std::vector<int> pi, std::vector<double> lambda) : _pi(pi), _lambda(lambda)
    {
        block_id = -1;
    }

    std::vector<int> getPi() const { return _pi; }
    std::vector<double> getLambda() const { return _lambda; }

    void setPi(std::vector<int> pi) { _pi = pi; }
    void setLambda(std::vector<double> lambda) { _lambda = lambda; }

    double checkValue() const
    {
        int piSum = std::accumulate(_pi.begin(), _pi.end(),
                                    0, [](int a, int b)
                                    { return a + b; });
        double lambdaSum = std::accumulate(_lambda.begin(), _lambda.end(),
                                           0.0, [](double a, double b)
                                           {
                if (a != INF && b != INF)
                    return a + b;
                if (a == INF)
                    return b;
                if (b == INF)
                    return a;
                return 0.0; });

        return piSum + lambdaSum;
    }

    void print() const
    {
        if (block_id != -1)
        {
            std::cout << "SLINK #" << block_id << " [" << start << ", " << end << "]" << std::endl;
        }

        std::cout << "      ";
        printVectorIndexHoriz(getPi().size());
        std::cout << "      ";
        printSep(getPi().size());
        std::cout << " π[i] ";
        printVectorHoriz(getPi());
        std::cout << " λ[i] ";
        printVectorHoriz(getLambda());
    }

public:
    std::vector<int> _pi;
    std::vector<double> _lambda;
    int block_id;
    int start, end;
};

const Slink *Slink::__sequential(const Matrix *matrix)
{
    std::vector<int> pi;
    std::vector<double> lambda;
    std::vector<double> M;

    int size = matrix->getDimension();

    for (int n = 0; n < size; ++n)
    {
        // INIT
        pi.push_back(n);
        lambda.push_back(INF);
        M.clear();
        M.reserve(n);

        for (int i = 0; i < n; ++i)
        {
            M.push_back(matrix->valueAt(i, n));
        }

        // UPDATE
        for (int i = 0; i < n; ++i)
        {
            if (lambda[i] >= M[i])
            {
                M[pi[i]] = std::min(M[pi[i]], lambda[i]);
                lambda[i] = M[i];
                pi[i] = n;
            }
            else
            {
                M[pi[i]] = std::min(M[pi[i]], M[i]);
            }
        }

        // FINALIZE
        for (int i = 0; i < n; ++i)
        {
            if (lambda[i] >= lambda[pi[i]])
            {
                pi[i] = n;
            }
        }
    }

    Slink *slink = new Slink(pi, lambda);
    slink->start = 0;
    slink->end = size - 1;
    slink->block_id = 0;
    return slink;
}

const Slink *Slink::__parallel__omp(const Matrix *matrix, int num_threads)
{
    std::vector<int> pi;
    std::vector<double> lambda;
    std::vector<double> M;
    int size = matrix->getDimension();

    for (int n = 0; n < size; ++n)
    {
        // INIT
        pi.push_back(n);
        lambda.push_back(INF);
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
            double _pi_value = pi[i];
            double min_1 = std::min(M[_pi_value], lambda[i]);
            double min_2 = std::min(M[_pi_value], M[i]);

            if (lambda[i] >= M[i])
            {
                M[_pi_value] = min_1;
                lambda[i] = M[i];
                pi[i] = n;
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
            if (lambda[i] >= lambda[pi[i]])
            {
                pi[i] = n;
            }
        }
    }

    Slink *slink = new Slink(pi, lambda);
    slink->start = 0;
    slink->end = size - 1;
    slink->block_id = 0;
    return new Slink(pi, lambda);
}

void __split_aux(const Matrix *matrix, int start, int end, Slink *out_slink)
{
    std::vector<int> pi(end - start);
    std::vector<double> lambda(end - start);
    std::vector<double> M;

    for (int n = start; n < end; ++n)
    {
        // INIT
        pi[n - start] = n;
        lambda[n - start] = INF;

        M.clear();
        M.reserve(n - start);

        for (int i = start; i < n; ++i)
        {
            M[i - start] = matrix->valueAt(i, n);
        }

        // UPDATE
        for (int i = start; i < n; ++i)
        {
            int _pi_value = pi[i - start];
            double min_1 = std::min(M[_pi_value - start], lambda[i - start]);
            double min_2 = std::min(M[_pi_value - start], M[i - start]);

            if (lambda[i - start] >= M[i - start])
            {
                M[_pi_value - start] = min_1;
                lambda[i - start] = M[i - start];
                pi[i - start] = n;
            }
            else
            {
                M[_pi_value - start] = min_2;
            }
        }

        // FINALIZE
        for (int i = start; i < n; ++i)
        {
            if (lambda[i - start] >= lambda[pi[i - start] - start])
            {
                pi[i - start] = n;
            }
        }
    }

    out_slink->setPi(pi);
    out_slink->setLambda(lambda);
}

Slink *__split_merge(const Matrix *matrix, const Slink *slink1, const Slink *slink2)
{
    int len1 = slink1->getPi().size();
    int len2 = slink2->getPi().size();
    int size = len1 + len2;

    std::vector<int> pi(size);
    std::vector<double> lambda(size);

    // std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" <<std::endl;
    // slink1->print();
    // slink2->print();

    // pre-load values of two slinks in the output vectors
    for (int i = 0; i < len1; ++i)
    {
        lambda[i] = slink1->_lambda[i];
        pi[i] = slink1->_pi[i];
    }
    for (int j = 0; j < len2; ++j)
    {
        lambda[len1 + j] = slink2->_lambda[j];
        pi[len1 + j] = slink2->_pi[j];
    }

    // loop through slink1:
    //  if a distance less than the current lambda[i] is found in the matrix
    //  replace the cluster of node i to the corresponding cluster in slink2 (slink2->lambda[j])
    for (int i = len1 - 1; i >= 0; --i)
    {
        for (int j = len2 - 1; j >= 0; --j)
        {
            double matrix_dist = matrix->valueAt(i, len1 + j);

            if (lambda[i] > matrix_dist)
            {
                int new_pi = j == 0 ? len1 : slink2->_pi[j];
                /*
                std::cout << " Changing: lambda["<<i<<"]="<<lambda[i] << " with matrix_dist.at("<<i<<","<< len1 + j<<")="<< matrix_dist<<". new pi["<<i<<"]="<< new_pi <<std::endl
                << "\tpi["<<i<<"]="<<pi[i]<< std::endl
                << "\tlen1="<<len1<< std::endl
                << "\tslink2->_pi["<<j<<"]="<<slink2->_pi[j]<< std::endl << std::endl;
                */
                lambda[i] = matrix_dist;
                pi[i] = new_pi; // the cluster to which node i will join
            }
        }
    }

    Slink *result = new Slink(pi, lambda);
    result->start = std::min(slink1->start, slink2->start);
    result->end = std::max(slink1->end, slink2->end);
    result->block_id = std::min(slink1->block_id, slink2->block_id);

    return result;
}

const Slink *Slink::__sequential__split(const Matrix *matrix, int num_blocks)
{
    int dimension = matrix->getDimension();
    std::vector<Slink *> partial_results(num_blocks);
    // Load Balancing
    const int block_size = (int)(std::floor(dimension / (num_blocks)));
    int num_remaining_blocks = dimension % num_blocks;

    std::cout << "Load balance:" << std::endl;
    std::cout << "\tdimension:" << dimension << std::endl;
    std::cout << "\tnum_blocks:" << num_blocks << std::endl;
    std::cout << "\tblock_size:" << block_size << std::endl;
    std::cout << "\tnum_remaining_blocks:" << num_remaining_blocks << std::endl;

    int total_jobs_assigned = 0;

    for (int block_id = 0; block_id < num_blocks; ++block_id)
    {
        int num_jobs_to_assign = block_size;
        if (num_remaining_blocks > 0)
        {
            --num_remaining_blocks;
            ++num_jobs_to_assign;
        }

        int start = total_jobs_assigned;
        int end = std::min(start + num_jobs_to_assign, dimension);
        total_jobs_assigned += num_jobs_to_assign;

        //std::cout << "\t("<<start<<","<<end<<")"<<std::endl;

        Slink *slink = Slink::empty();
        slink->start = start;
        slink->end = end - 1;
        slink->block_id = block_id;
        partial_results[block_id] = slink;

        __split_aux(matrix, start, end, slink);
    }

    if (num_blocks == 0)
        return Slink::empty();

    Slink *result = partial_results.at(0);
    for (int i = 1; i < num_blocks; ++i)
    {
        result = __split_merge(matrix, result, partial_results.at(i));
    }

    return result;
}

const Slink *Slink::__parallel__split(const Matrix *matrix, int num_threads)
{
    int dimension = matrix->getDimension();
    std::vector<Slink *> partial_results(num_threads);
    // Load Balancing
    const int block_size = (int)(std::floor(dimension / (num_threads)));
    int num_remaining_blocks = dimension % num_threads;

    std::cout << "Load balance:" << std::endl;
    std::cout << "\tdimension:" << dimension << std::endl;
    std::cout << "\tnum_threads:" << num_threads << std::endl;
    std::cout << "\tblock_size:" << block_size << std::endl;
    std::cout << "\tnum_remaining_blocks:" << num_remaining_blocks << std::endl;

    int total_jobs_assigned = 0;

    std::vector<std::thread> threads(num_threads);

    for (int block_id = 0; block_id < num_threads; ++block_id)
    {
        int num_jobs_to_assign = block_size;
        if (num_remaining_blocks > 0)
        {
            --num_remaining_blocks;
            ++num_jobs_to_assign;
        }

        int start = total_jobs_assigned;
        int end = std::min(start + num_jobs_to_assign, dimension);
        total_jobs_assigned += num_jobs_to_assign;

        //std::cout << "\t("<<start<<","<<end<<")"<<std::endl;

        Slink *slink = Slink::empty();
        slink->start = start;
        slink->end = end - 1;
        slink->block_id = block_id;
        partial_results[block_id] = slink;

        threads[block_id] = std::thread(__split_aux, matrix, start, end, slink);
    }

    for (int i = 0; i < threads.size(); ++i)
    {
        threads[i].join();
    }

    if (num_threads == 0)
        return Slink::empty();

    Slink *result = partial_results.at(0);
    for (int i = 1; i < num_threads; ++i)
    {
        result = __split_merge(matrix, result, partial_results.at(i));
    }

    return result;
}
