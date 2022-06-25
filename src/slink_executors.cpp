#include "slink_executors.h"

const Slink* SlinkExecutors::execute(const Matrix *matrix, int num_threads, SlinkExecutors::type type) {
    switch (type)
    {
    case SEQUENTIAL:
        return SlinkExecutors::Sequential::execute(matrix);
    case PARALLEL_OMP:
        return SlinkExecutors::ParallelOMP::execute(matrix, num_threads);
    case PARALLEL_SPLIT:
        return SlinkExecutors::ParallelSplit::execute(matrix, num_threads);
    case PARALLEL_SPLIT_OMP:
        return SlinkExecutors::ParallelSplitOMP::execute(matrix, num_threads);
    default:
        std::throw_with_nested(std::invalid_argument("Execution Type not managed."));
    }
}

std::string SlinkExecutors::execution_type_to_string(SlinkExecutors::type type)
{
    switch (type)
    {
    case SEQUENTIAL:
        return "Sequential";
    case PARALLEL_OMP:
        return "Parallel (OMP)";
    case PARALLEL_SPLIT:
        return "Parallel (SPLIT)";
    case PARALLEL_SPLIT_OMP:
            return "Parallel (SPLIT-OMP)";
    default:
        std::throw_with_nested(std::invalid_argument("Execution Type not managed."));
    }
}

bool SlinkExecutors::is_in_cluster(std::vector<int> *pi, int cluster, int index)
{
    int c = index;
    while (c < pi->size() && c < cluster)
        c = pi->at(c);
    return c == cluster;
}

void SlinkExecutors::split_aux(const Matrix *matrix, int start, int end, Slink *out_slink)
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
            M.push_back(matrix->valueAt(i, n));
        }

        // UPDATE
        for (int i = start; i < n; ++i)
        {
            int pi__value = pi[i - start];
            double min_1 = std::min(M[pi__value - start], lambda[i - start]);
            double min_2 = std::min(M[pi__value - start], M[i - start]);

            if (lambda[i - start] >= M[i - start])
            {
                M[pi__value - start] = min_1;
                lambda[i - start] = M[i - start];
                pi[i - start] = n;
            }
            else
            {
                M[pi__value - start] = min_2;
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
}

Slink* SlinkExecutors::split_merge(const Matrix *matrix, const Slink *slink1, const Slink *slink2)
{
    int len1 = slink1->size();
    int len2 = slink2->size();
    int size = len1 + len2;

    std::vector<int> pi(slink1->pi_);
    std::vector<double> lambda(slink1->lambda_);
    pi.insert(pi.end(), slink2->pi_.begin(), slink2->pi_.end());
    lambda.insert(lambda.end(), slink2->lambda_.begin(), slink2->lambda_.end());

    for (int i = len1 - 1; i >= 0; i--)
    {
        int cluster = slink1->pi_[i];
        double min_dist = slink1->lambda_[i];

        for (int j = 0; j < len2; j++)
        {
            double d = matrix->valueAt(i, len1 + j);
            if (d < min_dist)
            {
                min_dist = d;
                cluster = len1 + j;
            }
        }

        while (min_dist > lambda[cluster])
            cluster = pi[cluster];

        pi[i] = cluster;
        lambda[i] = min_dist;
    }

    // finalizzazione
    for (int n = 0; n < size; n++)
    {
        for (int i = 0; i < n; ++i)
        {
            // aggiorno la distanza del nodo n calcolando la distanza minima
            // tra tutti i nodi che sono nel cluster n
            // e tutti i nodi che sono nel cluster pi[n] (ovvero quello a cui n appartiene)
            if (pi[i] == n)
            {
                for (int j = 0; j < i; ++j)
                {
                    if (SlinkExecutors::is_in_cluster(&pi, pi[n], j) && !SlinkExecutors::is_in_cluster(&pi, n, j))
                    {
                        lambda[n] = std::min(lambda[n], matrix->valueAt(j, i));
                    }
                }
            }
            if (lambda[i] >= lambda[pi[i]])
            {
                pi[i] = n;
            }
        }
    }

    Slink *result = new Slink(pi, lambda);
    result->start_ = std::min(slink1->start_, slink2->start_);
    result->end_ = std::max(slink1->end_, slink2->end_);
    result->id_ = std::min(slink1->id_, slink2->id_);

    return result;
}
