#ifndef SLINK_EXECUTORS_H
#define SLINK_EXECUTORS_H

#include "slink.h"
#include "matrix.h"

namespace SlinkExecutors {
    enum type {
        SEQUENTIAL,
        PARALLEL_OMP,
        PARALLEL_SPLIT,
        PARALLEL_INNER_SPLIT,
        PARALLEL_SPLIT_OMP
    };

    std::string execution_type_to_string(SlinkExecutors::type type);
    bool is_in_cluster(std::vector<int> *pi, int cluster, int index);
    const Slink* execute(const Matrix *matrix, int num_threads, type type);
    void split_aux(const Matrix *matrix, int start, int end, Slink *out_slink);
    Slink* split_merge(const Matrix *matrix, const Slink *slink1, const Slink *slink2);

    namespace Sequential {
        const Slink* execute(const Matrix *matrix);
    }

    namespace ParallelOMP {
        const Slink* execute(const Matrix *matrix, int num_threads);
    }

    namespace ParallelSplit {
        const Slink* execute(const Matrix *matrix, int num_threads);
    }

    namespace ParallelInnerSplit {
        const Slink* execute(const Matrix *matrix, int num_threads);
    }

    namespace ParallelSplitOMP {
        const Slink* execute(const Matrix *matrix, int num_threads);
    }
}

#endif