#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstdlib>
using namespace std;

class City {
private:
    // 도시 전역
    char cityGround[100][100];
    // 점수
    int score = 100;
    int coalPower = 300; // 단위는 MW
    float dustMap[100][100] = {20}; // 도시 전역의 미세먼지 지수 단위는 PM 2.5
    float averageDustRate;

    // 도시 전역 출력
    void getCityGround() {
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j)
                cout << cityGround[i][j];
            cout << endl;
        }
    }

    // 도시 전역의 미세먼지 농도 출력
    void getCityDust() {
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j)
                cout << dustMap[i][j] << " ";
            cout << endl;
        }
    }

    float getAverageDustRate() {
        float totalDust = 0;

        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j)
                totalDust += dustMap[i][j];
        }

        return totalDust / 10000;
    }

    // 각 건물별 미세먼지 영향량
    int getEmission(char c) {
        switch(c) {
            case 'C': return 40;   // 석탄발전소
            case 'F': return 7;    // 자동차 공장
            case 'A': return 1;    // 행정시설
            case 'H': return 0;    // 병원
            case 'T': return -10;  // 유원지(완화)
            case 'M': return -5;   // 목초지(완화)
            case 'E': return 2;    // 기타
            default: return 0;
        }
    }

    // 거리
    double distance(int x1, int y1, int x2, int y2) {
        return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    }

    // 미세먼지 농도 초기화
    void initDustMap() {
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                dustMap[i][j] = 20;
                dustMap[i][j] += getEmission(cityGround[i][j]);
                if (dustMap[i][j] < 0) dustMap[i][j] = 0;
            }
        }
        
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                int maxEmission = 0;
                for (int x = max(0, i - 5); x <= min(99, i + 5); ++x) {
                    for (int y = max(0, j - 5); y <= min(99, j + 5); ++y) {
                        int emission = getEmission(cityGround[x][y]);
                        if (emission > 0) {
                            double dist = distance(i, j, x, y);
                            if (dist <= 5) {
                                // 거리 가중치 적용 (가까울수록 영향 큼)
                                int weightedEmission = (int)(emission * (5 - dist) / 5);
                                if (weightedEmission > maxEmission) {
                                    maxEmission = weightedEmission;
                                }
                            }
                        }
                    }
                }
                dustMap[i][j] += maxEmission;
            }
        }

        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                int maxReduction = 0;
                for (int x = max(0, i - 5); x <= min(99, i + 5); ++x) {
                    for (int y = max(0, j - 5); y <= min(99, j + 5); ++y) {
                        int emission = getEmission(cityGround[x][y]);
                        if (emission < 0) {
                            double dist = distance(i, j, x, y);
                            if (dist <= 5) {
                                int weightedReduction = (int)(-emission * (5 - dist) / 5);
                                if (weightedReduction > maxReduction) {
                                    maxReduction = weightedReduction;
                                }
                            }
                        }
                    }
                }
                dustMap[i][j] -= maxReduction;
                if (dustMap[i][j] < 0) dustMap[i][j] = 0;
            }
        }
    }



    int getPollutionPower(char c) {
        switch(c) {
            case 'C': return 40;   // 석탄발전소
            case 'F': return 7;    // 자동차 공장
            case 'A': return 1;    // 행정시설
            default: return 0;
        }
    }

    double getShortestDistanceCoalPowerFactory(int i, int j) {
        float minDistance = 10000;

        for (int k = 88; k < 100; ++k) {
            for (int l = 88; l < 100; ++l) {
                if (minDistance == 1)
                    return minDistance;

                float temp;
                temp = sqrt(abs(i - k) * abs(i - k) + abs(j - l) * abs(j - l));

                if (minDistance > temp)
                    minDistance = temp;
            }
        }

        return minDistance;
    }

    double getShortestDistanceCarFactory(int i, int j) {
        float minDistance = 10000;

        for (int k = 78; k <= 81; ++k) {
            for (int l = 95; l < 100; ++l) {
                if (minDistance == 1)
                    return minDistance;

                float temp;
                temp = sqrt(abs(i - k) * abs(i - k) + abs(j - l) * abs(j - l));

                if (minDistance > temp)
                    minDistance = temp;
            }
        }

        return minDistance;
    }

    // 미세먼지 확산
    void spreadDust() {
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                if (cityGround[i][j] == 'C') {
                    float temp;
                    temp = max(getPollutionPower('F') / (getShortestDistanceCarFactory(i, j) * getShortestDistanceCarFactory(i, j) + 1),
                    getPollutionPower('A') / (sqrt(abs(i - 49) * abs(i - 49) + abs(j - 49) * abs(j - 49)) + 1));
                } else if (cityGround[i][j] == 'F') {

                }
            }
        }
    }

public:
    // 생성자
    City(int coalPower) : coalPower(coalPower) {
        // 도시 전역의 건물 초기화
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j)
                cityGround[i][j] = 'E';
        }

        cityGround[30][30] = 'H';
        cityGround[49][49] = 'A';
        for (int i = 75; i <= 79; ++i) {
            for (int j = 19; j <= 24; ++j)
                cityGround[i][j] = 'T';
        }
        for (int i = 78; i <= 81; ++i) {
            for (int j = 95; j <= 99; ++j)
                cityGround[i][j] = 'F';
        }
        for (int i = 88; i <= 99; ++i) {
            for (int j = 88; j <= 99; ++j)
                cityGround[i][j] = 'C';
        }
        for (int i = 83; i <= 87; ++i) {
            for (int j = 83; j <= 87; ++j) {
                cityGround[i][j] = 'M';
            }
        }

        // 도시 전역의 미세먼지 지수 초기화
        initDustMap();

        // 평균 미세먼지 지수 초기화
        averageDustRate = getAverageDustRate();
    }

    // 메인 호출 부분
    void usingCity() {
        cout << averageDustRate << endl;
    }

};

int main() {
    int coalPower;
    cout << "발전소 활동률(100, 70, 30): ";
    cin >> coalPower;

    City callingVar = City(300 * coalPower / 100);
    callingVar.usingCity();
}