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
            case 'C': return 60;   // 석탄발전소
            case 'F': return 10;    // 자동차 공장
            case 'A': return 2;    // 행정시설
            case 'H': return 0;    // 병원
            case 'T': return -7;  // 유원지(완화)
            case 'M': return -5;   // 목초지(완화)
            case 'E': return 2;    // 기타
            default: return 0;
        }
    }

    // 거리 계산
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

    // 미세먼지 확산 함수
    double getDustSpread(double baseDust, int distance, double decayRate = 0.15) {
        return baseDust * exp(-decayRate * distance);
    }

    // 미세먼지 완화량 계산 함수
    double getDustAbsorption(double baseAbsorption, double dustDensity, double intensity = 0.005, double spreadAmount = 0) {
        double rawAbsorption = baseAbsorption * (1.0 + intensity * dustDensity);
        double limitedAbsorption = std::min(rawAbsorption, spreadAmount * 0.8); // 확산량 대비 최대 80%
        return limitedAbsorption;
    }

    // TODO: 값 수정 필요
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
                temp = distance(i, j, k, l);

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
                temp = distance(i, j, k, l);

                if (minDistance > temp)
                    minDistance = temp;
            }
        }

        return minDistance;
    }

    // 미세먼지 확산
    // spreadDust 함수 일부 수정 예시
    void spreadDust() {
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) { 
                double dustIncrement = 0;

                if (cityGround[i][j] == 'C') {
                    // 발전소 가동률에 따라 미세먼지 배출량 조정 (coalPower에 이미 가동률 반영됨)
                    dustIncrement = 20.0 * coalPower / 300.0;
                    dustMap[i][j] += dustIncrement;
                } else {
                    // 거리 기반 확산량 계산 (발전소 가동률 비례)
                    double coalDist = getShortestDistanceCoalPowerFactory(i, j);
                    double carDist = getShortestDistanceCarFactory(i, j);
                    double adminDist = distance(i, j, 49, 49);

                    double spreadC = getDustSpread(getPollutionPower('C') * coalPower / 300.0, int(coalDist));
                    double spreadF = getDustSpread(getPollutionPower('F'), int(carDist));
                    double spreadA = getDustSpread(getPollutionPower('A'), int(adminDist));

                    dustIncrement = max({spreadC, spreadF, spreadA});
                    dustMap[i][j] += dustIncrement;
                }
            }
        }
    }

    // 자연적 미세먼지 완화
    void naturalMitigateDust() {
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                if (dustMap[i][j] > 200)
                    dustMap[i][j] = dustMap[i][j] * 0.65;
                else if (dustMap[i][j] > 150)
                    dustMap[i][j] = dustMap[i][j] * 0.75;
                else if (dustMap[i][j] > 100)
                    dustMap[i][j] = dustMap[i][j] * 0.8;
                else if (dustMap[i][j] > 50)
                    dustMap[i][j] = dustMap[i][j] * 0.97;
                else if (dustMap[i][j] > 25)
                    dustMap[i][j] = dustMap[i][j] * 0.99;              
            }
        }
    }

    // TODO: 값 수정 필요
    int getMitigationPower(char c) {
        switch(c) {
            case 'T': return 25;   // 유원지
            case 'M': return 10;    // 목초지
            default: return 0;
        }
    }

    // TODO: 값 수정 필요
    float getMigigationRate(float dustDensity) {
        if (dustDensity > 200)
            return 2;
        else if (dustDensity > 150)
            return 1.5;
        else if (dustDensity > 100)
            return 1;
        else if (dustDensity > 75)
            return 0.6;
        else if (dustDensity > 50)
            return 0.2;
        else if (dustDensity > 25)
            return 0.05;
        else
            return 0;
    }

    double getShortestDistanceTomb(int i, int j) {
        float minDistance = 10000;

        for (int k = 75; k <= 79; ++k) {
            for (int l = 19; l < 24; ++l) {
                if (minDistance == 1)
                    return minDistance;

                float temp;
                temp = distance(i, j, k, l);

                if (minDistance > temp)
                    minDistance = temp;
            }
        }

        return minDistance;
    }

    double getShortestDistanceGrass(int i, int j) {
        float minDistance = 10000;

        for (int k = 83; k <= 87; ++k) {
            for (int l = 83; l < 87; ++l) {
                if (minDistance == 1)
                    return minDistance;

                float temp;
                temp = distance(i, j, k, l);

                if (minDistance > temp)
                    minDistance = temp;
            }
        }

        return minDistance;
    }

    // 지역 미세먼지 완화
    void localMitigateDust() {
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                double dustDensity = dustMap[i][j];
                double reduction = 0;

                if (cityGround[i][j] == 'T') {
                    double base = getMitigationPower('T');
                    reduction = getDustAbsorption(base, dustDensity);
                } else if (cityGround[i][j] == 'M') {
                    double base = max(
                        static_cast<double>(getMitigationPower('T')) / (pow(getShortestDistanceTomb(i, j), 2) + 1),
                        static_cast<double>(getMitigationPower('M')) / (pow(getShortestDistanceGrass(i, j), 2) + 1)
                    );                    
                    reduction = getDustAbsorption(base, dustDensity);
                } else {
                    double base = max(
                        getMitigationPower('T') / (pow(getShortestDistanceTomb(i, j), 2) + 1),
                        getMitigationPower('M') / (pow(getShortestDistanceGrass(i, j), 2) + 1)
                    );
                    reduction = getDustAbsorption(base, dustDensity);
                }

                dustMap[i][j] -= reduction;
                if (dustMap[i][j] < 0) dustMap[i][j] = 0;
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
            for (int j = 83; j <= 87; ++j)
                cityGround[i][j] = 'M';
        }

        // 도시 전역의 미세먼지 지수 초기화
        initDustMap();

        // 평균 미세먼지 지수 초기화
        averageDustRate = getAverageDustRate();
    }

    // 메인 호출 부분
    // usingCity 함수 일부 출력 메시지 개선
    void usingCity() {
        int temp = coalPower;
        coalPower = 300;
        for (int i = 0; i < 365; ++i) {
            spreadDust();
            naturalMitigateDust();
            localMitigateDust();
        }

        coalPower = temp;

        averageDustRate = getAverageDustRate();

        cout << "초기 평균 미세먼지: " << averageDustRate << endl;
        if (coalPower == 300) {
            cout << "현재 발전소 100% 가동 중, 미세먼지 농도가 높고 거의 일정하게 유지됩니다." << endl;
        }

        int n;
        cout << "시뮬레이션을 몇 일 돌릴건가요? ";
        cin >> n;

        for (int i = 0; i < n; ++i) {
            spreadDust();
            naturalMitigateDust();
            localMitigateDust();
        }
        
        averageDustRate = getAverageDustRate();

        getCityDust();

        cout << "최종 평균 미세먼지: " << averageDustRate << endl;
        if (coalPower < 300) {
            cout << "발전소 가동률 감소로 미세먼지 농도가 일정 수준 이하로 줄어들었습니다." << endl;
        }
    }

};

int main() {
    int coalPower;
    cout << "발전소 활동률(100, 70, 30): ";
    cin >> coalPower;

    City callingVar(300 * coalPower / 100);
    callingVar.usingCity();

    return 0;
}