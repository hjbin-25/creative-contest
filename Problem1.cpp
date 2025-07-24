#include <iostream>
#include <cmath> 
#include <fstream> 
using namespace std;

class City {
private:
    // 도시 전역
    char cityGround[100][100];
    // 도시 전역의 미세먼지 지수, 단위는 PM2.5
    float dustMap[100][100];

    // 점수: 실행할 대안을 이 점수 기반으로 평가
    float score = 0;
    // 시설에 대한 점수의 합
    float facilityScore = 0;
    // 시민의 미세먼지 관련 만족도 점수
    float dustScore = 0;

    // 하루에 생산 시킬 전력, 단위는 MW, 100%->300, 70%->210, 30%->90
    int coalPower = 300;

    // 도시 전역의 평균 미세먼지 지수, 단위는 PM2.5
    float averageDustRate;

    // 도시 전역의 누적 입원자 수
    int totalHospitalizationPeople = 0;
    // 도시 전역의 1달간 입원자 수
    int monthHospitalizationPeople = 0;
    // 도시 전역의 누적 사망자 수
    int totalDeathPeople = 0;
    // 도시 전역의 1달간 사망자 수
    int monthDeathPeople = 0;

    // 도시 전역 출력, 데이터 확인
    void getCityGround() {
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j)
                cout << cityGround[i][j];
            cout << endl;
        }
    }

    // 도시 전역의 미세먼지 농도 출력, 데이터 확인
    void getCityDust() {
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j)
                cout << dustMap[i][j] << " ";
            cout << endl;
        }
    }

    // 평균 미세먼지 지수 반환
    float getAverageDustRate() {
        float totalDust = 0;
        for (int i = 0; i < 100; ++i)
            for (int j = 0; j < 100; ++j)
                totalDust += dustMap[i][j];
        return totalDust / 10000;
    }

    // 각 건물별 미세먼지 영향량
    float getBaseEmission(char c) {
        switch (c) {
        case 'C': return 10.0f;    // 석탄발전소
        case 'F': return 3.0f;     // 자동차 공장
        case 'A': return 0.5f;     // 행정시설
        case 'H': return 0.1f;     // 병원
        case 'T': return -1.0f;    // 유원지
        case 'M': return -0.5f;    // 목초지
        case 'E': return 0.3f;     // 기타
        default: return 0.0f;
        }
    }

    // 거리 계산
    double distance(int x1, int y1, int x2, int y2) {
        // 대각선 길이
        return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    }

    // 미세먼지 농도 초기화
    void initDustMap() {
        // 기본 미세먼지 농도를 32로 설정
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                dustMap[i][j] = 32.0f;
            }
        }

        // 직접 영향 계산
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                float directEmission = getBaseEmission(cityGround[i][j]);
                // 해당 칸의 영향부터 계산 후 시작
                dustMap[i][j] += directEmission;
                // 음수 방지
                if (dustMap[i][j] < 0)
                    dustMap[i][j] = 0;
            }
        }

        // 주변 영향 계산
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                float maxAddition = -1;

                for (int x = max(0, i - 8); x <= min(99, i + 8); ++x) {
                    for (int y = max(0, j - 8); y <= min(99, j + 8); ++y) {
                        float emission = getBaseEmission(cityGround[x][y]);

                        // 예외 조건 방지
                        if (emission > 0) {
                            // 거리 부분 최적화
                            double dist = distance(i, j, x, y);

                            if (dist <= 8 && dist > 0) {
                                float weightedEmission = emission * (8 - dist) * 1.2;

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

        // 주변 영향 계산
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                float maxReduction = -1;

                for (int x = max(0, i - 4); x <= min(99, i + 4); ++x) {
                    for (int y = max(0, j - 4); y <= min(99, j + 4); ++y) {
                        float emission = getBaseEmission(cityGround[x][y]);
                        if (emission < 0) {
                            double dist = distance(i, j, x, y);

                            // 예외 조건 방지
                            if (dist <= 4 && dist > 0) {
                                float weightedReduction = -emission * (4 - dist) * 0.6f;

                                if (weightedReduction > maxReduction) {
                                    maxReduction = weightedReduction;
                                }
                            }
                        }
                    }
                }
                dustMap[i][j] -= maxReduction;
                // 음수 방지
                if (dustMap[i][j] < 0)
                    dustMap[i][j] = 0;
            }
        }
    }

    // 미세먼지 확산 함수
    double getDustSpread(double baseDust, int distance, double decayRate = 0.15) {
        // 지수 감수 함수
        return baseDust * exp(-decayRate * distance);
    }

    // 미세먼지 완화량 계산 함수
    double getDustAbsorption(double baseAbsorption, double dustDensity, double intensity = 0.01) {
        double efficiencyMultiplier = 1.0 + intensity * dustDensity;
        return baseAbsorption * efficiencyMultiplier;
    }

    // 발전소 오염력
    double getPollutionPower(char c) {
        switch (c) {
        case 'C': return 6.0 * (coalPower / 300.0);     // 석탄발전소 (가동률 비례하여 증가)
        case 'F': return 1.2;                           // 자동차 공장
        case 'A': return 0.15;                          // 행정시설
        default: return 0;
        }
    }

    // 석탄발전소까지 가장 가까운 거리 계산
    double getShortestDistanceCoalPowerFactory(int i, int j) {
        float minDistance = 10000;

        for (int k = 88; k < 100; ++k) {
            for (int l = 88; l < 100; ++l) {
                if (minDistance == 1)
                    return minDistance;

                float temp;
                // 최적화
                temp = distance(i, j, k, l);

                if (minDistance > temp)
                    minDistance = temp;
            }
        }

        return minDistance;
    }

    // 자동차 공장까지 가장 가까운 거리 계산
    double getShortestDistanceCarFactory(int i, int j) {
        float minDistance = 10000;

        for (int k = 78; k <= 81; ++k) {
            for (int l = 95; l < 100; ++l) {
                if (minDistance == 1)
                    return minDistance;

                float temp;
                // 최적화
                temp = distance(i, j, k, l);

                if (minDistance > temp)
                    minDistance = temp;
            }
        }

        return minDistance;
    }

    // 월별 미세먼지 확산, 한 달 기준
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
                    double baseEmission = 2.2 * (coalPower / 300.0);
                    dustIncrement = baseEmission * (1.0 + 0.02 * currentDust);
                    dustMap[i][j] += dustIncrement;
                }
                else {
                    // 거리 기반 확산량 계산
                    double coalDist = getShortestDistanceCoalPowerFactory(i, j);
                    double carDist = getShortestDistanceCarFactory(i, j);
                    double adminDist = distance(i, j, 49, 49);

                    double spreadC = getDustSpread(getPollutionPower('C'), int(coalDist));
                    double spreadF = getDustSpread(getPollutionPower('F'), int(carDist));
                    double spreadA = getDustSpread(getPollutionPower('A'), int(adminDist));

                    // 중복 제거 및 최대 값만 사용
                    dustIncrement = max({ spreadC, spreadF, spreadA });

                    // 현재 농도에 따른 확산 효율 조정
                    if (currentDust > 46) {
                        dustIncrement *= 0.85;
                    }
                    else if (currentDust > 25) {
                        dustIncrement *= 1.2;
                    }
                    else {
                        dustIncrement *= 1.6;
                    }

                    dustMap[i][j] += dustIncrement;
                }
            }
        }
    }

    // 월별 자연적 미세먼지 완화
    void monthlyNaturalMitigateDust() {
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                float currentDust = dustMap[i][j];

                // 농도별 자연 감소율
                if (currentDust > 56) {
                    dustMap[i][j] *= 0.98;
                }
                else if (currentDust > 46) {
                    dustMap[i][j] *= 0.985;
                }
                else if (currentDust > 36) {
                    dustMap[i][j] *= 0.987;
                }
                else if (currentDust > 31) {
                    dustMap[i][j] *= 0.99;
                }
                else {
                    dustMap[i][j] *= 0.995;
                }
            }
        }
    }

    // 완화 시설 효과
    double getMitigationPower(char c) {
        switch (c) {
        case 'T': return 1.5;            // 유원지
        case 'M': return 0.5;            // 목초지
        default: return 0;
        }
    }

    // 유원지까지 가장 가까운 거리 찾기
    double getShortestDistanceTomb(int i, int j) {
        float minDistance = 10000;

        for (int k = 75; k <= 79; ++k) {
            for (int l = 19; l < 24; ++l) {
                if (minDistance == 1)
                    return minDistance;

                float temp;
                // 최적화
                temp = distance(i, j, k, l);

                // 음수 방지
                if (minDistance > temp)
                    minDistance = temp;
            }
        }

        return minDistance;
    }

    // 목초지까지 가장 가까운 거리 찾기
    double getShortestDistanceGrass(int i, int j) {
        float minDistance = 10000;

        for (int k = 83; k <= 87; ++k) {
            for (int l = 83; l < 87; ++l) {
                if (minDistance == 1)
                    return minDistance;

                float temp;
                // 최적화
                temp = distance(i, j, k, l);

                // 음수 방지
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

                // 수식: (완화량) / ((거리)^2 + 1)
                // 0으로 나누어짐 방지 ->  1 더해서 나누기
                if (cityGround[i][j] == 'T') {
                    double base = getMitigationPower('T');
                    reduction = getDustAbsorption(base, dustDensity, 0.007);
                }
                else if (cityGround[i][j] == 'M') {
                    double tombEffect = getMitigationPower('T') / (pow(getShortestDistanceTomb(i, j), 2) + 1);
                    double grassEffect = getMitigationPower('M') / (pow(getShortestDistanceGrass(i, j), 2) + 1);
                    double base = max(tombEffect, grassEffect);
                    reduction = getDustAbsorption(base, dustDensity, 0.005);
                }
                else {
                    double tombEffect = getMitigationPower('T') / (pow(getShortestDistanceTomb(i, j), 2) + 1);
                    double grassEffect = getMitigationPower('M') / (pow(getShortestDistanceGrass(i, j), 2) + 1);
                    double base = max(tombEffect, grassEffect);
                    reduction = getDustAbsorption(base, dustDensity, 0.003);
                }

                dustMap[i][j] -= reduction;
                // 음수 방지
                if (dustMap[i][j] < 0)
                    dustMap[i][j] = 0;
            }
        }
    }

    // 입원자 수 계산
    void calculateHospitalizationPeople() {
        averageDustRate = getAverageDustRate();

        if (averageDustRate <= 35) {
            return;
        }
        else {
            monthHospitalizationPeople = 3 * (averageDustRate - 35);
            totalHospitalizationPeople += monthHospitalizationPeople;
        }
    }

    // 사망자 수 계산
    void calculateDeathPeople() {
        averageDustRate = getAverageDustRate();

        if (averageDustRate <= 35) {
            return;
        }
        else {
            monthDeathPeople = round(0.2 * (averageDustRate - 35));
            totalDeathPeople += monthDeathPeople;
        }
    }

    // 병원 점수 계산
    double getHospitalScore(double powerRatio) {
        // a, b: 시그모이드 조절 파라미터
        double a = 15.0;  // 기울기 조절
        double b = 0.7;   // 임계 가동률
        double sigmoid = 1.0 / (1.0 + exp(-a * (powerRatio - b)));
        return 30.0 * sigmoid;
    }    

    // 행정시설 점수 계산
    double getAdminScore(double powerRatio) {
        return 20.0 * sqrt(powerRatio);
    }    

    // 자동차공장 점수 계산
    double getFactoryScore(double powerRatio) {
        double a = 20.0;
        double b = 0.75;
        double sigmoid = 1.0 / (1.0 + exp(-a * (powerRatio - b)));
        return 30.0 * sigmoid;
    } 
    
    // 유원지 점수 계산
    double getParkScore(double powerRatio) {
        double k = 6.0;
        double base = log(1 + k * powerRatio) / log(1 + k);
        return 20.0 * sqrt(base);
    }    

    // 점수 계산
    void setScore() {
        // 점수 초기화
        score = 0;

        // 미세먼지 부분 점수
        dustScore = max(0.0, 100.0 * exp(-0.03 * averageDustRate));
        score += dustScore * 0.3;

        double powerRatio = (double)coalPower / 300.0;

        // 시설 부분 점수
        double hospitalScore = getHospitalScore(powerRatio);
        double adminScore = getAdminScore(powerRatio);
        double factoryScore = getFactoryScore(powerRatio);
        double parkScore = getParkScore(powerRatio);

        facilityScore = hospitalScore + adminScore + factoryScore + parkScore;

        score += facilityScore * 0.7;

        // 사상자 부분 점수 감점
        score -= monthHospitalizationPeople * 0.05;
        score -= monthDeathPeople * 2;
    }

    // 데이터 저장
    void recordData(int currentMonth) {
        // 추가 모드
        ofstream dustFile("dust.data", ios::app);
        if (dustFile.is_open()) {
            dustFile << currentMonth << " " << averageDustRate << endl;
            dustFile.close();
        }

        // 추가 모드
        ofstream scoreFile("score.data", ios::app);
        if (scoreFile.is_open()) {
            scoreFile << currentMonth << " " << score << endl;
            scoreFile.close();
        }
    }

