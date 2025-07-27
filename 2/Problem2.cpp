#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
using namespace std;

class DataCenter {
private:
    char ground[100][100];

    int nodeX[240], nodeY[240];
    double nodeBattery[240], nodeTemp[240];
    bool isCharging[240], isCooling[240];
    int chargingTime[240], coolingTime[240], totalNodes;

    int chargingX[240], chargingY[240];
    bool hasNode[240], isOccupied[240];
    int totalCharging;

    int currentHour;
    long long dailyCoolingCost;
    long long dailyPerformanceRevenue;
    int dayCount;
    int totalCoolingEvents;

    ofstream freezeCostFile;
    ofstream performanceRevenueFile;

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
                            // 초기 온도를 더욱 높게 설정 (60-75도)
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

    double getExternalTemperature(int hour) {
        return hourlyTemperatures[hour % 24];
    }

    // 강력한 열 발생과 약한 자연냉각으로 수정
    void updateNodeTemperature(int nodeIdx) {
        if (isCooling[nodeIdx]) return; // 인공냉각 중이면 온도 변화 없음
        
        // 1. 강력한 열 발생: 매시간 15-20도 상승
        double heatIncrease = 15.0 + (rand() % 6); // 15~20도
        
        // 배터리 사용량이 높으면 추가 발열
        if (nodeBattery[nodeIdx] > 50.0) {
            heatIncrease += 5.0;
        }
        
        // 이동 중일 때 더 많은 추가 발열
        if (!isCharging[nodeIdx] && !isCooling[nodeIdx]) {
            heatIncrease += 3.0;
        }
        
        // 2. 매우 약한 자연냉각
        double currentNodeTemp = nodeTemp[nodeIdx];
        double naturalCooling = 0.0;
        
        if (currentNodeTemp > currentTamb) {
            double tempDiff = currentNodeTemp - currentTamb;
            
            // 자연냉각을 거의 무력화
            double coolingRate = 0.005 + (tempDiff * 0.001); // 매우 약한 냉각
            
            // 밤시간에도 미미한 보너스만
            int hourOfDay = currentHour % 24;
            if (hourOfDay >= 22 || hourOfDay <= 6) {
                coolingRate *= 1.02; // 거의 의미없는 보너스
            }
            
            naturalCooling = tempDiff * coolingRate;
        }
        
        // 3. 최종 온도 변화 (거의 열 발생량과 동일)
        double netTempChange = heatIncrease - naturalCooling;
        nodeTemp[nodeIdx] += netTempChange;
        
        // 4. 온도 제한
        if (nodeTemp[nodeIdx] < currentTamb) {
            nodeTemp[nodeIdx] = currentTamb;
        }
        if (nodeTemp[nodeIdx] > 95.0) {
            nodeTemp[nodeIdx] = 95.0;
        }
        
        // 온도 변화 모니터링 (더 자주 출력)
        if (rand() % 100 < 2) { // 2% 확률로 출력
            cout << "노드 " << nodeIdx << ": 열+" << heatIncrease 
                 << "도, 자연냉각-" << naturalCooling 
                 << "도 → " << nodeTemp[nodeIdx] << "도" << endl;
        }
        
        // 70도 근처 도달 시 알림
        if (nodeTemp[nodeIdx] >= 65.0 && nodeTemp[nodeIdx] < 70.0) {
            if (rand() % 50 < 1) { // 가끔 출력
                cout << "⚠️ 노드 " << nodeIdx << " 고온 경고: " << nodeTemp[nodeIdx] << "도" << endl;
            }
        }
    }

