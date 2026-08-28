#ifndef OVH_H
#define OVH_H

#include <vector>

namespace ovh {

using std::vector;

// ---- model configuration (mirrors the Python prototype) ----
constexpr int YEARS = 15;
constexpr int MONTHS = YEARS * 12;
constexpr double FARM_AREA = 6400.0;      // m^2
constexpr double POND_DEPTH_M = 3.0;      // m
constexpr double INIT_FRAC = 0.5;         // pond starts half-full
constexpr double L_PER_M3 = 1000.0;
constexpr double V_THRESHOLD = 50.0;      // m^3, hard minimum pond storage
constexpr double PENALTY = 1e6;           // baht per m^2 shortfall vs. a floor
constexpr double BUDGET_LIMIT = 200000.0; // baht, initial investment cap

constexpr int N_AREA = 5; // 0 rice, 1 tree, 2 crop, 3 pond, 4 house

// Planting-schedule / layout parameters. These are the knobs a caller can
// vary; everything else above is a fixed model constant.
struct optimizeVariable {
  int riceMonth = 1;      // month (0-11) rice planting starts each year
  int treeMonth = 7;      // month (0-11) tree planting starts each year
  int noCropMonth_0 = 2;  // crop field left fallow before this month...
  int noCropMonth_1 = 4;  // ...and after this month, each year
  float depth = 3;
};

struct SolveResult {
  bool feasible = false;
  bool fully_met = false; // floors satisfied with zero shortfall
  double scale = 0.0;     // sweep scale that produced this result
  double profit = 0.0;
  vector<double> alloc = vector<double>(N_AREA, 0.0); // m^2 per area
  double slack_rice = 0.0, slack_tree = 0.0, slack_house = 0.0;
  double min_storage = 0.0; // m^3
};

// Monthly per-m^2 cash-flow matrix: rows = area, cols = month [0, MONTHS).
// Negative = outlay that month, positive = revenue that month.
vector<vector<double>> costMatrix(const optimizeVariable &ov);

// Solves the land-allocation LP for one set of soft floors (minimum m^2
// you want for rice/tree/house; shortfalls are allowed but penalized).
SolveResult solveLp(const optimizeVariable &ov, double floorRice,
                     double floorTree, double floorHouse);

// Scales a base rice:tree:house floor ratio from 0 up to maxScale and
// returns the highest-scale, fully-satisfied (zero shortfall) solution
// with the best profit.
SolveResult sweepBestScale(const optimizeVariable &ov, double baseRice,
                            double baseTree, double baseHouse,
                            double maxScale = 5.0, int steps = 51);

} // namespace ovh

#endif // OVH_H