#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
using namespace std;

class DataCenter {
private:
    // 땅 상태 저장 (잔디 'G', 바위 'B', 충전소 'C', 기타 'S')
    char ground[100][100];
    // 각 노드의 현재 위치 x, y 좌표
    int nodeX[240], nodeY[240];
    // 각 노드 배터리 잔량
    double nodeBattery[240];
    // 각 노드의 현재 온도 저장
    double nodeTemp[240];
    // 각 노드가 충전 중인지 여부
    bool isCharging[240];
    // 각 노드가 냉각 중인지 여부
    bool isCooling[240];
    // 각 노드의 충전 진행 시간(시간 단위)
    int chargingTime[240];
    // 각 노드의 냉각 진행 시간(시간 단위)
    int coolingTime[240];
    // 총 노드 개수
    int totalNodes;
    // 충전소 위치들의 x, y 좌표 배열
    int chargingX[240], chargingY[240];
    // 충전소 위치에 노드가 배치되어 있는지 여부
    bool hasNode[240];
    // 충전소가 현재 노드에 의해 사용 중인지 여부
    bool isOccupied[240];
    // 총 충전소 개수
    int totalCharging;
    // 현재 시뮬레이션 진행 시간 (시간 단위)
    int currentHour;
    // 하루 동안 누적된 냉각 비용 합계
    long long dailyCoolingCost;
    // 하루 동안 누적된 공연 수익 합계
    long long dailyPerformanceRevenue;
    // 하루 동안 발생한 총 냉각 이벤트 수
    int totalCoolingEvents;
    // 시뮬레이션에서 경과한 달 수 (30일 단위로 증가)
    int monthCount;
    // 냉각 비용 기록용 파일 스트림
    ofstream freezeCostFile;
    // 공연 수익 기록용 파일 스트림
    ofstream performanceRevenueFile;
    // 24시간 동안 시간별 외부 온도 데이터 (시간대별 섭씨 온도)
    const double hourlyTemperatures[24] = {
        13.0, 12.5, 12.0, 11.5, 11.0, 11.0,
        12.0, 13.5, 15.0, 17.0, 19.0, 21.0,
        22.5, 23.5, 24.0, 23.5, 22.0, 20.0,
        18.0, 16.5, 15.5, 14.5, 14.0, 13.5
    };
    // 현재 외부 온도 값
    double currentTamb;

    // 땅 구성 초기화 (잔디, 바위, 충전소 등)
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

    // 노드 및 충전소 배치 초기화
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

