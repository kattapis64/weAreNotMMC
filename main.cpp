#include <algorithm>
#include <cmath>
#include <highs/Highs.h>
#include <iostream>
#include <vector>
using namespace std;
struct optimizeVariable {
  int riceMonth = 0;
  int treeMonth = 0;
  int noCropMonth_0 = 0;
  int noCropMonth_1 = 0;
  vector<vector<int>> costMatrix = vector<vector<int>>{5, vector<int>(240, 0)};
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
int main() {
  optimizeVariable ovv{
      .costMatrix = vector<vector<int>>{5, vector<int>(240, 1)},
      .areaSize = {1, 2, 3, 4, 5},
  };

  cout << compoundedMoneyProfit(2, &ovv);
  cout << netWaterProfit(2, &ovv);
  for (int a = 0; a < 12; a++) {
    for (int i)
  }
  return 0;
}
