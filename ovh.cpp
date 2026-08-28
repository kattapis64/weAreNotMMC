#include "ovh.h"
#include "CSC.h"

#include <algorithm>
#include <array>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#define INF 1.0e30

namespace ovh {

namespace {

// * ค่าฝนและการระเหยในแต่ละเดือนเรียงลำดับเป็น [ฝนปกติ,ฝนน้อยสุด,ระเหยปกติ,ระเหยมากสุด]
const std::array<std::array<int, 4>, 12> rainAndEvap = {{
    {20, 10, 120, 130},  {30, 15, 140, 150},  {50, 20, 160, 180},
    {80, 30, 170, 190},  {150, 40, 150, 180}, {180, 50, 140, 170},
    {200, 60, 140, 170}, {250, 50, 130, 180}, {300, 80, 120, 160},
    {200, 40, 130, 150}, {80, 50, 130, 140},  {20, 10, 120, 130}}};

// * การใช้น้ำในแต่ละพื้นที่ ลิตรต่อตารางเมตร
const std::array<double, N_AREA> waterUsagePerM2 = {180.0, 90.0, 120.0, 0.0, 20.0};

// * คิดเงินที่ต้องใช้ในการลงทุนรอบแรก (สำหรับใช้กับ constraint ไม่เกิน 200000)
std::array<double, N_AREA> setupCost(const vector<vector<double>> &C) {
  std::array<double, N_AREA> costs{};
  for (int a = 0; a < N_AREA; a++) {
    costs[a] = 0.0;
    for (int m = 0; m < MONTHS; m++) {
      if (C[a][m] < 0) {
        costs[a] = -C[a][m];
        break;
      }
    }
  }
  return costs;
}

} // namespace
// * สร้างเวกเตอร์ 2 มิติ ระหว่างอันดับของพื้นที่ต่าง ๆ กับ เดือน แสดงรายรับ-จ่ายของแต่ละพื้นที่ในแต่ละเดือน
vector<vector<double>> costMatrix(const optimizeVariable &ov) {
  vector<vector<double>> C(N_AREA, vector<double>(MONTHS, 0.0));
  // * เดือนแรกเสียเงิน 30 ต่อตารางเมตรในการสร้างบ่อน้ำ
  C[3][0] = -30;
  // * ในปีต่อ ๆ ไป เสีย 20
  if (MONTHS > 12) C[3][12] = -20;

  for (int y = 0; y < YEARS; y++) {
    int monthsY = y * 12;

    int rs = ov.riceMonth + monthsY;
    if (rs < MONTHS) {
      // * เริ่มต้นจะเสียเงิน 15 ต่อตารางเมตร
      C[0][rs] = -15;
      // *  ระหว่างเดือน set เป็น 1 สำหรับให้บอกว่ามีการใช้น้ำ
      for (int m = rs + 1; m < std::min(rs + 4, MONTHS); m++) C[0][m] = 1;
      // * เช็คไม่ให้เกิน vector bound แล้วใส่ค่าตอบแทนที่ได้
      if (rs + 4 < MONTHS) C[0][rs + 4] = 45;
    }

    int ts = ov.treeMonth + monthsY;
    if (ts < MONTHS) {
      // * เริ่มต้นจะเสียเงิน 40 ต่อตารางเมตร
      C[1][ts] = -40;
      // *  ระหว่างเดือน set เป็น 1 สำหรับให้บอกว่ามีการใช้น้ำ
      for (int m = ts + 1; m < std::min(ts + 12, MONTHS); m++) C[1][m] = 1;
      // * เช็คไม่ให้เกิน vector bound แล้วใส่ค่าตอบแทนที่ได้
      if (ts + 12 < MONTHS) C[1][ts + 12] = 65;
    }

    for (int m = 0; m < 12; m++) {
      // ? พิจารณาเดือนที่ไม่ใช่เดือน noCropMonth_0 , noCropMonth_0 + 1 , noCropMonth_1 , noCropMonth_1 + 1
      if (m < ov.noCropMonth_0 || m > 1 + ov.noCropMonth_1) {
        if (m % 2 == (ov.noCropMonth_0 - 2) % 2) {
          // * เริ่มต้นเสีย 25 บาทต่อตารางเมตร
          C.at(2).at(m + 12 * y) = -25;
        } else {
          // * ผ่านไป 2 เดือน ได้ 80 บาทต่อตารางเมตร
          C.at(2).at(m + 12 * y) = 80;
        }
      }
    }
    
    // * ทุกเดือนแรก ที่มากกว่าเท่ากับปีที่ 2 ให้เสียเงิน -30 บาทต่อตารางเมตรบ่อน้ำ (หลบ ไม่ให้ขัดกับ อันแรก)
    if (y >= 2 && monthsY < MONTHS) C[3][monthsY] = -30;
  }
  // * เสียเงิน 500 บาทต่อตารางเมตรในการสร้างบ้าน
  if (MONTHS > 0) C[4][0] = -500;
  return C;
}

SolveResult solveLp(const optimizeVariable &ov, double floorRice,double floorTree, double floorHouse) {
  SolveResult result;

  auto C = costMatrix(ov);

  // * Array กำไรรวมทุกปีแยกพื้นที่ ช่วยคิดตอน objective เพื่อหากำไรที่มากที่สุด
  std::array<double, N_AREA> totalRevenueByArea{};
  for (int a = 0; a < N_AREA; a++) {
    double s = 0.0;
    for (int m = 0; m < MONTHS; m++) s += C[a][m];
    totalRevenueByArea[a] = s;
  }
  auto initCost = setupCost(C);

  /*
  * อัลกอริทึม คิด LP
  * - Variables ตัวแปรที่จะ optimize
  *     x พื้นที่ทั้ง 5 ประเภท |A|_i
  *     x ปริมาตรของน้ำในบ่อแต่ละเดือน S_m
  *     x Area soft constraint Slack_i -> Optimizer สามารถโกงโดยการลดพื้นที่หนึ่ง ๆ ให้น้อยกว่า minimum arbitrary requirement ได้แต่ผ่าน Slack แต่ต้องโดนลงโทษ (penalty)
  *
  * - Constraint
  *     x ปริมาตรน้ำในบ่อแต่ละเดือนต้องมากกว่า treshold
  *     x พื้นที่ทั้งหมด + slack + ต้องรวมกันได้ 4 ไร่
  *     x พื้นที่แต่ละพื้นที่ + slack >= minimum arbitrary requirement
  *     x ปริมาณน้ำที่เพิ่มขึ้นมาในเดือนนั้น จะต้องไม่มากกว่า ปริมาณน้ำฝนที่ตกลงมา ระเหยออกไปจากบ่อ ลบด้วยปริมาณการใช้งานน้ำ
  * - Goal
  *     x ปริมาณกำไรรวมตลอดทุกปีสูงสุด
  *
  */ 



  const int S0 = N_AREA;
  const int SLACK_RICE = S0 + MONTHS;
  const int SLACK_TREE = SLACK_RICE + 1;
  const int SLACK_HOUSE = SLACK_TREE + 1;
  const int NVAR = SLACK_HOUSE + 1;

  vector<double> objective(NVAR, 0.0);
  for (int a = 0; a < N_AREA; a++) objective[a] = totalRevenueByArea[a];
  objective[SLACK_RICE] = -PENALTY;
  objective[SLACK_TREE] = -PENALTY;
  objective[SLACK_HOUSE] = -PENALTY;

  // rows: 2 per month (rain balance + storage-capacity cap) + 3 floor rows
  // + 1 budget row + 1 equality row (total land == FARM_AREA)
  const int numRows = 2 * MONTHS + 3 + 1 + 1;
  vector<vector<double>> A(numRows, vector<double>(NVAR, 0.0));
  vector<double> rowLower(numRows, -INF);
  vector<double> rowUpper(numRows, INF);

  int row = 0;
  for (int m = 0; m < MONTHS; m++) {
    double rain = rainAndEvap[m % 12][0] * FARM_AREA / L_PER_M3;
    double evapCoef = rainAndEvap[m % 12][2] / L_PER_M3;

    // S_t - S_{t-1} + evap*pond + water-use(active areas) <= rain
    A[row][S0 + m] += 1;
    if (m > 0)
      A[row][S0 + m - 1] += -1;
    else
      A[row][3] += -INIT_FRAC * POND_DEPTH_M; // initial pond fill
    A[row][3] += evapCoef;
    if (C.at(0).at(m)!=0) A[row][0] += waterUsagePerM2[0] / L_PER_M3;
    if (C.at(1).at(m)!=0) A[row][1] += waterUsagePerM2[1] / L_PER_M3;
    if (C.at(2).at(m)!=0) A[row][2] += waterUsagePerM2[2] / L_PER_M3;
    A[row][4] += waterUsagePerM2[4] / L_PER_M3;
    rowUpper[row] = rain;
    row++;

    // S_t <= POND_DEPTH_M * pond_area  (storage capacity)
    A[row][S0 + m] = 1;
    A[row][3] = -POND_DEPTH_M;
    rowUpper[row] = 0;
    row++;
  }

  // soft floors: area + slack >= floor  <=>  -area - slack <= -floor
  A[row][0] = -1; A[row][SLACK_RICE] = -1; rowUpper[row] = -floorRice; row++;
  A[row][1] = -1; A[row][SLACK_TREE] = -1; rowUpper[row] = -floorTree; row++;
  A[row][4] = -1; A[row][SLACK_HOUSE] = -1; rowUpper[row] = -floorHouse; row++;

  // initial investment budget
  for (int a = 0; a < N_AREA; a++)
    if (initCost[a] != 0.0) A[row][a] = initCost[a];
  rowUpper[row] = BUDGET_LIMIT;
  row++;

  // total land == FARM_AREA
  for (int a = 0; a < N_AREA; a++) A[row][a] = 1;
  rowLower[row] = FARM_AREA;
  rowUpper[row] = FARM_AREA;
  row++;

  vector<double> colLower(NVAR, 0.0), colUpper(NVAR, INF);
  for (int m = 0; m < MONTHS; m++) colLower[S0 + m] = V_THRESHOLD;

  try {
    auto x = solveLP(objective, A, rowLower, rowUpper, colLower, colUpper,
                      /*maximize=*/true);
    result.feasible = true;
    for (int a = 0; a < N_AREA; a++) result.alloc[a] = x[a];
    result.slack_rice = x[SLACK_RICE];
    result.slack_tree = x[SLACK_TREE];
    result.slack_house = x[SLACK_HOUSE];
    result.fully_met =
        (result.slack_rice + result.slack_tree + result.slack_house) < 1e-6;

    double profit = 0.0;
    for (int a = 0; a < N_AREA; a++) profit += totalRevenueByArea[a] * x[a];
    result.profit = profit;

    double minS = INF;
    for (int m = 0; m < MONTHS; m++) minS = std::min(minS, x[S0 + m]);
    result.min_storage = minS;
  } catch (const std::exception &) {
    result.feasible = false;
  }
  return result;
}

SolveResult sweepBestScale(const optimizeVariable &ov, double baseRice,
                            double baseTree, double baseHouse,
                            double maxScale, int steps) {
  SolveResult best; // feasible == false until we find a fully-met solution

  std::cout << std::right << std::setw(6) << "scale" << std::setw(9) << "rice"
            << std::setw(9) << "tree" << std::setw(9) << "house"
            << std::setw(10) << "feasible" << std::setw(6) << "met"
            << std::setw(16) << "profit" << std::setw(9) << "minS" << "\n";

  for (int i = 0; i < steps; i++) {
    double s = (steps > 1) ? maxScale * i / (steps - 1) : 0.0;
    double fr = s * baseRice, ft = s * baseTree, fh = s * baseHouse;
    auto r = solveLp(ov, fr, ft, fh);

    std::cout << std::fixed << std::setprecision(2) << std::setw(6) << s
               << std::setprecision(0) << std::setw(9) << fr << std::setw(9)
               << ft << std::setw(9) << fh;
    if (!r.feasible) {
      std::cout << std::setw(10) << "NO" << "\n";
    } else {
      std::cout << std::setw(10) << "yes" << std::setw(6)
                 << (r.fully_met ? "yes" : "no") << std::setprecision(0)
                 << std::setw(16) << r.profit << std::setprecision(2)
                 << std::setw(9) << r.min_storage << "\n";
    }

    if (r.feasible && r.fully_met && (r.alloc.at(0) > best.alloc.at(0))) {
      best = r;
      best.scale = s;
      best.feasible = true;
    }
  }
  return best;
}

} // namespace ovh