#include "../slink_executors.h"

const Slink* SlinkExecutors::ParallelOMP::execute(const Matrix *matrix, int num_threads)
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