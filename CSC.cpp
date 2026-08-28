#include <Highs.h>
#include <vector>
#include <stdexcept>
using namespace std;

// Solves: optimize objective^T x
//   subject to  rowLower[i] <= (A x)[i] <= rowUpper[i]  for each row
//               colLower[j] <= x[j] <= colUpper[j]
// A is a plain dense matrix: A[row][col]. Returns the raw solution vector.
std::vector<double> solveLP(
    const std::vector<double>& objective,
    const std::vector<std::vector<double>>& A,
    const std::vector<double>& rowLower,
    const std::vector<double>& rowUpper,
    const std::vector<double>& colLower,
    const std::vector<double>& colUpper,
    bool maximize = true)
{
  const int numCols = static_cast<int>(objective.size());
  const int numRows = static_cast<int>(A.size());

  HighsModel model;
  model.lp_.num_col_ = numCols;
  model.lp_.num_row_ = numRows;
  model.lp_.sense_ = maximize ? ObjSense::kMaximize : ObjSense::kMinimize;
  model.lp_.col_cost_ = objective;
  model.lp_.col_lower_ = colLower;
  model.lp_.col_upper_ = colUpper;
  model.lp_.row_lower_ = rowLower;
  model.lp_.row_upper_ = rowUpper;

  // Convert dense A -> HiGHS row-wise sparse format.
  // Same logic as before, just pre-reserving capacity so the push_back loop
  // below doesn't repeatedly reallocate/copy as it grows (this matrix is
  // rebuilt from scratch on every call, so that reallocation cost adds up
  // across thousands of solves).
  std::vector<int> start(1, 0);
  std::vector<int> index;
  std::vector<double> value;
  index.reserve(static_cast<size_t>(numRows) * 4); // most rows are sparse
  value.reserve(static_cast<size_t>(numRows) * 4);
  for (int i = 0; i < numRows; ++i) {
    for (int j = 0; j < numCols; ++j) {
      if (A[i][j] != 0.0) {
        index.push_back(j);
        value.push_back(A[i][j]);
      }
    }
    start.push_back(static_cast<int>(index.size()));
  }
  model.lp_.a_matrix_.format_ = MatrixFormat::kRowwise;
  model.lp_.a_matrix_.start_ = start;
  model.lp_.a_matrix_.index_ = index;
  model.lp_.a_matrix_.value_ = value;

  Highs highs;
  highs.setOptionValue("output_flag", false);
  // These options matter a lot when solving many small/medium LPs back to
  // back: they cut per-call setup overhead without changing what problem
  // is solved or the readability of how you build A/bounds/objective.
  highs.setOptionValue("presolve", "on");     // usually a net win, but cheap to try "off" too if a given model is small
  highs.setOptionValue("solver", "simplex");  // avoids IPM setup/cleanup cost, which tends to be pure overhead on small LPs solved repeatedly
  highs.setOptionValue("parallel", "off");    // let YOUR outer loop parallelize (see main.cpp); HiGHS's own internal threading just adds overhead when solving many small problems concurrently
  highs.setOptionValue("threads", 1);
  if (highs.passModel(model) != HighsStatus::kOk)
    throw std::runtime_error("HiGHS: failed to pass model");
  if (highs.run() != HighsStatus::kOk)
    throw std::runtime_error("HiGHS: solve failed");
  if (highs.getModelStatus() != HighsModelStatus::kOptimal)
    throw std::runtime_error("HiGHS: infeasible or unbounded");

  return highs.getSolution().col_value;
}

// Convenience wrapper: solves, then normalizes the first `numLandVars`
// solution entries into ratios summing to 1 (e.g. land-use proportions).
std::vector<double> solveLandRatios(
    const std::vector<double>& objective,
    const std::vector<std::vector<double>>& A,
    const std::vector<double>& rowLower,
    const std::vector<double>& rowUpper,
    const std::vector<double>& colLower,
    const std::vector<double>& colUpper,
    int numLandVars,
    bool maximize = true)
{
  auto x = solveLP(objective, A, rowLower, rowUpper, colLower, colUpper, maximize);

  double total = 0.0;
  for (int i = 0; i < numLandVars; ++i) total += x[i];

  std::vector<double> ratios(numLandVars, 0.0);
  if (total > 1e-9)
    for (int i = 0; i < numLandVars; ++i) ratios[i] = x[i] / total;
  return ratios;
}