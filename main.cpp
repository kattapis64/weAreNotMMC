#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
using namespace std;
vector<int> areaSize = {0, 0, 0, 0, 0};
vector<string> areaName = {"Rice Paddy", "Tree", "Crops", "Pond", "House"};
vector<int> waterUsage = {180, 90, 120, 0, 20};
vector<vector<int>> costMatrix(5, vector<int>(240, 0));
const int rainAndEvap[12][4] = {
    {20, 10, 120, 130},  {30, 15, 140, 150},  {50, 20, 160, 180},
    {80, 30, 170, 190},  {150, 40, 150, 180}, {180, 50, 140, 170},
    {200, 60, 140, 170}, {250, 50, 130, 180}, {300, 80, 120, 160},
    {200, 40, 130, 150}, {80, 50, 130, 140},  {20, 10, 120, 130}};
int riceMonth = 0;
int treeMonth = 0;
int noCropMonth_0 = 0;
int noCropMonth_1 = 0;
int pondDepth = 0;
int rainWithENSO(int n) { return n; }
int evapWithENSO(int n) { return n; }
int memoiMoneyProfit[240] = {0};
int waterProfit(int month, int areaNumber) {
  if (costMatrix[areaNumber][month] != 0) {

    return areaSize[areaNumber] * (rainWithENSO(rainAndEvap[month][0]) -
                                   evapWithENSO(rainAndEvap[month][3])) -
           waterUsage[month];

  } else {
    return 0;
  }
}
int netWaterProfit(int month) {
  int sum = 0;
  for (int i = 0; i < 5; i++) {
    sum += waterProfit(month, i) + areaSize[3] * pondDepth;
  }
  return sum;
}
int moneyProfit(int month) {
  int sum = 0;
  for (int i = 0; i < 5; i++) {
    sum += costMatrix[i][month] * areaSize[i];
  }
  memoiMoneyProfit[month] = sum;
  return sum;
}
int compoundedMoneyProfit(int month) {}
int main() { return 0; }
