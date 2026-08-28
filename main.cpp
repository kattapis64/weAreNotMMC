#include <iomanip>
#include <iostream>

#include "ovh.h"

int main() {
  ovh::optimizeVariable ov{
    .riceMonth=3,
  }; // uses default planting schedule

  // Same base ratio as the Python sweep: "New Theory 30:30:10 style"
  
  ovh::SolveResult best = ovh::sweepBestScale(ov, /*baseRice=*/300.0,
                                               /*baseTree=*/300.0,
                                               /*baseHouse=*/100.0);

  if (!best.feasible) {
    std::cout << "No scale fully satisfied the floors within budget.\n";
    return 0;
  }

  static const char *names[ovh::N_AREA] = {"Rice", "Tree", "Crop", "Pond",
                                            "House"};

  std::cout << std::fixed << std::setprecision(0);
  std::cout << "Best fully-met scale: " << best.scale << "x\n";
  std::cout << "Profit: " << best.profit << " baht\n";
  std::cout << "Land allocation (m^2):\n";
  for (int a = 0; a < ovh::N_AREA; a++)
    std::cout << "  " << std::setw(6) << std::left << names[a]
               << std::right << std::setw(10) << best.alloc[a] << "\n";
  std::cout << std::setprecision(2);
  std::cout << "Min pond storage: " << best.min_storage << " m^3\n";
}