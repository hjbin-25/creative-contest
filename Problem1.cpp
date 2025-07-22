#include <iostream>     // 입출력 가져옴
#include <cmath>        // sqrt, pow, max 가져옴
#include <algorithm>    // 이거 왜 있는지 모르겠는데 불안해서 나둘거임
#include <cstdlib>      // 2222222
using namespace std;

// 도시를 구현
class City {
// 일단 접근 막음
private:
    // 도시 전역
    char cityGround[100][100];
    // 점수: 나중에 실행할 대안을 이 점수를 기반으로 평가할거임
    int score = 0;
    // 원래 수익
    long long originalRevenuePerMonth = (24 * 300 - 300) * 30 * 7000 - 150000000;
    // 도시의 전력 판매로 인한 수익
    long long revenuePerMonth = 0;
    // 하루에 생산 시킬 전력, 단위는 MW, 100%->300, 70%->210, 30%->90
    int coalPower = 300;
    // 시민의 예산 관련 만족도 점수
    float budgetScore = 0;
    // 시민의 미세먼지 관련 만족도 점수
    float dustScore = 0;
    // 도시 전역의 미세먼지 지수, 단위는 PM2.5
    float dustMap[100][100];
    // 도시 전역의 펑균 미세먼지 지수, 단위는 PM2.5
    float averageDustRate;

