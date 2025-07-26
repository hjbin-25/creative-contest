#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
using namespace std;

class DataCenter {
private:
    char ground[100][100];

    int nodeX[240], nodeY[240];
    double nodeBattery[240], nodeTemp[240], lastTempTime[240];
    bool isCharging[240], isCooling[240];
    int chargingTime[240], coolingTime[240], totalNodes;

    int chargingX[240], chargingY[240];
    bool hasNode[240], isOccupied[240];
    int totalCharging;

    int currentHour;
    double totalCost, totalPower;

    static constexpr double coolingConstant = 0.0003;  // 냉각 계수 (1/s)
    static constexpr double mass = 15.0;              // kg
    static constexpr double specificHeat = 900.0;     // J/kg·°C

    double currentTamb = 25.0;

    void makeGround() {
        for (int i = 0; i < 100; i++)
            for (int j = 0; j < 100; j++)
                ground[i][j] = 'G';

        for (int i = 35; i <= 65; i++)
            for (int j = 35; j < 65; j++)
                ground[i][j] = 'S';

        for (int i = 40; i < 60; i++)
            for (int j = 40; j < 60; j++)
                ground[i][j] = 'B';

        for (int i = 0; i < 100; i++) {
            for (int j = 30; j < 35; j++) {
                ground[i][j] = 'S';
                ground[j][i] = 'S';
            }
            for (int j = 65; j < 70; j++) {
                ground[i][j] = 'S';
                ground[j][i] = 'S';
            }
        }
    }

    void makeNodes() {
        totalNodes = 0;
        totalCharging = 0;

        int regions[8][4] = {
            {0, 29, 0, 29}, {0, 29, 35, 64}, {0, 29, 70, 99},
            {35, 64, 0, 29}, {35, 64, 70, 99},
            {70, 99, 0, 29}, {70, 99, 35, 64}, {70, 99, 70, 99}
        };

        for (int region = 0; region < 8; region++) {
            int startI = regions[region][0], endI = regions[region][1];
            int startJ = regions[region][2], endJ = regions[region][3];
            int chargingCount = 0, nodeCount = 0;

            for (int i = startI; i <= endI && chargingCount < 30; i += 3) {
                for (int j = startJ; j <= endJ && chargingCount < 30; j += 4) {
                    if (ground[i][j] == 'G') {
                        ground[i][j] = 'C';
                        chargingX[totalCharging] = i;
                        chargingY[totalCharging] = j;
                        isOccupied[totalCharging] = false;

                        if (nodeCount < 10) {
                            hasNode[totalCharging] = true;
                            nodeX[totalNodes] = i;
                            nodeY[totalNodes] = j;
                            nodeBattery[totalNodes] = 100.0;
                            nodeTemp[totalNodes] = 60.0;
                            lastTempTime[totalNodes] = 0.0;
                            isCharging[totalNodes] = false;
                            isCooling[totalNodes] = false;
                            chargingTime[totalNodes] = 0;
                            coolingTime[totalNodes] = 0;
                            totalNodes++;
                            nodeCount++;
                        } else {
                            hasNode[totalCharging] = false;
                        }

                        totalCharging++;
                        chargingCount++;
                    }
                }
            }
        }
    }

    double getDistance(int x1, int y1, int x2, int y2) {
        return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    }

    int findNearestCharging(int nodeIdx) {
        double minDist = 1e9;
        int nearestIdx = -1;
        for (int i = 0; i < totalCharging; i++) {
            if (!isOccupied[i]) {
                double dist = getDistance(nodeX[nodeIdx], nodeY[nodeIdx], chargingX[i], chargingY[i]);
                if (dist < minDist) {
                    minDist = dist;
                    nearestIdx = i;
                }
            }
        }
        return nearestIdx;
    }

    void moveNode(int nodeIdx) {
        if (isCharging[nodeIdx] || isCooling[nodeIdx]) return;

        int dx[] = {-2, -2, -2, -1, -1, -1, 0, 0, 1, 1, 1, 2, 2, 2};
        int dy[] = {-2, -1, 0, -2, -1, 0, -2, 2, -2, -1, 0, -2, -1, 0};

        for (int tries = 0; tries < 50; tries++) {
            int dir = rand() % 14;
            int newX = nodeX[nodeIdx] + dx[dir];
            int newY = nodeY[nodeIdx] + dy[dir];
            if (newX >= 0 && newX < 100 && newY >= 0 && newY < 100 && ground[newX][newY] == 'G') {
                nodeX[nodeIdx] = newX;
                nodeY[nodeIdx] = newY;
                break;
            }
        }
    }

    double updateTemperature(double deltaTimeSec, double T_old, double Tamb, double powerW, double k) {
        // deltaTimeSec: 시간 간격 초 단위
        // powerW: W (J/s)
        double heatIncrease = powerW * deltaTimeSec; // J
        double deltaTempIncrease = heatIncrease / (mass * specificHeat);
        double coolingEffect = k * (T_old - Tamb) * deltaTimeSec;
        return T_old + deltaTempIncrease - coolingEffect;
    }

