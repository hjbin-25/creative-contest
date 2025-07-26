#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
using namespace std;

class DataCenter {
private:
    char ground[100][100];  // 땅 모양
    
    // 노드 정보를 배열로 저장
    int nodeX[240];      // 노드 x좌표
    int nodeY[240];      // 노드 y좌표
    double nodeBattery[240]; // 노드 전력 (소수점 지원)
    double nodeTemp[240];    // 노드 온도 (소수점 지원)
    bool isCharging[240];    // 충전 중인지
    bool isCooling[240];     // 냉각 중인지
    int chargingTime[240];   // 충전 시간 (시간)
    int coolingTime[240];    // 냉각 시간 (시간)
    int totalNodes;          // 실제 노드 개수
    
    // 충전소 정보를 배열로 저장
    int chargingX[240];   // 충전소 x좌표
    int chargingY[240];   // 충전소 y좌표
    bool hasNode[240];    // 충전소에 노드가 있는지
    bool isOccupied[240]; // 충전소가 사용 중인지
    int totalCharging;    // 실제 충전소 개수
    
    // 시뮬레이션 변수
    int currentHour;      // 현재 시간
    double totalCost;     // 총 비용 (원)
    double totalPower;    // 총 전력 사용량 (kWh)
    
    void makeGround() {
        // 1. 모든 곳을 풀밭으로
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 100; j++) {
                ground[i][j] = 'G';
            }
        }
        
        // 2. 십자 도로 만들기
        for (int i = 35; i <= 65; i++) {
            for (int j = 35; j < 65; j++) {
                ground[i][j] = 'S';  // 도로
            }
        }
        
        // 3. 건물 만들기 (20x20)
        for (int i = 40; i < 60; i++) {
            for (int j = 40; j < 60; j++) {
                ground[i][j] = 'B';  // 건물
            }
        }
        
        // 4. 세로/가로 도로
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
        
        // 8개 구역의 경계
        int regions[8][4] = {
            {0, 29, 0, 29},     // 좌상단
            {0, 29, 35, 64},    // 상단
            {0, 29, 70, 99},    // 우상단
            {35, 64, 0, 29},    // 좌측
            {35, 64, 70, 99},   // 우측
            {70, 99, 0, 29},    // 좌하단
            {70, 99, 35, 64},   // 하단
            {70, 99, 70, 99}    // 우하단
        };
        
        // 각 구역별로 충전소 30개, 노드 10개 만들기
        for (int region = 0; region < 8; region++) {
            int startI = regions[region][0];
            int endI = regions[region][1];
            int startJ = regions[region][2];
            int endJ = regions[region][3];
            
            int chargingCount = 0;
            int nodeCount = 0;
            
            // 해당 구역에 충전소 30개 배치
            for (int i = startI; i <= endI && chargingCount < 30; i += 3) {
                for (int j = startJ; j <= endJ && chargingCount < 30; j += 4) {
                    if (ground[i][j] == 'G') {
                        ground[i][j] = 'C';  // 충전소
                        
                        // 충전소 정보 저장
                        chargingX[totalCharging] = i;
                        chargingY[totalCharging] = j;
                        isOccupied[totalCharging] = false;
                        
                        // 처음 10개 충전소에는 노드 배치
                        if (nodeCount < 10) {
                            hasNode[totalCharging] = true;
                            
                            nodeX[totalNodes] = i;
                            nodeY[totalNodes] = j;
                            nodeBattery[totalNodes] = 100.0;  // 100% 전력
                            nodeTemp[totalNodes] = 60.0;      // 60도
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
    
    // 두 점 사이의 거리 계산
    double getDistance(int x1, int y1, int x2, int y2) {
        return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
    }
    
    // 가장 가까운 빈 충전소 찾기
    int findNearestCharging(int nodeIdx) {
        double minDist = 999999;
        int nearestIdx = -1;
        
        for (int i = 0; i < totalCharging; i++) {
            if (!isOccupied[i]) {
                double dist = getDistance(nodeX[nodeIdx], nodeY[nodeIdx], 
                                        chargingX[i], chargingY[i]);
                if (dist < minDist) {
                    minDist = dist;
                    nearestIdx = i;
                }
            }
        }
        return nearestIdx;
    }
    
    // 노드 이동 (G만 가능)
    void moveNode(int nodeIdx) {
        if (isCharging[nodeIdx] || isCooling[nodeIdx]) return;
        
        int dx[] = {-2, -2, -2, -1, -1, -1, 0, 0, 1, 1, 1, 2, 2, 2};
        int dy[] = {-2, -1, 0, -2, -1, 0, -2, 2, -2, -1, 0, -2, -1, 0};
        
        for (int tries = 0; tries < 50; tries++) {
            int dir = rand() % 14;
            int newX = nodeX[nodeIdx] + dx[dir];
            int newY = nodeY[nodeIdx] + dy[dir];
            
            if (newX >= 0 && newX < 100 && newY >= 0 && newY < 100) {
                if (ground[newX][newY] == 'G') {
                    nodeX[nodeIdx] = newX;
                    nodeY[nodeIdx] = newY;
                    break;
                }
            }
        }
    }
    
    // 온도 계산 (간단한 상승 모델)
    double calculateTemperature(double T0) {
        // 2-5도 사이에서 랜덤하게 상승
        double tempRise = 2.0 + (rand() % 4); // 2, 3, 4, 5도 중 랜덤
        return T0 + tempRise;
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
        
        for (int i = 0; i < totalNodes; i++) {
            // 1. 배터리 소모 (4.17%) - 충전/냉각 중이 아닐 때만
            if (!isCharging[i] && !isCooling[i]) {
                nodeBattery[i] -= 4.17;
                if (nodeBattery[i] < 0) nodeBattery[i] = 0;
            }
            
            // 2. 온도 상승 - 냉각 중이 아닐 때만
            if (!isCooling[i]) {
                nodeTemp[i] = calculateTemperature(nodeTemp[i]);
            }
            
            // 3. 냉각 처리 (1시간 소요)
            if (isCooling[i]) {
                coolingTime[i]++;
                if (coolingTime[i] >= 1) {  // 1시간 완료
                    nodeTemp[i] = 40.0;  // 40도로 냉각 완료
                    isCooling[i] = false;
                    coolingTime[i] = 0;
                    // 충전소에서 나가기
                    for (int j = 0; j < totalCharging; j++) {
                        if (chargingX[j] == nodeX[i] && chargingY[j] == nodeY[i]) {
                            isOccupied[j] = false;
                            break;
                        }
                    }
                }
            }
            
            // 4. 충전 처리 (1시간 소요)
            if (isCharging[i]) {
                chargingTime[i]++;
                if (chargingTime[i] >= 1) {  // 1시간 완료
                    nodeBattery[i] = 100.0;  // 100%로 충전 완료
                    isCharging[i] = false;
                    chargingTime[i] = 0;
                    // 충전소에서 나가기
                    for (int j = 0; j < totalCharging; j++) {
                        if (chargingX[j] == nodeX[i] && chargingY[j] == nodeY[i]) {
                            isOccupied[j] = false;
                            break;
                        }
                    }
                }
            }
            
            // 5. 온도가 70도 이상이면 냉각하러 가기
            if (nodeTemp[i] >= 70 && !isCooling[i] && !isCharging[i]) {
                int chargingIdx = findNearestCharging(i);
                if (chargingIdx != -1) {
                    nodeX[i] = chargingX[chargingIdx];
                    nodeY[i] = chargingY[chargingIdx];
                    isOccupied[chargingIdx] = true;
                    isCooling[i] = true;
                    coolingTime[i] = 0;  // 냉각 시간 초기화
                    totalCost += 625;     // 625원 추가
                    totalPower += 4.167;  // 4.167kWh 추가
                }
            }
            
            // 6. 배터리가 30% 이하면 충전하러 가기
            else if (nodeBattery[i] <= 30 && !isCharging[i] && !isCooling[i]) {
                int chargingIdx = findNearestCharging(i);
                if (chargingIdx != -1) {
                    nodeX[i] = chargingX[chargingIdx];
                    nodeY[i] = chargingY[chargingIdx];
                    isOccupied[chargingIdx] = true;
                    isCharging[i] = true;
                    chargingTime[i] = 0;  // 충전 시간 초기화
                }
            }
            
            // 7. 정상 상태면 이동
            else if (!isCharging[i] && !isCooling[i]) {
                moveNode(i);
            }
        }
        
        // 24시간마다 일일 전력 비용 추가
        if (currentHour % 24 == 0) {
            totalCost += 18000000;    // 1800만원
            totalPower += 120000;     // 12만kWh
        }
    }
    
    void showStatus() {
        cout << "\n=== " << currentHour << "시간 후 상태 ===" << endl;
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
        cout << "평균 배터리: " << avgBattery/totalNodes << "%" << endl;
        cout << "평균 온도: " << avgTemp/totalNodes << "도" << endl;
        
        // 상세 현황
        cout << "\n[상세 현황]" << endl;
        cout << "- 총 노드: " << totalNodes << "개" << endl;
        cout << "- 충전소 사용률: " << (charging + cooling) << "/" << totalCharging 
             << " (" << ((double)(charging + cooling) / totalCharging * 100) << "%)" << endl;
    }
    
    void runSimulation(int hours) {
        cout << "시뮬레이션 시작!" << endl;
        showStatus();
        
        for (int h = 1; h <= hours; h++) {
            simulateOneHour();
            if (h % 1 == 0) {  // 6시간마다 상태 출력
                showStatus();
            }
        }
        
        cout << "\n=== 최종 결과 ===" << endl;
        showStatus();
    }
};

int main() {
    DataCenter dc;
    dc.runSimulation(72);  // 72시간 (3일) 시뮬레이션
    
    return 0;
}