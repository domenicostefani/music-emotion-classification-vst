

#include <iostream>

#include "extractionpipeline.h"

using namespace std;
using namespace essentia;

emosmi::MusicnnFeatureExtractor extractionPipeline;

void usage(char *progname)
{
    cout << "Error: wrong number of arguments" << endl;
    cout << "Usage: " << progname << " input_audiofile" << endl;

    exit(1);
}



void print2DVectorHead(const vector<vector<Real>> &vec, int n = 5, int m = 5)
{
    if (vec.size() < n)
        throw EssentiaException("Vector size is smaller than n ("+to_string(vec.size())+" < " + to_string(n) + ")");
    if (vec[0].size() < m)
        throw EssentiaException("Inner vector size is smaller than n ("+to_string(vec[0].size())+" < " + to_string(m) + ")");
    for (int i = 0; i < n-1; i++) {
        printf("[ ");
        for (int j = 0; j < m-1; j++)
            printf("%.8f ", vec[i][j]);
        printf("... %.8f]", vec[i][vec[i].size()-1]);

        cout << endl;
    }
    for (int j = 0; j < m-1; j++)
        printf("     ...        ");
    printf("\n[ ");
    int i = vec.size()-1;
    for (int j = 0; j < m-1; j++)
        printf("%.8f ", vec[i][j]);
    printf("... %.8f]\n", vec[i][vec[i].size()-1]);
}


int main(int argc, char *argv[])
{

    string audioFilename, outputFilename, profileFilename;

    switch (argc)
    {
    case 2:
        audioFilename = argv[1];
        break;
    default:
        usage(argv[0]);
    }

    cout << "Analyzing " << audioFilename << endl;
    std::vector<std::vector<Real>> bandsVec;

    extractionPipeline.extractFromFile(audioFilename, bandsVec);

    cout << "size(output): " << bandsVec.size() << "x" << bandsVec[0].size() << endl;
    cout << "head(output): " << endl;
    print2DVectorHead(bandsVec);

    return 0;
}