    // 도시 전역 출력
    void getCityGround() {
        // 예쁘게 찍는 건 포기함, 어짜피 출력할일 거의 없어
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j)
                cout << cityGround[i][j];
            cout << endl;
        }
    }

    // 도시 전역의 미세먼지 농도 출력
    void getCityDust() {
        // 2222222
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j)
                cout << dustMap[i][j] << " ";
            cout << endl;
        }
    }

    // 평균 미세먼지 지수 리턴
    float getAverageDustRate() {
        float totalDust = 0;

        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j)
                // 제발 오버플로우만 나지 마라
                totalDust += dustMap[i][j];
        }

        return totalDust / 10000;
    }

    // 각 건물별 미세먼지 영향량
    float getBaseEmission(char c) {
        switch(c) {
            // 나중에 고칠 수도 있음, 신중하게 결정!!
            case 'C': return 10.0f;    // 석탄발전소
            case 'F': return 3.0f;     // 자동차 공장
            case 'A': return 0.5f;     // 행정시설
            case 'H': return 0.1f;     // 병원
            case 'T': return -1.0f;    // 유원지
            case 'M': return -0.5f;    // 목초지
            case 'E': return 0.3f;     // 기타
            default: return 0.0f;      // 예외는 없음, 그냥 형식상 붙임
        }
    }

    // 거리 계산
    double distance(int x1, int y1, int x2, int y2) {
        // 단순 피타고라스 공식
        return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    }

    // 미세먼지 농도 초기화
    void initDustMap() {
        // 기본 배경 미세먼지 농도를 32로 설정
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                // 기본적으로 32정도라고 함, 어짜피 나중에 1년 돌리고 시작할거라 상관 없긴함
                dustMap[i][j] = 32.0f;
            }
        }
        
        // 직접 영향 계산
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                float directEmission = getBaseEmission(cityGround[i][j]);
                // 일단 먼저 해당 칸의 영향부터 계산하고 시작
                dustMap[i][j] += directEmission;
                // 음수 나오는거 막아줌
                if (dustMap[i][j] < 0)
                    dustMap[i][j] = 0;
            }
        }
        
        // 주변 영향 계산 (오염원) - 영향 범위와 강도 조정
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                // 혹시나 죽을까봐 -1로 초기화 해둠
                float maxAddition = -1;

                // 시간복잡도 망해서 터질까봐 쫄리긴 함
                for (int x = max(0, i - 8); x <= min(99, i + 8); ++x) {
                    for (int y = max(0, j - 8); y <= min(99, j + 8); ++y) {
                        float emission = getBaseEmission(cityGround[x][y]);

                        if (emission > 0) {
                            // 거리 부분 최적화, 아마도?
                            double dist = distance(i, j, x, y);

                            if (dist <= 8 && dist > 0) {
                                // 너무 약해서 20% 추가로 줌
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

        // 주변 영향 계산 (완화원) - 완화 효과 증가
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                // 2222222
                float maxReduction = -1;

                // 2222222222
                for (int x = max(0, i - 4); x <= min(99, i + 4); ++x) {
                    for (int y = max(0, j - 4); y <= min(99, j + 4); ++y) {
                        float emission = getBaseEmission(cityGround[x][y]);
                        if (emission < 0) {
                            double dist = distance(i, j, x, y);

                            if (dist <= 4 && dist > 0) {
                                // 얘는 반대로 너무 강해서 40% 줄임
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
        // 지수 감수 함수라는데 솔직히 잘 모르겠음, 애들이 이거 쓰래서 만들긴 함
        return baseDust * exp(-decayRate * distance);
        // 결과는 잘 나오니 됬음
    }

    // 미세먼지 완화량 계산 함수
    double getDustAbsorption(double baseAbsorption, double dustDensity, double intensity = 0.01) {
        // 가중치 떡칠
        double efficiencyMultiplier = 1.0 + intensity * dustDensity;
        return baseAbsorption * efficiencyMultiplier;
    }

    // 발전소 오염력 (가동률 반영)
    double getPollutionPower(char c) {
        switch(c) {
            // 제발 값만 잘 나와라
            case 'C': return 6.0 * (coalPower / 300.0);     // 석탄발전소(가동률 비례하여 증가)
            case 'F': return 1.2;                           // 자동차 공장
            case 'A': return 0.15;                          // 행정시설
            default: return 0;                              // 예외는 없지만 형식상 달아줌
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
        
        // 나중에 시간되면 위에 함수랑 같이 최적화 하면 좋을 듯
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

    // 월별 미세먼지 확산 (30일 기준)
    void monthlySpreadDust() {
        float tempDustMap[100][100];
        
        // 기존 dustMap 복사
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                tempDustMap[i][j] = dustMap[i][j];
            }
        }
        
        // 시간복잡도, O(2 * 100^2) => 상당히 안 좋긴함
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) { 
                double dustIncrement = 0;
                double currentDust = tempDustMap[i][j];

                if (cityGround[i][j] == 'C') {
                    // 발전소에서 직접 배출, 가중치 조정 매우 중요함!!
                    double baseEmission = 2.2 * (coalPower / 300.0);
                    dustIncrement = baseEmission * (1.0 + 0.02 * currentDust);
                    dustMap[i][j] += dustIncrement;
                } else {
                    // 거리 기반 확산량 계산
                    double coalDist = getShortestDistanceCoalPowerFactory(i, j);
                    double carDist = getShortestDistanceCarFactory(i, j);
                    // 행정시설은 1칸으로 해서 이렇게 적음
                    double adminDist = distance(i, j, 49, 49);

                    double spreadC = getDustSpread(getPollutionPower('C'), int(coalDist));
                    double spreadF = getDustSpread(getPollutionPower('F'), int(carDist));
                    double spreadA = getDustSpread(getPollutionPower('A'), int(adminDist));

                    // 중복 제거 및 최대 값만 사용
                    dustIncrement = max({spreadC, spreadF, spreadA});
                    
                    // 현재 농도에 따른 확산 효율 조정
                    // 이 또한 매우 예민한 값임
                    if (currentDust > 46) {
                        dustIncrement *= 0.85;
                    } else if (currentDust > 25) {
                        dustIncrement *= 1.2;
                    } else {
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
                // 일단 내 기준 최적의 값인 듯
                if (currentDust > 56) {
                    dustMap[i][j] *= 0.98;
                } else if (currentDust > 46) {
                    dustMap[i][j] *= 0.985;
                } else if (currentDust > 36) {
                    dustMap[i][j] *= 0.987;
                } else if (currentDust > 31) {
                    dustMap[i][j] *= 0.99;
                } else {
                    dustMap[i][j] *= 0.995;
                }              
            }
        }
    }

    // 완화 시설 효과
    double getMitigationPower(char c) {
        switch(c) {
            // 값 수정 하면 좋을 듯
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

                // 저 비율 절대 절대 절대 건들면 안됨!!!!!!!
                // 수식은 (완화량) / ((거리)^2 + 1)
                // 0으로 나누는거 막을려고 1 더해줌
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
                // 음수 방지
                if (dustMap[i][j] < 0)
                    dustMap[i][j] = 0;
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

        // 기본 도시의 월 예산 계산
        originalRevenuePerMonth = (24 * 300 - 300) * 30 * 7000 - 150000000;

        // 도시의 월 예산 계산
        // 하루에 사용하는 전력인 300MWh/day 제외하고, 실제 수입인 7% 계산, 편의상 한 달을 30일로 가정하고, 1MWh/day == 100,000원으로 가정, 부채 상환액은 150,000,000으로 가정
        revenuePerMonth = (coalPower * 24 - 300) * 30 * 7000 - 150000000;

        // 점수 계산 - 실수 기반
        double revenueRatio = (double)revenuePerMonth / (double)originalRevenuePerMonth * 100.0;
        double reduction = 100.0 - revenueRatio;

        // 도로 및 교통 인프라 예산
        budgetScore += 0.2 * reduction;
        // 복지 및 사회 서비스 예산
        budgetScore += 0.18 * reduction;
        // 공공 안전 및 치안 예산
        budgetScore += 0.15 * reduction;
        // 미세먼지 대응 예산
        budgetScore += 0.12 * reduction;
        // 행정 운영비 예산
        budgetScore += 0.1 * reduction;
        // 교육 및 문화 예산
        budgetScore += 0.08 * reduction;
        // 경제 활성화 및 창업 지원 예산
        budgetScore += 0.06 * reduction;
        // 환경 보전(미세먼지 제외) 예산
        budgetScore += 0.05 * reduction;
        // 비상 예산 및 적립금
        budgetScore += 0.03 * reduction;
        // 도시 개발 및 투자 예산
        budgetScore += 0.03 * reduction;


        budgetScore = 100 - budgetScore;

        // 가중치 바꿔야 될 듯
        score += budgetScore * 0.7;
    }

    // 메인 호출 부분
    void usingCity() {
        // 처음에 100%로 1년 돌리고 시작
        int initialCoalPower = coalPower;
        coalPower = 300;
        
        cout << "=== 초기 상태 (100% 가동 1년 시뮬레이션) ===" << endl;
        for (int month = 0; month < 12; ++month) {
            monthlySpreadDust();
            monthlyNaturalMitigateDust();
            monthlyLocalMitigateDust();
        }

        averageDustRate = getAverageDustRate();
        cout << "초기 평균 미세먼지 (100% 1년 후): " << averageDustRate << endl;
        
        // 다시 선택한 가동률로 재설정
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
            
            // 3개월 마다 결과 출력
            if ((month + 1) % 3 == 0) {
                float currentAvg = getAverageDustRate();
                cout << (month + 1) << "개월 후 평균 미세먼지: " << currentAvg << endl;
            }
        }
        
        averageDustRate = getAverageDustRate();

        cout << "\n=== 최종 결과 ===" << endl;
        cout << "최종 평균 미세먼지: " << averageDustRate << endl;
        
        if (coalPower == 300) {
            cout << "발전소 100% 가동 - 미세먼지 농도가 높게 유지됩니다." << endl;
        } else if (coalPower == 210) {
            cout << "발전소 70% 가동 - 미세먼지 농도가 점진적으로 개선되고 있습니다." << endl;
        } else if (coalPower == 90) {
            cout << "발전소 30% 가동 - 미세먼지 농도가 크게 개선되었습니다." << endl;
        }

        // 미세먼지 지수로 인한 점수 변화
        // 솔직히 이게 무슨 공식인지 모르겠다, 일단 하라니까 해본다
        dustScore = max(0.0, 100.0 * exp(-0.03 * averageDustRate));
        
        // 가중치 바꿔야 될듯
        score += dustScore * 0.3;

        // 일단 점수 출력 해봄
        cout << "미세먼지 점수: " << dustScore << endl;
        cout << "예산 점수: " << budgetScore << endl;
        cout << "총 점수: " << score << endl;
        
        char showMap;
        // 보기 안 예뻐서 선택적 출력으로 바꿈
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
        // 다시 입력 받기 귀찮은거 맞음
        default:
            cout << "잘못된 입력입니다. 기본값 100%로 설정합니다." << endl;
            coalPower = 300;
            break;
    }

    // 일단 클래스한테 떠넘김
    City callingVar(coalPower);
    callingVar.usingCity();
}