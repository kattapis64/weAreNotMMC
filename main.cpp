#include <iomanip>
#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include "ovh.h"
#ifdef _OPENMP
#include <omp.h>
#endif

std::string names[ovh::N_AREA] = {"Rice", "Tree", "Crop",
"Pond", "House"};
std::string monthNames[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
std::string names2[] = {"Rice", "Tree", "No crop Harvest first two-month","No crop Harvest second two-month","pond depth"};

int main() {
  // * Iterate หา ค่ากำไรที่มากท่ี่สุด
  double bestCost = 0;
  ovh::SolveResult bestResult;
  int bestVals[5];

 
  const long long totalCount = 12*12*12*12*5;
  std::atomic<long long> completedCount{0};

  // Background reporter thread: wakes up once a second, prints how far the
  // atomic counter has gotten, and exits once the main work is done. This
  // keeps the hot loop itself completely free of any print statements -
  // no interleaved output, no I/O overhead inside the parallel region.
  std::atomic<bool> done{false};
  std::thread reporter([&]() {
    while (!done.load()) {
      long long c = completedCount.load();
      double pct = 100.0 * static_cast<double>(c) / static_cast<double>(totalCount);
      std::cout << "\rProgress: " << std::fixed << std::setprecision(1)
                << pct << "% (" << c << "/" << totalCount << ")   " << std::flush;
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  });

  // The (d, c) pair fully determines whether an (a, b, h) combo passes the
  // "(c - d) % 2 == 0 && c < d" filter below, and nothing in the loop body
  // depends on execution order, so it's safe to parallelize the outer two
  // loops. Each thread runs its own independent slice of (d, c) values and
  // does its own full (b, a, h) sweep, calling sweepBestScale/solveLp, each
  // of which creates its own local HiGHS instance in CSC.cpp - so there is
  // no shared solver state between threads.
  //
  // Only the shared bestCost/bestResult/bestVals update at the end needs
  // protecting (via #pragma omp critical), since multiple threads could
  // otherwise race on reading and writing them at the same time.

  // * หาเดือนที่ให้ผลผลิตดีที่สุด และ ความลึกของบ่อ
  #pragma omp parallel for collapse(2) schedule(dynamic)
  for (int d = 0; d < 12; d++) {
    for (int c = 0; c < 12; c++) {
      for (int b = 0; b < 12; b++) {
        for (int a = 0; a < 12; a++) {
          for (float h = 1; h <= 3; h += 0.5) {
            completedCount.fetch_add(1);
            if ((c - d) % 2 == 0 && c < d) {
              ovh::optimizeVariable ov{
                  .riceMonth = a,
                  .treeMonth = b,
                  .noCropMonth_0 = c,
                  .noCropMonth_1 = d,
                  .depth = h,

              };

              // * ใช้ soft constraint เป็นอัตราส่วนเกษตรทฤษฎ๊ใหม่
              ovh::SolveResult best = ovh::sweepBestScale(
                  ov, /*baseRice=*/300.0, /*baseTree=*/300.0,
                  /*baseHouse=*/100.0, 5, 10);

              // Multiple threads may reach here with a candidate "best" at
              // the same time; #pragma omp critical makes the
              // read-compare-write of bestCost/bestResult/bestVals atomic
              // as a whole, which a plain if-statement would not guarantee
              // under concurrent access.
              #pragma omp critical
              {
                if (best.profit > bestCost) {
                  bestCost = best.profit;
                  bestResult = best;
                  bestVals[0] = a;
                  bestVals[1] = b;
                  bestVals[2] = c;
                  bestVals[3] = d;
                  bestVals[4] = h;
                }
              }
            }
          }
        }
      }
    }
  }

  done = true;
  reporter.join();
  std::cout << std::endl; // move past the progress line

  std::cout << "------------------- Absolute Best --------------------" << std::endl;
  std::cout << "Profit: " << bestResult.profit << " baht\n";
  std::cout << "Land allocation (m^2):\n";
  for (int a = 0; a < ovh::N_AREA; a++)
    std::cout << "  " << std::setw(6) << std::left << names[a]
              << std::right << std::setw(10) << bestResult.alloc[a]
              << "\n";
  std::cout << std::setprecision(2);
  std::cout << "Min pond storage: " << bestResult.min_storage << " m^3\n";
  std::cout << "Best Months To start farming \n";
  for (int i = 0; i < 4; i++) {
    std::cout << names2[i] << " : " << monthNames[bestVals[i]] << "\n";
  }
  std::cout << names2[4] << " : " << bestVals[4] << "\n";
}