#include <iostream>
#include <cmath>
using namespace std;

// 미래형 데이터 센터 구현을 위한 클래스
class DataCenter {
private:
    // 각 칸에 따른 건물의 종류 저장
    char ground[100][100];
    // G: Grass, N: Node, C: Charging Point, B: Building, S: Street

    void initGround() {
        // ground 초기화
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j)
                ground[i][j] = 'G';
        }

        for (int i = 35; i <= 65; ++i) {
            for (int j = 35; j < 65; ++j)
                ground[i][j] = 'S';
        }

        // 20*20
        for (int i = 40; i < 60; ++i) {
            for (int j = 40; j < 60; ++j)
                ground[i][j] = 'B';
        }

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

public:
    DataCenter() {
        initGround();
    }

    void getGround() {
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j)
                cout << ground[i][j];
            cout << endl;
        }
    }

};

int main() {
    DataCenter dataCenter;
    dataCenter.getGround();
}