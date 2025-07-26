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
    double totalRevenue; // ✅ 행사 수익

    static constexpr double coolingConstant = 0.001;
    static constexpr double mass = 15.0;
    static constexpr double specificHeat = 900.0;

    const double hourlyTemperatures[24] = {
        13.0, 12.5, 12.0, 11.5, 11.0, 11.0,
        12.0, 13.5, 15.0, 17.0, 19.0, 21.0,
        22.5, 23.5, 24.0, 23.5, 22.0, 20.0,
        18.0, 16.5, 15.5, 14.5, 14.0, 13.5
    };

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

    double calcTemperature(double deltaTime, double T0, double Tamb, double energyJ, double k) {
        double expTerm = exp(-k * deltaTime * 3600);
        double deltaT = energyJ / (mass * specificHeat);
        double T1 = T0 + deltaT;
        return Tamb + (T1 - Tamb) * expTerm;
    }

    double getExternalTemperature(int hour) {
        return hourlyTemperatures[hour % 24];
    }

    double getHeatGeneration(double nodePower = 1.25, double efficiency = 0.15) {
        return nodePower * efficiency * 3600 * 1000;
    }

    double calculateCoolingCost(double nodeTemp, double externalTemp) {
        double deltaT = max(0.0, nodeTemp - externalTemp);
        const double costPerDegree = 0.7;
        return deltaT * costPerDegree;
    }

public:
    DataCenter() {
        srand(time(NULL));
        makeGround();
        makeNodes();
        currentHour = 0;
        totalCost = 0.0;
        totalPower = 0.0;
        totalRevenue = 0.0;
    }

    void simulateOneHour() {
        currentHour++;
        currentTamb = getExternalTemperature(currentHour);

        bool fireflyShowTime = ((currentHour % 24) >= 20 && (currentHour % 24) <= 21);
        if (fireflyShowTime) {
            int attendees = 100 + rand() % 51;  // 100~150명
            double ticketPrice = 1000.0;
            totalRevenue += attendees * ticketPrice;
        }

        for (int i = 0; i < totalNodes; i++) {
            if (!isCharging[i] && !isCooling[i]) {
                nodeBattery[i] -= 4.17;
                nodeBattery[i] = max(0.0, nodeBattery[i]);
            }

            if (!isCooling[i]) {
                double deltaTime = currentHour - lastTempTime[i];
                if (deltaTime >= 1.0) {
                    double Q = getHeatGeneration(i);
                    nodeTemp[i] = calcTemperature(deltaTime, nodeTemp[i], currentTamb, Q, coolingConstant);
                    lastTempTime[i] = currentHour;
                }
            }

            if (isCooling[i]) {
                if (fireflyShowTime) {
                    coolingTime[i]++;
                    nodeTemp[i] -= 2.0;
                    nodeTemp[i] = max(40.0, nodeTemp[i]);
                } else {
                    coolingTime[i]++;
                    nodeTemp[i] -= 10.0;
                    nodeTemp[i] = max(40.0, nodeTemp[i]);
                    double cost = calculateCoolingCost(nodeTemp[i], currentTamb);
                    totalCost += cost;
                    totalPower += cost / 150.0;
                }

                if (coolingTime[i] >= 3) {
                    isCooling[i] = false;
                    coolingTime[i] = 0;
                    lastTempTime[i] = currentHour;
                    for (int j = 0; j < totalCharging; j++) {
                        if (chargingX[j] == nodeX[i] && chargingY[j] == nodeY[i]) {
                            isOccupied[j] = false;
                            break;
                        }
                    }
                }
            }

            if (fireflyShowTime) continue;

            if (nodeTemp[i] >= 70 && !isCooling[i] && !isCharging[i]) {
                int chargingIdx = findNearestCharging(i);
                if (chargingIdx != -1) {
                    nodeX[i] = chargingX[chargingIdx];
                    nodeY[i] = chargingY[chargingIdx];
                    isOccupied[chargingIdx] = true;
                    isCooling[i] = true;
                    coolingTime[i] = 0;
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
        cout << "공연 수익: " << totalRevenue << "원" << endl;

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
            if (h % 1 == 0) showStatus();
        }

        cout << "\n=== 최종 결과 ===" << endl;
        showStatus();
    }
};

int main() {
    DataCenter dc;
    dc.runSimulation(24);
    return 0;
}
