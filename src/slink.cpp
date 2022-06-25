#include "slink.h"

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

Slink *Slink::empty()
{
    return new Slink({},{});
}


