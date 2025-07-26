#include <iostream>
#include <vector>
#include <utility>
#include <random>
using namespace std;

// 미래형 데이터 센터 구현을 위한 클래스
class DataCenter {
private:
    // 각 칸에 따른 건물의 종류 저장
    // G: Grass, N: Node, C: Charging Point, B: Building, S: Street
    char ground[100][100];
    
    // 8개 구역별 충전소 정보 (각 구역당 30개)
    vector<vector<pair<int,int>>> chargingStations; // 8개 구역, 각 구역당 30개 충전소 좌표
    vector<vector<bool>> hasNodeCluster;           // 8개 구역, 각 구역당 30개 중 10개만 true
    
    // 8개 구역의 경계 정의
    struct Region {
        int startI, endI, startJ, endJ;
    };
    
    vector<Region> regions = {
        {0, 29, 0, 29},     // 좌상단
        {0, 29, 35, 64},    // 상단 중앙  
        {0, 29, 70, 99},    // 우상단
        {35, 64, 0, 29},    // 좌측 중앙
        {35, 64, 70, 99},   // 우측 중앙
        {70, 99, 0, 29},    // 좌하단
        {70, 99, 35, 64},   // 하단 중앙
        {70, 99, 70, 99}    // 우하단
    };
    
    void initGround() {
        // ground 초기화 (모든 곳을 풀밭으로)
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j)
                ground[i][j] = 'G';
        }
        
        // 중앙 십자 도로 설정
        for (int i = 35; i <= 65; ++i) {
            for (int j = 35; j < 65; ++j)
                ground[i][j] = 'S';
        }
        
        // 중앙 건물 설정 (20*20)
        for (int i = 40; i < 60; ++i) {
            for (int j = 40; j < 60; ++j)
                ground[i][j] = 'B';
        }
        
        // 세로/가로 도로 설정
        for (int i = 0; i < 100; ++i) {
            for (int j = 30; j < 35; ++j) {
                ground[i][j] = 'S';
                ground[j][i] = 'S';
            }
            for (int j = 65; j < 70; ++j) {
                ground[i][j] = 'S';
                ground[j][i] = 'S';
            }
        }
    }
    
    void setupChargingStations() {
        chargingStations.resize(8);
        hasNodeCluster.resize(8);
        
        random_device rd;
        mt19937 gen(rd());
        
        // 각 구역별로 충전소 30개씩 배치
        for (int regionIdx = 0; regionIdx < 8; ++regionIdx) {
            Region& region = regions[regionIdx];
            chargingStations[regionIdx].clear();
            hasNodeCluster[regionIdx].resize(30, false);
            
            // 해당 구역의 풀밭 좌표들 수집
            vector<pair<int,int>> grassPositions;
            for (int i = region.startI; i <= region.endI; ++i) {
                for (int j = region.startJ; j <= region.endJ; ++j) {
                    if (ground[i][j] == 'G') {
                        grassPositions.push_back({i, j});
                    }
                }
            }
            
            // 풀밭 위치들을 섞어서 30개 선택
            shuffle(grassPositions.begin(), grassPositions.end(), gen);
            
            // 30개 충전소 배치
            for (int i = 0; i < min(30, (int)grassPositions.size()); ++i) {
                int x = grassPositions[i].first;
                int y = grassPositions[i].second;
                
                ground[x][y] = 'C';
                chargingStations[regionIdx].push_back({x, y});
            }
            
            // 30개 중 랜덤하게 10개에 노드 군집 배치
            vector<int> indices(30);
            for (int i = 0; i < 30; ++i) indices[i] = i;
            shuffle(indices.begin(), indices.end(), gen);
            
            for (int i = 0; i < 10; ++i) {
                hasNodeCluster[regionIdx][indices[i]] = true;
            }
        }
    }
    
public:
    DataCenter() {
        initGround();
        setupChargingStations();
    }
    
    void printRegionInfo() {
        cout << "=== 데이터센터 구역별 충전소 정보 ===" << endl;
        for (int regionIdx = 0; regionIdx < 8; ++regionIdx) {
            cout << "\n구역 " << (regionIdx + 1) << ":" << endl;
            cout << "충전소 개수: " << chargingStations[regionIdx].size() << endl;
            
            int nodeCount = 0;
            for (int i = 0; i < hasNodeCluster[regionIdx].size(); ++i) {
                if (hasNodeCluster[regionIdx][i]) nodeCount++;
            }
            cout << "노드 군집이 있는 충전소: " << nodeCount << "개" << endl;
            
            cout << "노드가 있는 충전소 좌표: ";
            for (int i = 0; i < chargingStations[regionIdx].size(); ++i) {
                if (hasNodeCluster[regionIdx][i]) {
                    cout << "(" << chargingStations[regionIdx][i].first 
                         << "," << chargingStations[regionIdx][i].second << ") ";
                }
            }
            cout << endl;
        }
    }
    
    void getGround() {
        // 모든 충전소에 대해 노드 존재 여부를 순서대로 출력
        for (int regionIdx = 0; regionIdx < 8; ++regionIdx) {
            for (int i = 0; i < hasNodeCluster[regionIdx].size(); ++i) {
                cout << hasNodeCluster[regionIdx][i] << " ";
            }
        }
        cout << endl;
    }
    
    // 전체 맵 출력 (디버깅용)
    void printMap() {
        cout << "\n=== 데이터센터 맵 ===" << endl;
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                cout << ground[i][j];
            }
            cout << endl;
        }
    }
};

int main() {
    DataCenter dataCenter;
    dataCenter.printRegionInfo();
    cout << "\n노드 존재 여부 (240개 충전소): ";
    dataCenter.getGround();
    
    return 0;
}