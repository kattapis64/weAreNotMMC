#ifndef CSC
#define CSC

#include <vector>
using namespace std;

std::vector<double> solveLP(
    const std::vector<double>& objective,
    const std::vector<std::vector<double>>& A,
    const std::vector<double>& rowLower,
    const std::vector<double>& rowUpper,
    const std::vector<double>& colLower,
    const std::vector<double>& colUpper,
    bool maximize = true);
std::vector<double> solveLandRatios(
    const std::vector<double>& objective,
    const std::vector<std::vector<double>>& A,
    const std::vector<double>& rowLower,
    const std::vector<double>& rowUpper,
    const std::vector<double>& colLower,
    const std::vector<double>& colUpper,
    int numLandVars,
    bool maximize = true);
#endif