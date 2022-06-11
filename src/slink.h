#pragma once

#include <iostream>
#include <vector>
#include <numeric>
#include <cmath>
#include <thread>

#include <condition_variable>

#include "matrix.h"
#include "util.h"

class Slink
{
public:
    enum ExecType {SEQUENTIAL, PARALLEL_OMP, SEQUENTIAL_SPLIT };
    static inline const Slink execute(Matrix *matrix, int num_threads, ExecType et) {
        switch (et)
        {
            case SEQUENTIAL:  return Slink::execute_sequential(matrix);
            case PARALLEL_OMP:  return Slink::execute_parallel(matrix, num_threads);
            case SEQUENTIAL_SPLIT: return Slink::execute_split(matrix, num_threads);
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
            default:      return "[Unknown ExecType]";
        }
    }
private:
    static Slink execute_sequential(Matrix *matrix);
    static Slink execute_parallel(Matrix *matrix, int num_threads);
    
    static Slink execute_split(Matrix *matrix, int num_threads);
    static Slink execute_split_aux(Matrix *matrix, int start, int end);
    static Slink execute_split_merge(Matrix *matrix, Slink *slink1, Slink *slink2, int mid_point);


    static Slink execute_parallel_threads(Matrix *matrix, int num_threads);
    static void aux(Matrix *matrix, Slink *slink, int n);
    static void aux_initialize(Matrix *matrix, std::vector<double> *M, Slink *slink, int n);
    static void aux_update(std::vector<double> *M, Slink *slink, int n, int i);
    static void aux_finalize(Slink *slink, int n, int i);

    Slink(): Slink({}, {}) {};
    Slink(std::vector<int> pi, std::vector<double> lambda): _pi(pi), _lambda(lambda) {}

public:
    std::vector<int> getPi() { return _pi; }
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
    std::vector<int> _pi;
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

Slink Slink::execute_parallel_threads(Matrix *matrix, int num_threads)
{
    Slink slink;
    std::vector<double> M;
    int dim =  matrix->getDimension();
    std::vector<std::thread> threads;
    slink._lambda = std::vector<double>(dim);
    slink._pi = std::vector<int>(dim);
    
    for (int n = 0; n < dim; n+=num_threads) {
        threads.clear();
        threads.reserve(num_threads);
        for (int t = 0; t < num_threads && (n + t) < dim; t++) {
            threads.push_back(std::thread(Slink::aux, matrix, &slink, n + t));
        }
        
        for (int t = 0; t < num_threads && (n + t) < dim; t++) {
            threads[t].join();
        }
    }

    return slink;
}

void Slink::aux(Matrix *matrix, Slink *slink, int n)
{
    std::vector<double> M;
    
    // INIT
    Slink::aux_initialize(matrix, &M, slink, n);

    // UPDATE
    for (int i = 0; i < n; i++)
    {
        Slink::aux_update(&M, slink, n, i);
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
    }
    else
    {
        std::cout<< " [" << std::this_thread::get_id()  << "] :" << "len: "<< M->size() << "  -- idx: " << slink->_pi[i] << std::endl;
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
    slink._pi.reserve(dim);
    slink._lambda.reserve(dim);

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


Slink Slink::execute_split_aux(Matrix *matrix, int start, int end) {
    Slink slink;
    std::vector<double> M;
    for (int n = start; n < end; ++n)
    {
        // INIT
        slink._pi.push_back(n);
        slink._lambda.push_back(INF);
        M.clear();
        M.reserve(n - start);

        for (int i = start; i < n; ++i)
        {
            M[i - start] = matrix->valueAt(i, n);
        }


        
        // UPDATE        
        for (int i = start; i < n; ++i)
        {
            int _pi_value = slink._pi[i - start];
            double min_1 = std::min(M[_pi_value - start], slink._lambda[i - start]);
            double min_2 = std::min(M[_pi_value - start], M[i - start]);

            if (slink._lambda[i - start] >= M[i - start])
            {
                M[_pi_value - start] = min_1;
                slink._lambda[i - start] = M[i - start];
                slink._pi[i - start] = n;
            }
            else
            {
                M[_pi_value - start] = min_2;
            }
        }

        // FINALIZE
        for (int i = start; i < n; ++i)
        {
            if (slink._lambda[i - start] >= slink._lambda[slink._pi[i - start] - start])
            {
                slink._pi[i - start] = n;
            }
        }

    }

    return slink;
}

Slink Slink::execute_split_merge(Matrix *matrix, Slink *slink1, Slink *slink2, int mid_point) {
    std::vector<int> pi(matrix->getDimension());
    std::vector<double> lambda(matrix->getDimension());
    
    int len1 = slink1->_pi.size();
    int len2 = slink2->_pi.size();

    // ciclo i nodi della prima meta'
    for (int i = 0; i < len1; ++i) {
        
        // assumo che i valori attuali siano ottimali
        pi.at(i) = slink1->_pi.at(i);
        lambda.at(i) = slink1->_lambda.at(i);

        // calcolo la distanza con ciascun nodo della seconda meta'
        // ovvero merge!
        for (int j=0; j < len2; ++j) {
            double dist = matrix->valueAt(i,j+mid_point);

            // se ho trovato una distanza minore
            // aggiorno i valori di pi e lambda 
            if (lambda.at(i) > dist) {
                pi.at(i) = slink2->_pi.at(j); // il nuovo nodo finale del cluster
                lambda.at(i) = dist;    // la nuova distanza
            }
        }
    }

    // i nodi nella seconda meta' sono gia' sistemati
    // li unisco al cluser risultante
    for (int j = 0; j < len2; ++j) {
        pi.at(j + mid_point) = slink2->_pi.at(j);
        lambda.at(j + mid_point) = slink2->_lambda.at(j);
    }

    Slink result(pi, lambda);
    return result;
}


Slink Slink::execute_split(Matrix *matrix, int num_threads)
{
    int dim = matrix->getDimension();

    int mid_point = dim;

    Slink slink1 = execute_split_aux(matrix, 0, mid_point);
    Slink slink2 = execute_split_aux(matrix, mid_point, dim);
    Slink result = execute_split_merge(matrix, &slink1, &slink2, mid_point);

    return result;
}