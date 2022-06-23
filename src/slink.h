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
    Slink(std::vector<int> t_pi, std::vector<double> t_lambda) : pi_(t_pi), lambda_(t_lambda) { }

    double checkValue() const
    {
        int piSum = std::accumulate(pi_.begin(), pi_.end(),
                                    0, [](int a, int b)
                                    { return a + b; });
        double lambdaSum = std::accumulate(lambda_.begin(), lambda_.end(),
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
        if (id_ != -1)
        {
            std::cout << "SLINK #" << id_ << " [" << start_ << ", " << end_ << "]" << std::endl;
        }

        std::cout << "      ";
        printVectorIndexHoriz(size());
        std::cout << "      ";
        printSep(pi_.size());
        std::cout << " π[i] ";
        printVectorHoriz(pi_);
        std::cout << " λ[i] ";
        printVectorHoriz(lambda_);
    }

    int size() const 
    {
        return pi_.size();
    }

public:
    std::vector<int> pi_;
    std::vector<double> lambda_;
    int id_ { -1 };
    int start_ { -1 }, end_ { -1 };
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
    slink->start_ = 0;
    slink->end_ = size - 1;
    slink->id_ = 0;
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
            double pi__value = pi[i];
            double min_1 = std::min(M[pi__value], lambda[i]);
            double min_2 = std::min(M[pi__value], M[i]);

            if (lambda[i] >= M[i])
            {
                M[pi__value] = min_1;
                lambda[i] = M[i];
                pi[i] = n;
            }
            else
            {
                M[pi__value] = min_2;
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
    slink->start_ = 0;
    slink->end_ = size - 1;
    slink->id_ = 0;
    return new Slink(pi, lambda);
}

void __split_aux(const Matrix *matrix, int start, int end, Slink *out_slink, std::vector<double> **out_m)
{
    std::vector<int> pi(end - start);
    std::vector<double> lambda(end - start);
    std::vector<double>* M = new std::vector<double>();

    for (int n = start; n < end; ++n)
    {
        // INIT
        pi[n - start] = n;
        lambda[n - start] = INF;

        M->clear();
        M->reserve(n - start);

        for (int i = start; i < n; ++i)
        {
            M->push_back(matrix->valueAt(i, n));
        }        

        // UPDATE
        for (int i = start; i < n; ++i)
        {
            int pi__value = pi[i - start];
            double min_1 = std::min(M->at(pi__value - start), lambda[i - start]);
            double min_2 = std::min(M->at(pi__value - start), M->at(i - start));

            if (lambda[i - start] >= M->at(i - start))
            {
                M->at(pi__value - start) = min_1;
                lambda[i - start] = M->at(i - start);
                pi[i - start] = n;
            }
            else
            {
                M->at(pi__value - start) = min_2;
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

    out_slink->pi_.swap(pi);
    out_slink->lambda_.swap(lambda);
    
    *out_m = M;
}


Slink *__split_merge(const Matrix *matrix, const Slink *slink1, const Slink *slink2)
{
    int len1 = slink1->size();
    int len2 = slink2->size();
    int size = len1 + len2;
    
    std::vector<int> pi(slink1->pi_);
    std::vector<double> lambda(slink1->lambda_);
    pi.insert(pi.end(), slink2->pi_.begin(), slink2->pi_.end());
    lambda.insert(lambda.end(), slink2->lambda_.begin(), slink2->lambda_.end());

    std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" <<std::endl;
    slink1->print();
    slink2->print();


    for (int i=len1-1; i>=0; i--) {

        pi[i] = slink1->pi_[i];
        lambda[i] = slink1->lambda_[i];

        int cluster = slink1->pi_[i];
        double min_dist = slink1->lambda_[i];

        for (int j=0; j<len2; j++) {
            double d = matrix->valueAt(i, len1+j);
            if (d < min_dist) {
                min_dist = d;
                cluster = len1 + j;
            }
        }

        while (min_dist > lambda[cluster]) {
            cluster = pi[cluster];
        }

        if (cluster != pi[i]) {
            pi[i] = cluster;
            lambda[i] = min_dist;
        }
    }

    Slink *result = new Slink(pi, lambda);
    result->start_ = std::min(slink1->start_, slink2->start_);
    result->end_ = std::max(slink1->end_, slink2->end_);
    result->id_ = std::min(slink1->id_, slink2->id_);

    return result;
}

Slink *__split_merge2(const Matrix *matrix, const Slink *slink1, const Slink *slink2)
{
    int len1 = slink1->size();
    int len2 = slink2->size();
    int size = len1 + len2;

    std::vector<int> pi(size);
    std::vector<double> lambda(size);

    std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" <<std::endl;
    slink1->print();
    slink2->print();
    std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" <<std::endl;

    // pre-load values of two slinks in the output vectors
    for (int i = 0; i < len1; ++i)
    {
        lambda[i] = slink1->lambda_[i];
        pi[i] = slink1->pi_[i];
    }
    for (int j = 0; j < len2; ++j)
    {
        lambda[len1 + j] = slink2->lambda_[j];
        pi[len1 + j] = slink2->pi_[j];
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
                int newpi_;
                if (lambda[len1+j] < matrix_dist)
                    newpi_ = len1 + j;
                else {
                    int k = len1 + j;
                    while (lambda[k] < matrix_dist) ;
                    newpi_ = pi[k];
                }
                /*
                std::cout << " Changing: lambda["<<i<<"]="<<lambda[i] << " with matrix_dist.at("<<i<<","<< len1 + j<<")="<< matrix_dist<<". new pi["<<i<<"]="<< newpi_ <<std::endl
                << "\tpi["<<i<<"]="<<pi[i]<< std::endl
                << "\tlen1="<<len1<< std::endl
                << "\tslink2->pi_["<<j<<"]="<<slink2->pi_[j]<< std::endl << std::endl;
                */
                lambda[i] = matrix_dist;
                pi[i] = newpi_; // the cluster to which node i will join
            }
        }
        
        // //for (int k = 0; k < i; ++k)
        // for (int k = 0; k < i; ++k)
        // {
        //     if (lambda[k] >= lambda[pi[k]])
        //     {
        //         pi[k] = i;
        //     }
        // }

    }

    Slink *result = new Slink(pi, lambda);
    result->start_ = std::min(slink1->start_, slink2->start_);
    result->end_ = std::max(slink1->end_, slink2->end_);
    result->id_ = std::min(slink1->id_, slink2->id_);

    return result;
}

const Slink *Slink::__sequential__split(const Matrix *matrix, int num_blocks)
{
    int dimension = matrix->getDimension();
    std::vector<Slink *> partial_results(num_blocks);
    std::vector<std::vector<double> *> partial_ms(num_blocks);

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
        slink->start_ = start;
        slink->end_ = end - 1;
        slink->id_ = block_id;

        std::vector<double>* m = new std::vector<double>();

        __split_aux(matrix, start, end, slink, &m);

        partial_results[block_id] =slink ;
        partial_ms[block_id] = m;
    }

    if (num_blocks == 0)
        return Slink::empty();

    Slink *result = partial_results.at(0);
    std::vector<double>* m = partial_ms.at(0);
    for (int i = 1; i < num_blocks; ++i)
    {
        //result = __split_merge(matrix, result, partial_results.at(i), m, partial_ms.at(i));
        result = __split_merge(matrix, result, partial_results.at(i));
        //m->swap(*partial_ms.at(i));
    }

    return result;
}

const Slink *Slink::__parallel__split(const Matrix *matrix, int num_threads)
{
    return nullptr;
    /*
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
        slink->start_ = start;
        slink->end_ = end - 1;
        slink->id_ = block_id;
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
*/
}