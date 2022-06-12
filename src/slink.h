#pragma once

#include <iostream>
#include <vector>
#include <numeric>
#include <cmath>
#include <iterator>
#include <thread>

#include <condition_variable>

#include "matrix.h"
#include "util.h"

class Slink
{
public:
    enum ExecType {SEQUENTIAL, PARALLEL_OMP, SEQUENTIAL_SPLIT, PARALLEL_SPLIT };
    static inline const Slink execute(Matrix *matrix, int num_threads, ExecType et) {
        switch (et)
        {
            case SEQUENTIAL:  return Slink::__sequential(matrix);
            case PARALLEL_OMP:  return Slink::__parallel__omp(matrix, num_threads);
            case SEQUENTIAL_SPLIT: return Slink::__sequential__split(matrix, num_threads);
            case PARALLEL_SPLIT: return Slink::__parallel__split(matrix, num_threads);
            default:        
                std::throw_with_nested( std::invalid_argument("Execution Type not managed.") );
        }
    }

    static inline const std::string executionTypeToString(ExecType et) {
    switch (et)
        {
            case SEQUENTIAL:   return "Sequential";
            case PARALLEL_OMP:   return "Parallel (OMP)";
            case SEQUENTIAL_SPLIT: return "Sequential (SPLIT)";
            case PARALLEL_SPLIT: return "Parallel (SPLIT)";
            default:
                std::throw_with_nested( std::invalid_argument("Execution Type not managed.") );
        }
    }

private:
    static Slink __sequential(Matrix *matrix);
    static Slink __parallel__omp(Matrix *matrix, int num_threads);
    static Slink __sequential__split(Matrix *matrix, int num_threads);
    static Slink __parallel__split(Matrix *matrix, int num_threads);

public:
    Slink(std::vector<int> pi, std::vector<double> lambda): _pi(pi), _lambda(lambda) {}

    std::vector<int> getPi() { return _pi; }
    std::vector<double> getLambda() { return _lambda; }

    void setPi(std::vector<int> pi) { _pi = pi; }
    void setLambda(std::vector<double> lambda) { _lambda = lambda; }

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
    std::vector<int> _pi;
    std::vector<double> _lambda;
};

Slink Slink::__sequential(Matrix *matrix)
{
    std::vector<int> pi;
    std::vector<double> lambda;
    std::vector<double> M;

    for (int n = 0; n < matrix->getDimension(); ++n)
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
            if (lambda[i] >= lambda[pi[i]]) {
                pi[i] = n;
            }
        }
    }

    return Slink(pi, lambda);
}

Slink Slink::__parallel__omp(Matrix *matrix, int num_threads)
{
    std::vector<int> pi;
    std::vector<double> lambda;
    std::vector<double> M;
    int dim = matrix->getDimension();
    pi.reserve(dim);
    lambda.reserve(dim);

    for (int n = 0; n < dim; ++n)
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

    return Slink(pi, lambda);
}

void __split_aux(Matrix *matrix, int start, int end, Slink *out_slink) {
    std::vector<int> pi;
    std::vector<double> lambda;
    std::vector<double> M;

    for (int n = start; n < end; ++n)
    {
        // INIT
        pi.push_back(n);
        lambda.push_back(INF);
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

Slink* __split_merge(Matrix *matrix, Slink *slink1, Slink *slink2) {
    std::vector<int> pi(slink1->getPi().size() + slink2->getPi().size());
    std::vector<double> lambda(slink1->getLambda().size() + slink2->getLambda().size());
    
    int len1 = slink1->getPi().size();
    int len2 = slink2->getPi().size();

    // std::cout << "=======================" << std::endl;
    // std::cout << "Pi 1:" << std::endl;
    // printVectorHoriz(slink1->getPi());
    // std::cout << "Pi 2:" << std::endl;
    // printVectorHoriz(slink2->getPi());
    // std::cout << "Lambda 1:" << std::endl;
    // printVectorHoriz(slink1->getLambda());
    // std::cout << "Lambda 2:" << std::endl;
    // printVectorHoriz(slink2->getLambda());

    // ciclo i nodi della prima meta'
    for (int i = 0; i < len1; ++i) {
        
        // assumo che i valori attuali siano ottimali
        pi.at(i) = slink1->getPi().at(i);
        lambda.at(i) = slink1->getLambda().at(i);

        // calcolo la distanza con ciascun nodo della seconda meta'
        // ovvero merge!
        for (int j=0; j < len2; ++j) {
            double dist = matrix->valueAt(i,j+len1);

            // se ho trovato una distanza minore
            // aggiorno i valori di pi e lambda 
            if (lambda.at(i) > dist) {
                pi.at(i) = slink2->getPi().at(j); // il nuovo nodo finale del cluster
                lambda.at(i) = dist;    // la nuova distanza
            }
        }
    }

    // i nodi nella seconda meta' sono gia' sistemati
    // li unisco al cluster risultante
    for (int j = 0; j < len2; ++j) {
        pi.at(len1+j) = slink2->getPi().at(j);
        lambda.at(len1+j) = slink2->getLambda().at(j);
    }


    return new Slink(pi, lambda);
}

Slink Slink::__sequential__split(Matrix *matrix, int num_threads)
{
    int dim = matrix->getDimension();

    int mid_point = dim / 2;

    Slink *slink1 = new Slink({},{});
    __split_aux(matrix, 0, mid_point, slink1);
    Slink *slink2 = new Slink({},{});;
    __split_aux(matrix, mid_point, dim, slink2);
    Slink *result = __split_merge(matrix, slink1, slink2);

    printVector(result->getPi());
    printVector(result->getLambda());

    return *result;
}

Slink Slink::__parallel__split(Matrix *matrix, int num_threads)
{
    std::vector<double> M;
    int dim =  matrix->getDimension();

    std::vector<std::thread> workers(num_threads);
    std::vector<Slink *> partial_results;

    // Load Balancing
    // each thread get (dim/num_threads) job 
    const int load_per_thread = (int)(std::floor(dim / num_threads));
    int num_jobs_remaining = dim % num_threads;
    int num_jobs_assigned_so_far = 0;

    for (int tid=0; tid < num_threads; ++tid) {
        /*
        adjust the load by adding one job to this thread.
        num_jobs_remaining will be for sure less than the number of jobs.

        - best case: no thread gets the additional job (ie: dim % num_threads == 0)
        - worst case: all threads except one gets the additional job (ie dim % num_threads == num_threads-1)
        */
        int num_jobs_to_assign = load_per_thread;
        if (num_jobs_remaining > 0) {
            ++num_jobs_to_assign;
            --num_jobs_remaining;
        }

        int start = num_jobs_assigned_so_far;
        int end = std::min(start + num_jobs_to_assign, dim);

        num_jobs_assigned_so_far += num_jobs_to_assign;
        
        // std::cout << "["<< tid <<"] -> ("<<start<<","<<end-1<<")"<<std::endl;
        Slink *slink = new Slink({},{});
        partial_results.push_back(slink);
        workers.push_back( std::thread(__split_aux, matrix, start, end, slink) );
    }
    
    for (std::thread &t: workers) { 
        if (t.joinable()) {
            t.join();
        }
    }
    
    if (partial_results.size() == 0)
        return Slink({},{});

    if (partial_results.size() == 1)
        return *(partial_results.at(0));
    
    Slink *result = __split_merge(matrix, partial_results.at(0), partial_results.at(1));
    for (int i = 2; i < partial_results.size(); i++) {
        result = __split_merge(matrix, result, partial_results.at(i));
    }

    return *result;
}
