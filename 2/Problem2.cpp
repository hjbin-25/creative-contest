#include <iostream>
#include <vector>
using namespace std;

class DataCenter {
private:
    char ground[100][100];  // 땅 모양
    
    // 노드 정보를 배열로 저장
    int nodeX[240];      // 노드 x좌표
    int nodeY[240];      // 노드 y좌표
    int nodeBattery[240]; // 노드 전력
    int nodeTemp[240];    // 노드 온도
    int totalNodes;       // 실제 노드 개수
    
    // 충전소 정보를 배열로 저장
    int chargingX[240];   // 충전소 x좌표
    int chargingY[240];   // 충전소 y좌표
    bool hasNode[240];    // 충전소에 노드가 있는지
    int totalCharging;    // 실제 충전소 개수
    
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
                        
                        // 처음 10개 충전소에는 노드 배치
                        if (nodeCount < 10) {
                            hasNode[totalCharging] = true;
                            
                            nodeX[totalNodes] = i;
                            nodeY[totalNodes] = j;
                            nodeBattery[totalNodes] = 100;  // 100% 전력
                            nodeTemp[totalNodes] = 17;      // 17도
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
    
public:
    DataCenter() {
        makeGround();
        makeNodes();
    }
    
    void showNodes() {
        cout << "총 노드 개수: " << totalNodes << endl;
        for (int i = 0; i < totalNodes; i++) {
            cout << "노드 " << i+1 << ": ";
            cout << "위치(" << nodeX[i] << "," << nodeY[i] << ") ";
            cout << "전력:" << nodeBattery[i] << "% ";
            cout << "온도:" << nodeTemp[i] << "도" << endl;
        }
    }
    
    void changeNode(int nodeNum, int newBattery, int newTemp) {
        if (nodeNum >= 0 && nodeNum < totalNodes) {
            nodeBattery[nodeNum] = newBattery;
            nodeTemp[nodeNum] = newTemp;
            cout << "노드 " << nodeNum+1 << " 업데이트!" << endl;
        }
    }
    
    void showMap() {
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 100; j++) {
                cout << ground[i][j];
            }
            cout << endl;
        }
    }
    
    void showCharging() {
        cout << "총 충전소 개수: " << totalCharging << endl;
        for (int i = 0; i < totalCharging; i++) {
            cout << "충전소 " << i+1 << ": ";
            cout << "위치(" << chargingX[i] << "," << chargingY[i] << ") ";
            if (hasNode[i]) {
                cout << "노드 있음";
            } else {
                cout << "노드 없음";
            }
            cout << endl;
        }
    }
    
    int getChargingCount() {
        return totalCharging;
    }
};

int main() {
    DataCenter dc;
    
    cout << "충전소 정보:" << endl;
    dc.showCharging();
    
    cout << "\n노드 정보:" << endl;
    dc.showNodes();
    
    cout << "\n노드 1번 수정:" << endl;
    dc.changeNode(0, 85, 22);
    
    return 0;
}