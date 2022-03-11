#pragma once

#include <iostream>
#include <vector>
#include <cmath>

#include "matrix.h"
#include "util.h"

class Slink {
public:
    static Slink execute(Matrix *matrix);

    std::vector<double> getPi() { return pi; }
    std::vector<double> getLambda() { return lambda; }

private:
    std::vector<double> pi;
    std::vector<double> lambda;

};

Slink Slink::execute(Matrix *matrix)
{    
    Slink slink;
    std::vector<double> M;

    for (int n=0; n < matrix->getDimension(); ++n) {
        
        // INIT
        slink.pi.push_back(n);
        slink.lambda.push_back(INF);
        M.clear();
        M.reserve(n);
        for (int i=0; i<n; ++i) {
            M.push_back(matrix->valueAt(i, n));
        }
        
        // UPDATE
        for (int i=0; i<n; ++i) {
            if (slink.lambda[i] >= M[i]) {
                M[slink.pi[i]] = std::min(M[slink.pi[i]], slink.lambda[i]); 
                slink.lambda[i] = M[i];
                slink.pi[i] = n;
            }
            
            if (slink.lambda[i] < M[i]) {
                M[slink.pi[i]] = std::min(M[slink.pi[i]], M[i]);
            }
        }

        // FINALIZE
        for (int i=0; i<n; ++i) {
            if (slink.lambda[i] >= slink.lambda[slink.pi[i]]) {
                slink.pi[i] = n;
            }
        }
    }

    return slink;
}

