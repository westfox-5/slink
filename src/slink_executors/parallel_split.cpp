#include "../slink_executors.h"

const Slink* SlinkExecutors::ParallelSplit::execute(const Matrix *matrix, int num_threads)
{
    int total_jobs_assigned = 0;
    int dimension = matrix->getDimension();
    std::vector<std::thread> threads(num_threads);
    std::vector<Slink *> partial_results(num_threads);
    int num_remaining_blocks = dimension % num_threads;
    const int block_size = (int)(std::floor(dimension / (num_threads)));

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
        threads[block_id] = std::thread(SlinkExecutors::split_aux, matrix, start, end, slink);
    }

    for (int i = 0; i < threads.size(); ++i)
    {
        threads[i].join();
    }

    if (num_threads == 0)
    {
        return Slink::empty();
    }

    Slink *result = partial_results.at(0);
    for (int i = 1; i < num_threads; ++i)
    {
        result = SlinkExecutors::split_merge(matrix, result, partial_results.at(i));
    }

    return result;
}