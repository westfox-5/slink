#include "slink.h"

bool _is_in_cluster(std::vector<int> *pi, int cluster, int index)
{
    int c = index;
    while (c < pi->size() && c < cluster)
        c = pi->at(c);
    return c == cluster;
}

const Slink *Slink::execute(const Matrix *matrix, int num_threads, Slink::ExecType et)
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

const std::string Slink::executionTypeToString(Slink::ExecType et)
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

double Slink::checkValue() const
{
    int piSum = std::accumulate(pi_.begin(), pi_.end(), 0, [](int a, int b){ return a + b; });
    double lambdaSum = std::accumulate(lambda_.begin(), lambda_.end(), 0.0, [](double a, double b)
        {
            if (a != INF && b != INF)
                return a + b;
            if (a == INF)
                return b;
            if (b == INF)
                return a;
            return 0.0;
        }
    );
    return piSum + lambdaSum;
}

void Slink::print() const
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

int Slink::size() const
{
    return pi_.size();
}

inline Slink *Slink::empty()
{
    return new Slink({},{});
}

const Slink *Slink::__sequential(const Matrix *matrix)
{
    std::vector<int> pi;
    std::vector<double> M;
    std::vector<double> lambda;

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

Slink *__split_merge(const Matrix *matrix, const Slink *slink1, const Slink *slink2)
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
                    if (_is_in_cluster(&pi, pi[n], j) && !_is_in_cluster(&pi, n, j))
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

const Slink *Slink::__sequential__split(const Matrix *matrix, int num_blocks)
{
    int dimension = matrix->getDimension();
    std::vector<Slink *> partial_results(num_blocks);

    // Load Balancing
    const int block_size = (int)(std::floor(dimension / (num_blocks)));
    int num_remaining_blocks = dimension % num_blocks;
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

        Slink *slink = Slink::empty();
        slink->start_ = start;
        slink->end_ = end - 1;
        slink->id_ = block_id;

        __split_aux(matrix, start, end, slink);

        partial_results[block_id] = slink;
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
}


