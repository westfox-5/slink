#include "../slink_executors.h"

const Slink* SlinkExecutors::Sequential::execute(const Matrix *matrix)
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