#include <algorithm>
#include <cmath>
#include <highs/Highs.h>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>
using namespace std;
struct optimizeVariable {
  int riceMonth = 0;
  int treeMonth = 0;
  int noCropMonth_0 = 0;
  int noCropMonth_1 = 0;
  vector<vector<int>> costMatrix = vector<vector<int>>{5, vector<int>(241, 0)};
  int pondDepth = 0;
  vector<int> areaSize = vector<int>(5);
};
vector<string> areaName = {"Rice Paddy", "Tree", "Crops", "Pond", "House"};
vector<int> waterUsage = {180, 90, 120, 0, 20};
const vector<vector<int>> rainAndEvap = {
    {20, 10, 120, 130},  {30, 15, 140, 150},  {50, 20, 160, 180},
    {80, 30, 170, 190},  {150, 40, 150, 180}, {180, 50, 140, 170},
    {200, 60, 140, 170}, {250, 50, 130, 180}, {300, 80, 120, 160},
    {200, 40, 130, 150}, {80, 50, 130, 140},  {20, 10, 120, 130}};

int rainWithENSO(int n) { return n; }
int evapWithENSO(int n) { return n; }
vector<int> memoiMoneyProfit(240, 0);
int waterProfit(int month, int areaNumber, optimizeVariable *ov) {
  if ((ov->costMatrix).at(areaNumber).at(month) != 0) {

    return ov->areaSize.at(areaNumber) *
               (rainWithENSO(rainAndEvap.at(month).at(0)) -
                evapWithENSO(rainAndEvap.at(month).at(3))) -
           waterUsage.at(month);

  } else {
    return 0;
  }
}

float netWaterProfit(int month, optimizeVariable *ov) {
  float sum = 0;
  for (int i = 0; i < 5; i++) {
    sum += waterProfit(month, i, ov) + ov->areaSize.at(3) * ov->pondDepth;
  }
  return sum;
}
int moneyProfit(int month, optimizeVariable *ov) {
  int sum = 0;
  if (memoiMoneyProfit[month] == 200000) {
    for (int i = 0; i < 5; i++) {
      sum += ov->costMatrix.at(i).at(month) * ov->areaSize.at(i);
      memoiMoneyProfit.at(month) += sum;
    }

  } else if (memoiMoneyProfit.at(month) == 0) {
    for (int i = 0; i < 5; i++) {
      sum += ov->costMatrix.at(i).at(month) * ov->areaSize.at(i);
      memoiMoneyProfit.at(month) = sum;
    }
  } else {
    sum = memoiMoneyProfit.at(month);
  }
  cout << month << " " << sum << "\n";
  return sum;
}
int compoundedMoneyProfit(int month, optimizeVariable *ov) {
  int sum = 200000;
  for (int i = 1; i < month; i++) {
    for (int j = 0; j < 5; j++) {
      sum += ov->areaSize[i] * ov->costMatrix.at(j).at(i);
    }
  }
  return sum;
}
void populateCostMatrix(optimizeVariable *ov) {
  ov->costMatrix.at(3).at(0) = -30;
  ov->costMatrix.at(3).at(12) = -20;

  for (int y = 0; y < 19; y++) {
    ov->costMatrix.at(0).at(ov->riceMonth + (y) * 12) = -15;
    for (int m = ov->riceMonth + (y) * 12 + 1; m < ov->riceMonth + (y) * 12 + 4;
         m++) {
      ov->costMatrix.at(0).at(m) = 1;
    }
    ov->costMatrix.at(0).at(ov->riceMonth + 4 + (y) * 12) = -45;

    ov->costMatrix.at(1).at(ov->treeMonth + (y) * 12) = -40;

    if (y < 19) {
      for (int m = ov->treeMonth + (y) * 12 + 1;
           m < ov->treeMonth + (y) * 12 + 12; m++) {
        ov->costMatrix.at(1).at(m) = 1;
      }
    }
    ov->costMatrix.at(1).at(ov->treeMonth + 12 + (y) * 12) = 65;

    for (int m = 0; m < 12; m++) {
      if (m < ov->noCropMonth_0 || m > 1 + ov->noCropMonth_1) {
        if (m % 2 == (ov->noCropMonth_0 - 2) % 2) {
          ov->costMatrix.at(2).at(m + 12 * y) = -25;
        } else {

          ov->costMatrix.at(2).at(m + 12 * y) = 80;
        }
      }
      if (m != 0) {
        ov->costMatrix.at(4).at(m) = 1;
      }
    }

    if (y >= 2) {
      ov->costMatrix.at(3).at(12 * y) = -30;
    }
  }
  ov->costMatrix.at(4).at(0) = 500;
}
int main() {
  for (int a = 0; a < 12; a++) {
    for (int b = 0; b < 12; b++) {
      for (int c = 0; c < 12; c++) {
        for (int d = 0; d < 12; d++) {
          if ((c - d) % 2 == 0 && c < d) {
            optimizeVariable ovv{
                .riceMonth = a,
                .treeMonth = b,
                .noCropMonth_0 = c,
                .noCropMonth_1 = d,
                .areaSize = {1, 1, 1, 1, 1},
            };
            populateCostMatrix(&ovv);

            for (int i = 0; i < 5; i++) {
              for (int j = 0; j < 24; j++) {
                cout << ovv.costMatrix[i][j] << setw(6);
              }
              cout << "\n";
            }
          }
        }
      }
    }
  }
  // optimizeVariable ovv{
  //     .riceMonth = 1,
  //     .treeMonth = 1,
  //     .noCropMonth_0 = 2,
  //     .noCropMonth_1 = 4,
  //     .areaSize = {1, 1, 1, 1, 1},
  // };
  // populateCostMatrix(&ovv);
  //
  // for (int i = 0; i < 5; i++) {
  //   for (int j = 0; j < 12; j++) {
  //     cout << ovv.costMatrix[i][j] << setw(6);
  //   }
  //   cout << "\n";
  // }

  return 0;
}
