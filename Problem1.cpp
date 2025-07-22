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
    float dustMap[100][100]; // 도시 전역의 미세먼지 지수 단위는 PM 2.5
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

    // 각 건물별 미세먼지 영향량 (기본값) - 초기 농도 32 맞추기 위해 조정
    float getBaseEmission(char c) {
        switch(c) {
            case 'C': return 10.0f;    // 석탄발전소 (10.0 → 8.0으로 감소)
            case 'F': return 3.0f;    // 자동차 공장 (2.0 → 1.5로 감소)
            case 'A': return 0.5f;    // 행정시설 (0.5 → 0.3으로 감소)
            case 'H': return 0.1f;    // 병원
            case 'T': return -1.0f;   // 유원지(완화) (-1.0 → -1.2로 완화 효과 증가)
            case 'M': return -0.5f;   // 목초지(완화) (-0.8 → -1.0으로 완화 효과 증가)
            case 'E': return 0.3f;    // 기타 (0.2 → 0.1로 감소)
            default: return 0.0f;
        }
    }

    // 거리 계산
    double distance(int x1, int y1, int x2, int y2) {
        return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    }

    // 미세먼지 농도 초기화 (초기 평균값 32 정도로 조정)
    void initDustMap() {
        // 기본 배경 미세먼지 농도를 32로 설정
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                dustMap[i][j] = 32.0f;
            }
        }
        
        // 직접 영향 계산
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                float directEmission = getBaseEmission(cityGround[i][j]);
                dustMap[i][j] += directEmission;
                if (dustMap[i][j] < 0) dustMap[i][j] = 0;
            }
        }
        
        // 주변 영향 계산 (오염원) - 영향 범위와 강도 조정
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                float maxAddition = 0;
                for (int x = max(0, i - 8); x <= min(99, i + 8); ++x) { // 범위 8 → 6으로 감소
                    for (int y = max(0, j - 8); y <= min(99, j + 8); ++y) {
                        float emission = getBaseEmission(cityGround[x][y]);
                        if (emission > 0) {
                            double dist = distance(i, j, x, y);
                            if (dist <= 8 && dist > 0) { // 범위 8 → 6으로 감소
                                float weightedEmission = emission * (8 - dist) * 1.2; // 강도 20% 감소
                                if (weightedEmission > maxAddition) {
                                    maxAddition = weightedEmission;
                                }
                            }
                        }
                    }
                }
                dustMap[i][j] += maxAddition;
            }
        }

        // 주변 영향 계산 (완화원) - 완화 효과 증가
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                float maxReduction = 0;
                for (int x = max(0, i - 4); x <= min(99, i + 4); ++x) { // 범위 6 → 7로 증가
                    for (int y = max(0, j - 4); y <= min(99, j + 4); ++y) {
                        float emission = getBaseEmission(cityGround[x][y]);
                        if (emission < 0) {
                            double dist = distance(i, j, x, y);
                            if (dist <= 4 && dist > 0) { // 범위 6 → 7로 증가
                                float weightedReduction = -emission * (4 - dist) * 0.6f; // 강도 20% 증가
                                if (weightedReduction > maxReduction) {
                                    maxReduction = weightedReduction;
                                }
                            }
                        }
                    }
                }
                cout << "(" << i << ", " << j << "): " << maxReduction << endl;
                dustMap[i][j] -= maxReduction;
                if (dustMap[i][j] < 0) dustMap[i][j] = 0;
            }
        }
    }

    // 미세먼지 확산 함수
    double getDustSpread(double baseDust, int distance, double decayRate = 0.2) {
        return baseDust * exp(-decayRate * distance);
    }

    // 미세먼지 완화량 계산 함수
    double getDustAbsorption(double baseAbsorption, double dustDensity, double intensity = 0.01) {
        // 미세먼지 농도가 높을수록 완화 효과 증가
        double efficiencyMultiplier = 1.0 + intensity * dustDensity;
        return baseAbsorption * efficiencyMultiplier;
    }

    // 발전소 오염력 (가동률 반영)
    double getPollutionPower(char c) {
        switch(c) {
            case 'C': return 6.0 * (coalPower / 300.0);   // 석탄발전소 (가동률 비례, 증가)
            case 'F': return 1.2;    // 자동차 공장
            case 'A': return 0.15;    // 행정시설
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

    // 월별 미세먼지 확산 (30일 기준)
    void monthlySpreadDust() {
        float tempDustMap[100][100];
        
        // 기존 dustMap 복사
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                tempDustMap[i][j] = dustMap[i][j];
            }
        }
        
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) { 
                double dustIncrement = 0;
                double currentDust = tempDustMap[i][j];

                if (cityGround[i][j] == 'C') {
                    // 발전소에서 직접 배출 (현재 농도에 따라 조정)
                    double baseEmission = 2.0 * (coalPower / 300.0);
                    dustIncrement = baseEmission * (1.0 + 0.0175 * currentDust);
                    dustMap[i][j] += dustIncrement;
                } else {
                    // 거리 기반 확산량 계산
                    double coalDist = getShortestDistanceCoalPowerFactory(i, j);
                    double carDist = getShortestDistanceCarFactory(i, j);
                    double adminDist = distance(i, j, 49, 49);

                    double spreadC = getDustSpread(getPollutionPower('C'), int(coalDist));
                    double spreadF = getDustSpread(getPollutionPower('F'), int(carDist));
                    double spreadA = getDustSpread(getPollutionPower('A'), int(adminDist));

                    dustIncrement = max({spreadC, spreadF, spreadA});
                    
                    // 현재 농도에 따른 확산 효율 조정
                    if (currentDust > 46) {
                        dustIncrement *= 0.85; // 높은 농도에서는 확산 효율 감소
                    } else if (currentDust > 25) {
                        dustIncrement *= 1;
                    } else {
                        dustIncrement *= 1.5;
                    }
                    
                    dustMap[i][j] += dustIncrement;
                }
            }
        }
    }

    // 월별 자연적 미세먼지 완화 (현재 농도 고려)
    void monthlyNaturalMitigateDust() {
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                float currentDust = dustMap[i][j];
                
                // 농도별 자연 감소율 (월 단위)
                if (currentDust > 56) {
                    dustMap[i][j] *= 0.98; // 15% 감소
                } else if (currentDust > 46) {
                    dustMap[i][j] *= 0.985; // 8% 감소
                } else if (currentDust > 36) {
                    dustMap[i][j] *= 0.987; // 4% 감소
                } else if (currentDust > 31) {
                    dustMap[i][j] *= 0.99; // 2% 감소
                } else {
                    dustMap[i][j] *= 0.995; // 0.5% 감소
                }              
            }
        }
    }

    // 완화 시설 효과
    double getMitigationPower(char c) {
        switch(c) {
            case 'T': return 1.5;   // 유원지
            case 'M': return 0.5;    // 목초지
            default: return 0;
        }
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

    // 월별 지역 미세먼지 완화 (현재 농도 고려)
    void monthlyLocalMitigateDust() {
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                double dustDensity = dustMap[i][j];
                double reduction = 0;

                if (cityGround[i][j] == 'T') {
                    double base = getMitigationPower('T');
                    reduction = getDustAbsorption(base, dustDensity, 0.007);
                } else if (cityGround[i][j] == 'M') {
                    double tombEffect = getMitigationPower('T') / (pow(getShortestDistanceTomb(i, j), 2) + 1);
                    double grassEffect = getMitigationPower('M') / (pow(getShortestDistanceGrass(i, j), 2) + 1);
                    double base = max(tombEffect, grassEffect);                    
                    reduction = getDustAbsorption(base, dustDensity, 0.005);
                } else {
                    double tombEffect = getMitigationPower('T') / (pow(getShortestDistanceTomb(i, j), 2) + 1);
                    double grassEffect = getMitigationPower('M') / (pow(getShortestDistanceGrass(i, j), 2) + 1);
                    double base = max(tombEffect, grassEffect);
                    reduction = getDustAbsorption(base, dustDensity, 0.003);
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

    // 메인 호출 부분 (월 단위 시뮬레이션)
    void usingCity() {
        // 초기 상태는 100% 가동으로 1년(12개월) 시뮬레이션
        int initialCoalPower = coalPower;
        coalPower = 300; // 100% 가동
        
        cout << "=== 초기 상태 (100% 가동 1년 시뮬레이션) ===" << endl;
        for (int month = 0; month < 12; ++month) {
            monthlySpreadDust();
            monthlyNaturalMitigateDust();
            monthlyLocalMitigateDust();
        }

        averageDustRate = getAverageDustRate();
        cout << "초기 평균 미세먼지 (100% 1년 후): " << averageDustRate << endl;
        
        // 사용자가 설정한 가동률로 변경
        coalPower = initialCoalPower;
        
        cout << "\n=== 현재 설정 ===" << endl;
        cout << "발전소 가동률: " << (coalPower * 100 / 300) << "%" << endl;
        
        int months;
        cout << "시뮬레이션을 몇 개월 돌릴건가요? ";
        cin >> months;

        cout << "\n=== 시뮬레이션 진행 ===" << endl;
        for (int month = 0; month < months; ++month) {
            monthlySpreadDust();
            monthlyNaturalMitigateDust();
            monthlyLocalMitigateDust();
            
            if ((month + 1) % 3 == 0) { // 3개월마다 중간 결과 출력
                float currentAvg = getAverageDustRate();
                cout << (month + 1) << "개월 후 평균 미세먼지: " << currentAvg << endl;
            }
        }
        
        averageDustRate = getAverageDustRate();

        cout << "\n=== 최종 결과 ===" << endl;
        cout << "최종 평균 미세먼지: " << averageDustRate << endl;
        
        if (coalPower == 300) {
            cout << "발전소 100% 가동 - 미세먼지 농도가 높게 유지됩니다. (목표: 34-36)" << endl;
        } else if (coalPower == 210) {
            cout << "발전소 70% 가동 - 미세먼지 농도가 점진적으로 개선되고 있습니다. (목표: 32-34)" << endl;
        } else if (coalPower == 90) {
            cout << "발전소 30% 가동 - 미세먼지 농도가 크게 개선되었습니다. (목표: 28-31)" << endl;
        }
        
        char showMap;
        cout << "미세먼지 농도 지도를 출력할까요? (y/n): ";
        cin >> showMap;
        if (showMap == 'y' || showMap == 'Y') {
            getCityDust();
        }
    }
};

int main() {
    int coalPower;
    cout << "발전소 활동률을 선택하세요:" << endl;
    cout << "1. 100% (300MW)" << endl;
    cout << "2. 70% (210MW)" << endl;
    cout << "3. 30% (90MW)" << endl;
    cout << "선택 (1-3): ";
    
    int choice;
    cin >> choice;
    
    switch(choice) {
        case 1: coalPower = 300; break;
        case 2: coalPower = 210; break;
        case 3: coalPower = 90; break;
        default: 
            cout << "잘못된 입력입니다. 기본값 100%로 설정합니다." << endl;
            coalPower = 300;
            break;
    }

    City callingVar(coalPower);
    callingVar.usingCity();

    return 0;
}