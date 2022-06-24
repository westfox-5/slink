#include "../slink_executors.h"

const Slink* SlinkExecutors::ParallelSplitOMP::execute(const Matrix *matrix, int num_threads)
{
    int dimension = matrix->getDimension();
    std::vector<Slink *> partial_results;
    static const int blocks_per_thread = 10;

    #pragma omp parallel for schedule(static, 1)
    for (int i=0; i<dimension; i+=blocks_per_thread)
    {
        int start = i;
        int end = std::min(i + blocks_per_thread, dimension);
        Slink *slink = Slink::empty();
        slink->start_ = i;
        slink->end_ = end;
        slink->id_ = i;
        SlinkExecutors::split_aux(matrix, start, end, slink);
        partial_results.push_back(slink);
    }

    if (partial_results.empty())
    {
        return Slink::empty();
    }

    Slink *result = partial_results.at(0);
    for (int i=1; i<partial_results.size(); ++i)
    {
        result = SlinkExecutors::split_merge(matrix, result, partial_results.at(i));
    }

    return result;
}