    // 냉각 비용 함수 - 일별 20,000원 목표로 조정
    long long calculateCoolingCost(double startTemp, double targetTemp) {
        double coolingAmount = max(0.0, startTemp - targetTemp);
        
        // 현재 약 127만원/일이 나오므로 1/64 수준으로 조정
        // 127만원 ÷ 64 ≈ 20,000원
        const double costPerDegree = 0.8; // 50에서 0.8로 대폭 감소
        long long cost = static_cast<long long>(coolingAmount * costPerDegree);
        
        // 최소 비용을 낮춤 (냉각이 발생하면 최소 10원)
        if (cost > 0 && cost < 10) {
            cost = 10;
        }
        
        return cost;
    }

public:
    DataCenter() {
        srand(time(NULL));
        makeGround();
        makeNodes();
        currentHour = 0;
        dayCount = 0;
        dailyCoolingCost = 0;
        dailyPerformanceRevenue = 0;
        totalCoolingEvents = 0;

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

    void simulateOneHour() {
        currentHour++;
        currentTamb = getExternalTemperature(currentHour);

        bool fireflyShowTime = ((currentHour % 24) >= 20 && (currentHour % 24) <= 21);

        // 공연 수익 계산
        if (fireflyShowTime) {
            int attendees = 50 + rand() % 35;
            int ticketPrice = 10000;
            dailyPerformanceRevenue += static_cast<long long>(attendees) * ticketPrice;

            int goodsPrice = 25000;
            dailyPerformanceRevenue += static_cast<long long>(attendees * 0.2) * goodsPrice;
        }

        for (int i = 0; i < totalNodes; i++) {
            // 충전 처리
            if (isCharging[i]) {
                chargingTime[i]++;
                if (chargingTime[i] >= 1) {
                    nodeBattery[i] = 100.0;
                    isCharging[i] = false;
                    chargingTime[i] = 0;
                    
                    // 충전소 해제
                    for (int j = 0; j < totalCharging; j++) {
                        if (chargingX[j] == nodeX[i] && chargingY[j] == nodeY[i]) {
                            isOccupied[j] = false;
                            break;
                        }
                    }
                }
                continue;
            }

            // 냉각 처리
            if (isCooling[i]) {
                coolingTime[i]++;
                if (coolingTime[i] >= 1) {
                    double startTemp = nodeTemp[i];
                    nodeTemp[i] = 40.0;
                    isCooling[i] = false;
                    coolingTime[i] = 0;
                    
                    // 냉각 비용 계산
                    long long coolingCost = calculateCoolingCost(startTemp, 40.0);
                    dailyCoolingCost += coolingCost;
                    totalCoolingEvents++;
                    
                    cout << "[ " << (currentHour % 24) << "시 ] ❄️  노드 " << i << " 인공냉각 완료: " 
                         << startTemp << "도 → 40도 (비용: " << coolingCost << "원)" << endl;
                    
                    // 충전소 해제
                    for (int j = 0; j < totalCharging; j++) {
                        if (chargingX[j] == nodeX[i] && chargingY[j] == nodeY[i]) {
                            isOccupied[j] = false;
                            break;
                        }
                    }
                }
                continue;
            }

            // 배터리 감소
            nodeBattery[i] -= 4.17;
            if (nodeBattery[i] < 0) nodeBattery[i] = 0;

            // 온도 상승
            updateNodeTemperature(i);

            // 공연 시간에는 노드 이동 제한
            if (fireflyShowTime) continue;

            // 온도가 65도 이상일 때 인공냉각 (기준 낮춤)
            if (nodeTemp[i] >= 65.0) {
                int chargingIdx = findNearestCharging(i);
                if (chargingIdx != -1) {
                    cout << "[ " << (currentHour % 24) << "시 ] 🔥 노드 " << i << " 인공냉각 시작: " 
                         << nodeTemp[i] << "도 (65도 기준 초과)" << endl;
                    nodeX[i] = chargingX[chargingIdx];
                    nodeY[i] = chargingY[chargingIdx];
                    isOccupied[chargingIdx] = true;
                    isCooling[i] = true;
                    coolingTime[i] = 0;
                } else {
                    cout << "⚠️ 노드 " << i << " 냉각 필요하지만 충전소 부족! (온도: " << nodeTemp[i] << "도)" << endl;
                }
            }
            // 배터리가 낮으면 충전소로 이동
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
            // 일반 이동
            else {
                moveNode(i);
            }
        }

        // 하루가 끝나면 데이터 기록
        if (currentHour % 24 == 0) {
            dayCount++;
            
            cout << "\n### " << dayCount << "일차 종료 ###" << endl;
            cout << "인공냉각 이벤트: " << totalCoolingEvents << "회" << endl;
            cout << "일일 냉각 비용: " << dailyCoolingCost << "원" << endl;
            
            // 자연냉각 효율성 계산
            double artificialCoolingRate = (double)totalCoolingEvents / totalNodes * 100;
            double naturalCoolingRate = 100.0 - artificialCoolingRate;
            
            cout << "🌿 자연냉각 비율: " << naturalCoolingRate << "%" << endl;
            cout << "⚡ 인공냉각 비율: " << artificialCoolingRate << "%" << endl;
            
            if (naturalCoolingRate >= 90.0) {
                cout << "✅ 목표 달성! 90% 이상 자연냉각으로 비용 절약!" << endl;
            } else {
                cout << "🎯 목표: 90% 자연냉각 (현재: " << naturalCoolingRate << "%)" << endl;
            }

            // 파일에 기록
            freezeCostFile << dayCount << " " << dailyCoolingCost << "\n";
            freezeCostFile.flush(); // 즉시 파일에 쓰기
            performanceRevenueFile << dayCount << " " << dailyPerformanceRevenue << "\n";
            performanceRevenueFile.flush(); // 즉시 파일에 쓰기

            // 리셋
            dailyCoolingCost = 0;
            dailyPerformanceRevenue = 0;
            totalCoolingEvents = 0;
        }
    }

    void showStatus() {
        cout << "\n=== " << dayCount << "일째 상태 ===" << endl;
        cout << "현재 시간: " << (currentHour % 24) << "시" << endl;
        cout << "외부 온도: " << currentTamb << "도" << endl;
        cout << "오늘 냉각 비용: " << dailyCoolingCost << "원" << endl;
        cout << "오늘 공연 수익: " << dailyPerformanceRevenue << "원" << endl;

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

        cout << "충전 중인 노드: " << charging << "개" << endl;
        cout << "인공냉각 중인 노드: " << cooling << "개" << endl;
        cout << "이동 중인 노드: " << moving << "개" << endl;
        cout << "🌡️ 자연냉각 중인 노드(<60도): " << naturalCooledNodes << "개 (" 
             << (naturalCooledNodes * 100 / totalNodes) << "%)" << endl;
        cout << "🔥 고온 노드(60-64도): " << (highTempNodes - criticalTempNodes) << "개" << endl;
        cout << "⚠️ 위험 노드(65도+): " << criticalTempNodes << "개" << endl;
        cout << "평균 배터리: " << (avgBattery / totalNodes) << "% (범위: " 
             << minBattery << "% ~ " << maxBattery << "%)" << endl;
        cout << "평균 온도: " << (avgTemp / totalNodes) << "도 (범위: " 
             << minTemp << "도 ~ " << maxTemp << "도)" << endl;
        cout << "총 노드: " << totalNodes << "개" << endl;
        cout << "충전소 사용률: " << (charging + cooling) << "/" << totalCharging
             << " (" << ((double)(charging + cooling) / totalCharging * 100) << "%)" << endl;
    }

    void runSimulation(int hours) {
        cout << "🚀 데이터센터 시뮬레이션 시작!" << endl;
        cout << "총 노드: " << totalNodes << "개, 충전소: " << totalCharging << "개" << endl;
        showStatus();

        for (int h = 1; h <= hours; h++) {
            simulateOneHour();
            if (h % 24 == 0) {
                cout << "\n=== " << h / 24 << "일째 결과 ===" << endl;
                showStatus();
            }
        }

        cout << "\n🎯 시뮬레이션 완료!" << endl;
        showStatus();
        
        cout << "\n📊 데이터 파일이 생성되었습니다:" << endl;
        cout << "- freezeCost.data: 일별 냉각 비용" << endl;
        cout << "- performanceRevenue.data: 일별 공연 수익" << endl;
    }
};

int main() {
    DataCenter dc;
    dc.runSimulation(24 * 10);  // 10일 시뮬레이션
    return 0;
}