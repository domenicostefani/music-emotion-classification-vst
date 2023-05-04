# pragma once

#include <vector>
#include <string>

#include <essentia/algorithmfactory.h>

void print2DVectorHead(const std::vector<std::vector<essentia::Real>> &vec, int n = 5, int m = 5)
{
    if (vec.size() < n)
        throw essentia::EssentiaException("Vector size is smaller than n ("+std::to_string(vec.size())+" < " + std::to_string(n) + ")");
    if (vec[0].size() < m)
        throw essentia::EssentiaException("Inner vector size is smaller than n ("+std::to_string(vec[0].size())+" < " + std::to_string(m) + ")");
    for (int i = 0; i < n-1; i++) {
        printf("[ ");
        for (int j = 0; j < m-1; j++)
            printf("%.8f ", vec[i][j]);
        printf("... %.8f]", vec[i][vec[i].size()-1]);
        printf("\n");
    }
    for (int j = 0; j < m-1; j++)
        printf("     ...        ");
    printf("\n[ ");
    int i = vec.size()-1;
    for (int j = 0; j < m-1; j++)
        printf("%.8f ", vec[i][j]);
    printf("... %.8f]\n", vec[i][vec[i].size()-1]);
}