    double getExternalTemperature(int hour) {
        const double T_min = 18.0;
        const double T_max = 33.0;
        const int t_sunrise = 6;
        const int t_sunset = 19;
        const double k = 0.2;

        int t = hour % 24;
        if (t >= t_sunrise && t <= t_sunset) {
            double ratio = (double)(t - t_sunrise) / (t_sunset - t_sunrise);
            double sinTerm = pow(sin(M_PI * ratio), 1.5);
            return T_min + (T_max - T_min) * sinTerm;
        } else {
            int t_effective = (t > t_sunset) ? (t - t_sunset) : (24 - t_sunset + t);
            double T_sunset = T_min + (T_max - T_min) * pow(sin(M_PI), 1.5);
            return T_min + (T_sunset - T_min) * exp(-k * t_effective);
        }
    }

    double getHeatGenerationRate(double nodePower = 1.25, double efficiency = 0.15) {
        return nodePower * efficiency * 1000; // kW → W (J/s)
    }

public:
    DataCenter() {
        srand(time(NULL));
        makeGround();
        makeNodes();
        currentHour = 0;
        totalCost = 0;
        totalPower = 0;
    }

    void simulateOneHour() {
        currentHour++;
        currentTamb = getExternalTemperature(currentHour);

        for (int i = 0; i < totalNodes; i++) {
            if (!isCharging[i] && !isCooling[i]) {
                nodeBattery[i] -= 4.17;
                if (nodeBattery[i] < 0) nodeBattery[i] = 0;
            }

            if (!isCooling[i]) {
                double deltaTimeSec = (currentHour - lastTempTime[i]) * 3600;
                if (deltaTimeSec >= 3600) {
                    double powerRate = getHeatGenerationRate();
                    nodeTemp[i] = updateTemperature(deltaTimeSec, nodeTemp[i], currentTamb, powerRate, coolingConstant);
                    lastTempTime[i] = currentHour;
                }
            }

            if (isCooling[i]) {
                if (++coolingTime[i] >= 1) {
                    nodeTemp[i] = 40.0;
                    lastTempTime[i] = currentHour;
                    isCooling[i] = false;
                    coolingTime[i] = 0;
                    for (int j = 0; j < totalCharging; j++) {
                        if (chargingX[j] == nodeX[i] && chargingY[j] == nodeY[i]) {
                            isOccupied[j] = false;
                            break;
                        }
                    }
                }
            }

            if (isCharging[i]) {
                if (++chargingTime[i] >= 1) {
                    nodeBattery[i] = 100.0;
                    isCharging[i] = false;
                    chargingTime[i] = 0;
                    for (int j = 0; j < totalCharging; j++) {
                        if (chargingX[j] == nodeX[i] && chargingY[j] == nodeY[i]) {
                            isOccupied[j] = false;
                            break;
                        }
                    }
                }
            }

            if (nodeTemp[i] >= 70 && !isCooling[i] && !isCharging[i]) {
                int chargingIdx = findNearestCharging(i);
                if (chargingIdx != -1) {
                    nodeX[i] = chargingX[chargingIdx];
                    nodeY[i] = chargingY[chargingIdx];
                    isOccupied[chargingIdx] = true;
                    isCooling[i] = true;
                    coolingTime[i] = 0;
                    totalCost += 625;
                    totalPower += 4.167;
                }
            } else if (nodeBattery[i] <= 30 && !isCharging[i] && !isCooling[i]) {
                int chargingIdx = findNearestCharging(i);
                if (chargingIdx != -1) {
                    nodeX[i] = chargingX[chargingIdx];
                    nodeY[i] = chargingY[chargingIdx];
                    isOccupied[chargingIdx] = true;
                    isCharging[i] = true;
                    chargingTime[i] = 0;
                }
            } else if (!isCharging[i] && !isCooling[i]) {
                moveNode(i);
            }
        }

        if (currentHour % 24 == 0) {
            totalCost += 18000000;
            totalPower += 120000;
        }
    }

    void showStatus() {
        cout << "\n=== " << currentHour << "시간 후 상태 ===" << endl;
        cout << "외부 온도: " << currentTamb << "도" << endl;
        cout << "총 비용: " << totalCost << "원" << endl;
        cout << "총 전력: " << totalPower << "kWh" << endl;

        int charging = 0, cooling = 0, moving = 0;
        double avgBattery = 0, avgTemp = 0;

        for (int i = 0; i < totalNodes; i++) {
            if (isCharging[i]) charging++;
            else if (isCooling[i]) cooling++;
            else moving++;
            avgBattery += nodeBattery[i];
            avgTemp += nodeTemp[i];
        }

        cout << "충전 중인 노드: " << charging << "개" << endl;
        cout << "냉각 중인 노드: " << cooling << "개" << endl;
        cout << "이동 중인 노드: " << moving << "개" << endl;
        cout << "평균 배터리: " << avgBattery / totalNodes << "%" << endl;
        cout << "평균 온도: " << avgTemp / totalNodes << "도" << endl;
        cout << "- 총 노드: " << totalNodes << "개" << endl;
        cout << "- 충전소 사용률: " << (charging + cooling) << "/" << totalCharging
             << " (" << ((double)(charging + cooling) / totalCharging * 100) << "%)" << endl;
    }

    void runSimulation(int hours) {
        cout << "시뮬레이션 시작!" << endl;
        showStatus();

        for (int h = 1; h <= hours; h++) {
            simulateOneHour();
            if (h % 1 == 0) {
                showStatus();
            }
        }

        cout << "\n=== 최종 결과 ===" << endl;
        showStatus();
    }
};

int main() {
    DataCenter dc;
    dc.runSimulation(12);
    return 0;
}
