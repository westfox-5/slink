#include "../slink_executors.h"

void inner_cycle(std::vector<double> *M, std::vector<int> *pi, std::vector<double> *lambda, int start, int end)
{
    for (int i = start; i < end; ++i)
    {
        if (lambda->at(i) >= M->at(i))
        {
            M->at(pi->at(i)) = std::min(M->at(pi->at(i)), lambda->at(i));
            lambda->at(i) = M->at(i);
            pi->at(i) = end;
        }
        else
        {
            M->at(pi->at(i)) = std::min(M->at(pi->at(i)), M->at(i));
        }
    }
}

const Slink* SlinkExecutors::ParallelInnerSplit::execute(const Matrix *matrix, int num_threads)
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
        int len = 3;
        for (int x = 0; x < n; x += len) {
            int start = x;
            int end = std::min(start + len, n);
            std::cout << "n: " << n << ", x: " << x << ", start: " << start << ", end: " << end << std::endl;
            std::vector<int> _pi(&pi[start], &pi[end]);
            std::vector<double> _M(&M[start], &M[end]);
            std::vector<double> _lambda(&lambda[start], &lambda[end]);
            inner_cycle(&_M, &_pi, &_lambda, 0, std::min(len, n - start));
            std::cout << "###########################################" << std::endl;
            std::cout << "M" << std::endl;
            printVectorHoriz(_M);
            std::cout << "pi" << std::endl;
            printVectorHoriz(_pi);
            std::cout << "lambda" << std::endl;
            printVectorHoriz(_lambda);
            std::cout << "###########################################" << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
        std::cout << "Real M, P, lambda" << std::endl;
        inner_cycle(&M, &pi, &lambda, 0, n);
        std::cout << "M" << std::endl;
        printVectorHoriz(M);
        std::cout << "pi" << std::endl;
        printVectorHoriz(pi);
        std::cout << "lambda" << std::endl;
        printVectorHoriz(lambda);
        std::cout << "---------------------------" << std::endl;

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