            // 지역별로 충전소 최대 30개, 노드 최대 10개 배치
            for (int i = startI; i <= endI && chargingCount < 30; i += 3) {
                for (int j = startJ; j <= endJ && chargingCount < 30; j += 4) {
                    if (ground[i][j] == 'G') {
                        ground[i][j] = 'C';  // 충전소 표시
                        chargingX[totalCharging] = i;
                        chargingY[totalCharging] = j;
                        isOccupied[totalCharging] = false;

                        if (nodeCount < 10) {
                            hasNode[totalCharging] = true;
                            nodeX[totalNodes] = i;
                            nodeY[totalNodes] = j;
                            nodeBattery[totalNodes] = 100.0;
                            nodeTemp[totalNodes] = 60.0 + (rand() % 16);
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

    // 두 점 사이 거리 계산
    double getDistance(int x1, int y1, int x2, int y2) {
        return sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
    }

    // 노드가 가장 가까운 빈 충전소 찾기
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

    // 노드 이동 시도
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

    // 현재 시간에 따른 외부 온도 반환
    double getExternalTemperature(int hour) {
        return hourlyTemperatures[hour % 24];
    }

    // 노드 온도 갱신
    void updateNodeTemperature(int nodeIdx) {
        if (isCooling[nodeIdx]) return;

        double heatIncrease = 15.0 + (rand() % 6);

        if (nodeBattery[nodeIdx] > 50.0) {
            heatIncrease += 5.0;
        }

        if (!isCharging[nodeIdx] && !isCooling[nodeIdx]) {
            heatIncrease += 3.0;
        }

        double currentNodeTemp = nodeTemp[nodeIdx];
        double naturalCooling = 0.0;

        if (currentNodeTemp > currentTamb) {
            double tempDiff = currentNodeTemp - currentTamb;
            double coolingRate = 0.005 + (tempDiff * 0.001);

            int hourOfDay = currentHour % 24;
            if (hourOfDay >= 22 || hourOfDay <= 6) {
                coolingRate *= 1.02;
            }

            naturalCooling = tempDiff * coolingRate;
        }

        double netTempChange = heatIncrease - naturalCooling;
        nodeTemp[nodeIdx] += netTempChange;

        if (nodeTemp[nodeIdx] < currentTamb) nodeTemp[nodeIdx] = currentTamb;
        if (nodeTemp[nodeIdx] > 95.0) nodeTemp[nodeIdx] = 95.0;
    }

    // 냉각 비용 계산
    long long calculateCoolingCost(double startTemp, double targetTemp) {
        double coolingAmount = max(0.0, startTemp - targetTemp);
        const double costPerDegree = 0.8;
        long long cost = static_cast<long long>(coolingAmount * costPerDegree);
        if (cost > 0 && cost < 10) cost = 10;
        return cost;
    }

public:
    DataCenter() {
        srand(time(NULL));
        makeGround();
        makeNodes();
        currentHour = 0;
        dailyCoolingCost = 0;
        dailyPerformanceRevenue = 0;
        totalCoolingEvents = 0;
        monthCount = 0;

        freezeCostFile.open("freezeCost.data");
        performanceRevenueFile.open("performanceRevenue.data");
        if (!freezeCostFile.is_open() || !performanceRevenueFile.is_open()) {
            cerr << "파일 열기 실패!" << endl;
            exit(1);
        }
    }

    ~DataCenter() {
        if (freezeCostFile.is_open()) freezeCostFile.close();
        if (performanceRevenueFile.is_open()) performanceRevenueFile.close();
    }

    // 1시간 단위 시뮬레이션 진행
    void simulateOneHour() {
        currentHour++;
        currentTamb = getExternalTemperature(currentHour);

        // 공연 시간 동안 수익 발생
        bool fireflyShowTime = ((currentHour % 24) >= 20 && (currentHour % 24) <= 22);

        if (fireflyShowTime) {
            int attendees = 75 + rand() % 31;
            int ticketPrice = 10000;
            dailyPerformanceRevenue += static_cast<long long>(attendees) * ticketPrice;

            int goodsPrice = 25000;
            dailyPerformanceRevenue += static_cast<long long>(attendees * 0.2) * goodsPrice;
        }

        for (int i = 0; i < totalNodes; i++) {
            // 충전 중 처리
            if (isCharging[i]) {
                chargingTime[i]++;
                if (chargingTime[i] >= 1) {
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
                continue;
            }

            // 냉각 중 처리
            if (isCooling[i]) {
                coolingTime[i]++;
                if (coolingTime[i] >= 1) {
                    double startTemp = nodeTemp[i];
                    nodeTemp[i] = 40.0;
                    isCooling[i] = false;
                    coolingTime[i] = 0;

                    long long coolingCost = calculateCoolingCost(startTemp, 40.0);
                    dailyCoolingCost += coolingCost;
                    totalCoolingEvents++;

                    for (int j = 0; j < totalCharging; j++) {
                        if (chargingX[j] == nodeX[i] && chargingY[j] == nodeY[i]) {
                            isOccupied[j] = false;
                            break;
                        }
                    }
                }
                continue;
            }

            // 배터리 소모
            nodeBattery[i] -= 4.17;
            if (nodeBattery[i] < 0) nodeBattery[i] = 0;

            updateNodeTemperature(i);

            if (fireflyShowTime) continue;

            // 고온 시 냉각소 이동
            if (nodeTemp[i] >= 65.0) {
                int chargingIdx = findNearestCharging(i);
                if (chargingIdx != -1) {
                    nodeX[i] = chargingX[chargingIdx];
                    nodeY[i] = chargingY[chargingIdx];
                    isOccupied[chargingIdx] = true;
                    isCooling[i] = true;
                    coolingTime[i] = 0;
                }
            }
            // 배터리 부족 시 충전소 이동
            else if (nodeBattery[i] <= 30.0) {
                int chargingIdx = findNearestCharging(i);
                if (chargingIdx != -1) {
                    nodeX[i] = chargingX[chargingIdx];
                    nodeY[i] = chargingY[chargingIdx];
                    isOccupied[chargingIdx] = true;
                    isCharging[i] = true;
                    chargingTime[i] = 0;
                }
            }
            // 그 외에는 자유 이동
            else {
                moveNode(i);
            }
        }

        // 1달 단위(30일)로 비용 및 수익 기록
        if (currentHour % (24 * 30) == 0) {
            monthCount++;
            freezeCostFile << monthCount << " " << dailyCoolingCost << "\n";
            freezeCostFile.flush();
            performanceRevenueFile << monthCount << " " << dailyPerformanceRevenue << "\n";
            performanceRevenueFile.flush();

            dailyCoolingCost = 0;
            dailyPerformanceRevenue = 0;
            totalCoolingEvents = 0;
        }
    }

    // 현재 상태 요약 (필요시 주석 해제 후 확인 가능)
    void showStatus() {
        int charging = 0, cooling = 0, moving = 0;
        int highTempNodes = 0, criticalTempNodes = 0, naturalCooledNodes = 0;
        double avgBattery = 0, avgTemp = 0;
        double minBattery = 100, maxBattery = 0;
        double minTemp = 100, maxTemp = 0;

        for (int i = 0; i < totalNodes; i++) {
            if (isCharging[i]) charging++;
            else if (isCooling[i]) cooling++;
            else moving++;

            if (nodeTemp[i] >= 60.0) highTempNodes++;
            if (nodeTemp[i] >= 65.0) criticalTempNodes++;
            if (nodeTemp[i] < 60.0) naturalCooledNodes++;

            avgBattery += nodeBattery[i];
            avgTemp += nodeTemp[i];

            minBattery = min(minBattery, nodeBattery[i]);
            maxBattery = max(maxBattery, nodeBattery[i]);
            minTemp = min(minTemp, nodeTemp[i]);
            maxTemp = max(maxTemp, nodeTemp[i]);
        }

        // cout << "충전 중인 노드: " << charging << "개" << endl;
        // cout << "인공냉각 중인 노드: " << cooling << "개" << endl;
        // cout << "이동 중인 노드: " << moving << "개" << endl;
        // cout << "자연냉각 중인 노드(<60도): " << naturalCooledNodes << "개 ("
        //      << (naturalCooledNodes * 100 / totalNodes) << "%)" << endl;
        // cout << "평균 배터리: " << (avgBattery / totalNodes) << "% (범위: "
        //      << minBattery << "% ~ " << maxBattery << "%)" << endl;
        // cout << "평균 온도: " << (avgTemp / totalNodes) << "도 (범위: "
        //      << minTemp << "도 ~ " << maxTemp << "도)" << endl;
        // cout << "총 노드: " << totalNodes << "개" << endl;
        // cout << "충전소 사용률: " << (charging + cooling) << "/" << totalCharging
        //      << " (" << ((double)(charging + cooling) / totalCharging * 100) << "%)" << endl;
    }

    // 지정 시간만큼 시뮬레이션 실행
    void runSimulation(int hours) {
        for (int i = 0; i < hours; i++) {
            simulateOneHour();
            if (i % 24 == 0) {
                showStatus();
            }
        }
    }
};

int main() {
    DataCenter dc;
    dc.runSimulation(24 * 3600);  // 60일(2달) 시뮬레이션
    return 0;
}