public:
    City(int coalPower) : coalPower(coalPower) {
        // 도시 전역의 건물 초기화
        for (int i = 0; i < 100; ++i)
            for (int j = 0; j < 100; ++j)
                cityGround[i][j] = 'E';

        cityGround[30][30] = 'H';
        cityGround[49][49] = 'A';

        for (int i = 75; i <= 79; ++i)
            for (int j = 19; j <= 24; ++j)
                cityGround[i][j] = 'T';

        for (int i = 78; i <= 81; ++i)
            for (int j = 95; j <= 99; ++j)
                cityGround[i][j] = 'F';

        for (int i = 88; i <= 99; ++i)
            for (int j = 88; j <= 99; ++j)
                cityGround[i][j] = 'C';

        for (int i = 83; i <= 87; ++i)
            for (int j = 83; j <= 87; ++j)
                cityGround[i][j] = 'M';

        // 도시 전역의 미세먼지 초기화
        initDustMap();
        averageDustRate = getAverageDustRate();
    }

    // 메인 호출 부분
    void usingCity() {
        int initialCoalPower = coalPower;
        coalPower = 300;

        // 처음에 100%로 1년 돌리고 시작
        cout << "=== 초기 상태 (100% 가동 1년 시뮬레이션) ===" << endl;
        for (int month = 0; month < 12; ++month) {
            // 확산
            monthlySpreadDust();
            // 자연적 완화
            monthlyNaturalMitigateDust();
            // 지형적 완화
            monthlyLocalMitigateDust();
        }
        recordData(0);

        // 평균 미세먼지 농도 확인
        averageDustRate = getAverageDustRate();
        cout << "초기 평균 미세먼지 (100% 1년 후): " << averageDustRate << endl;

        // 다시 선택한 가동률로 재설정
        coalPower = initialCoalPower;

        // 실질적 본문 코드
        cout << "\n=== 현재 설정 ===" << endl;
        cout << "발전소 가동률: " << (coalPower * 100 / 300) << "%" << endl;

        int months;
        cout << "시뮬레이션을 몇 개월 돌릴건가요? ";
        cin >> months;

        cout << "\n=== 시뮬레이션 진행 ===" << endl;
        for (int month = 0; month < months; ++month) {
            setScore();
            monthlySpreadDust();
            monthlyNaturalMitigateDust();
            monthlyLocalMitigateDust();
            calculateHospitalizationPeople();
            calculateDeathPeople();
            if ((month + 1) % 6 == 0)
                recordData(month + 1);
        }

        // 시뮬레이션 결과
        cout << "\n=== 최종 결과 ===" << endl;
        cout << "최종 평균 미세먼지: " << averageDustRate << endl;
        cout << "총 점수: " << score << endl;

        char showMap;
        cout << "미세먼지 농도 지도를 출력할까요? (y/n): ";
        cin >> showMap;
        if (showMap == 'y' || showMap == 'Y')
            getCityDust();
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

    switch (choice) {
    case 1: coalPower = 300; break;
    case 2: coalPower = 210; break;
    case 3: coalPower = 90; break;
    default:
        cout << "잘못된 입력입니다. 기본값 100%로 설정합니다." << endl;
        coalPower = 300;
        break;
    }

    City citySim(coalPower);
    citySim.usingCity();
}
