#pragma once

#include<iostream>
#include<vector>
#include<cmath>

#include "col_major.h"
#include "util.h"

class Slink {
public:

private:
    std::vector<double> pi;
    std::vector<double> lambda;

public:
    void execute(ColMajorMatrix *matrix);

    std::vector<double> getPi() {
        return pi;
    }

    std::vector<double> getLambda() {
        return lambda;
    }
};

void Slink::execute(ColMajorMatrix *matrix)
{    
    std::vector<double> M;

    for (int n=0; n < matrix->N; ++n) {
        
        // INIT
        pi.push_back(n);
        lambda.push_back(INF);
        M.clear();
        M.reserve(n);
        for (int i=0; i<n; ++i) {
            M.push_back(matrix->valueAt(i, n));
        }
        
        // UPDATE
        for (int i=0; i<n; ++i) {
            if (lambda[i] >= M[i]) {
                M[pi[i]] = std::min(M[pi[i]], lambda[i]); 
                lambda[i] = M[i];
                pi[i] = n;
            }
            
            if (lambda[i] < M[i]) {
                M[pi[i]] = std::min(M[pi[i]], M[i]);
            }
        }

        // FINALIZE
        for (int i=0; i<n; ++i) {
            if (lambda[i] >= lambda[pi[i]]) {
                pi[i] = n;
            }
        }
    }
}

