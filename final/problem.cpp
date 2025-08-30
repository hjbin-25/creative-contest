#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fstream>
using namespace std;

class City {
private:

    /// 건물별 기존 하루 이용 전력 (kWh/일)
    double beforeElectricUseAmounts[20] = {
        12000.0, // F1 종합병원
        480.0,   // F2 초등 돌봄교실
        12000.0, // F3 대형 쇼핑몰
        600.0,   // F4 노인복지관
        1200.0,  // F5 소방서
        6000.0,  // F6 상수도 처리장
        800.0,   // F7 시청·지휘본부
        1000.0,  // F8 중소 제조공장
        3000.0,  // F9 지하철 역사
        4000.0,  // F10 대규모 아파트 단지
        2500.0,  // F11 중형병원
        1200.0,  // F12 대학 캠퍼스
        1000.0,  // F13 경찰서
        1800.0,  // F14 노인 요양원
        1500.0,  // F15 냉장·냉동 물류창고
        300.0,   // F16 작은 도서관
        2000.0,  // F17 통신 기지국
        500.0,   // F18 전통시장
        400.0,   // F19 동네 병원
        2000.0   // F20 하수 처리장
    };

    // 건물별 세부 점수: {생명 안전, 전력 필요성, 피해 규모, 공적 가치, 합계, 최종점수*2}
    double buildingScores[20][6] = {
        {5, 5, 4, 5, 19, 9.5},
        {4, 3, 3, 3, 13, 6.5},
        {2, 4, 5, 2, 13, 6.5},
        {4, 4, 4, 3, 15, 7.5},
        {5, 4, 4, 5, 18, 9.0},
        {4, 5, 5, 5, 19, 9.5},
        {3, 3, 4, 5, 15, 7.5},
        {2, 3, 3, 3, 11, 5.5},
        {4, 4, 5, 5, 18, 9.0},
        {3, 4, 5, 4, 16, 8.0},
        {5, 5, 3, 4, 17, 8.5},
        {2, 3, 4, 3, 12, 6.0},
        {4, 4, 4, 5, 17, 8.5},
        {5, 5, 4, 4, 18, 9.0},
        {2, 4, 4, 3, 13, 6.5},
        {3, 3, 3, 3, 12, 6.0},
        {4, 5, 5, 5, 19, 9.5},
        {2, 2, 3, 2, 9, 4.5},
        {3, 3, 3, 3, 12, 6.0},
        {2, 4, 5, 2, 13, 6.5}
    };

    int blackoutSchedule[20][24];

    // 건물별 가동률 (수정됨)
    double buildingWorkingPercent[20];
    
    // 건물별 전기 모드 (수정됨: 'F' -> 'K'로 변경)
    char electricModes[20] = {
        'K', // F1 종합병원 - 생명 직결, 응급실/수술실 24시간
        'R', // F2 초등 돌봄교실 - 아동 취약계층, 최소 냉방 유지
        'S', // F3 대형 쇼핑몰 - 경제적 피해, 피난 가능
        'R', // F4 노인복지관 - 고위험 노인, 최소 전력 공급
        'K', // F5 소방서 - 대응력 저하 방지
        'K', // F6 상수도 처리장 - 도시 급수, 필수 유지
        'R', // F7 시청·지휘본부 - 일부 발전기 있으므로 절약 가능
        'S', // F8 중소 제조공장 - 경제적 손실, 직접 생명위협 낮음
        'R', // F9 지하철 역사 - 군중 안전 위해 최소 전력
        'R', // F10 대규모 아파트 - 일부 자구책 가능, 절약 모드
        'K', // F11 중형병원 - 특정 환자군 필수 전력
        'R', // F12 대학 캠퍼스 - 대체 공간 활용 가능
        'K', // F13 경찰서 - 치안 유지 필수
        'K', // F14 노인 요양원 - 대규모 취약계층, 전력 필수
        'S', // F15 냉장·냉동 물류 - 경제적 피해 중심
        'S', // F16 작은 도서관(피난처) - 대체 가능
        'K', // F17 통신 기지국 - 전사회 통신망 필수
        'S', // F18 전통시장 - 불편 크지만 기본 영업 가능
        'R', // F19 동네 병원 - 외래 중심, 최소 전력 유지
        'S'  // F20 하수 처리장 - 단기 임시 대응 가능
    };

    double workingHours[20];

    // 건물별 최소 하루 전력 필요량 (kWh/일) - 계산 방식 수정
    double minRequiredElectricUse[20];

    // 건물별 피해점수 (100점 만점)
    float damageScore[20];

public:
    
    City() {
        for (int i = 0; i < 20; ++i) {
            damageScore[i] = 100;
            workingHours[i] = 24;

            // 최소 전력 필요량 계산 로직 수정
            // 가중평균을 5점 만점으로 정규화한 후 30-80% 범위로 매핑
            double weightedAvg = (buildingScores[i][0] * 0.4 + 
                                 buildingScores[i][1] * 0.3 + 
                                 buildingScores[i][2] * 0.15 + 
                                 buildingScores[i][3] * 0.15);
            
            // 5점 만점을 1.0으로 정규화하고, 0.3~0.8 범위로 매핑
            double minRatio = (weightedAvg / 5.0) * 0.5 + 0.3;
            minRequiredElectricUse[i] = beforeElectricUseAmounts[i] * minRatio;
        }
    }

    void fullModeUsing(int index) {
        buildingWorkingPercent[index] = 100.0;
        workingHours[index] = 24.0; // 24시간 가동
    }

    void reduceModeUsing(int index) {
        double requiredElectric = minRequiredElectricUse[index];
        double beforeElectric = beforeElectricUseAmounts[index];

        double workingRate = (requiredElectric / beforeElectric) * 100;
        buildingWorkingPercent[index] = workingRate;
        workingHours[index] = 24.0; // 24시간 가동하되 전력만 줄임
    }
    
    void stopModeUsing(int index) {
        // 순환정전: 최소 전력으로 제한된 시간만 가동
        double requiredElectric = minRequiredElectricUse[index];
        double beforeElectric = beforeElectricUseAmounts[index];

        buildingWorkingPercent[index] = 100.0; // 가동 시에는 정상 전력
        
        // 가동 시간을 줄여서 전체 전력 사용량을 최소 필요량으로 제한
        workingHours[index] = 24.0 * (requiredElectric / beforeElectric);
        
        // 건물별 특성에 맞는 정전 시간대 설정
        for (int h = 0; h < 24; h++) {
            blackoutSchedule[index][h] = 0; // 기본적으로 정전
        }
        
        int operatingHours = (int)workingHours[index];
        
        // // 건물별 맞춤형 가동 시간대 설정
        // switch(index) {
        //     case 2: // F3 대형 쇼핑몰
        //         // 영업시간 10~22시 중심, 새벽 1~6시 정전
        //         for (int h = 10; h < 22 && operatingHours > 0; h++) {
        //             blackoutSchedule[index][h] = 1;
        //             operatingHours--;
        //         }
        //         break;
                
        //     case 7: // F8 중소 제조공장  
        //         // 근무시간 9~18시 중심, 야간 22~6시 정전
        //         for (int h = 9; h < 18 && operatingHours > 0; h++) {
        //             blackoutSchedule[index][h] = 1;
        //             operatingHours--;
        //         }
        //         break;
                
        //     case 14: // F15 냉장·냉동 물류창고
        //         // 새벽 배송준비 4~6시, 오전 8~12시, 오후 14~18시 가동
        //         // 야간 22~4시, 점심 12~14시 정전
        //         int priority_hours[] = {4, 5, 8, 9, 10, 11, 14, 15, 16, 17};
        //         for (int i = 0; i < 10 && operatingHours > 0; i++) {
        //             blackoutSchedule[index][priority_hours[i]] = 1;
        //             operatingHours--;
        //         }
        //         break;
                
        //     case 15: // F16 작은 도서관
        //         // 이용시간 9~18시, 야간 20~8시 정전  
        //         for (int h = 9; h < 18 && operatingHours > 0; h++) {
        //             blackoutSchedule[index][h] = 1;
        //             operatingHours--;
        //         }
        //         break;
                
        //     case 17: // F18 전통시장
        //         // 장보기 시간 6~8시, 10~15시, 야간 18~6시 정전
        //         int market_hours[] = {6, 7, 10, 11, 12, 13, 14};
        //         for (int i = 0; i < 7 && operatingHours > 0; i++) {
        //             blackoutSchedule[index][market_hours[i]] = 1;
        //             operatingHours--;
        //         }
        //         break;
                
        //     case 19: // F20 하수 처리장
        //         // 사용량 많은 시간 피해서: 7~9시, 11~13시, 15~17시, 20~22시 가동
        //         // 새벽 1~6시, 점심직후 13~15시, 늦은밤 23~1시 정전
        //         int sewage_hours[] = {7, 8, 11, 12, 15, 16, 20, 21};
        //         for (int i = 0; i < 8 && operatingHours > 0; i++) {
        //             blackoutSchedule[index][sewage_hours[i]] = 1;
        //             operatingHours--;
        //         }
        //         break;
                
        //     default:
        //         // 기타 S 모드 건물들: 새벽시간 우선 정전 (1~6시)
        //         // 필요시간만큼 중요도 높은 시간부터 가동
        //         for (int h = 7; h < 24 && operatingHours > 0; h++) {
        //             blackoutSchedule[index][h] = 1;
        //             operatingHours--;
        //         }
        //         for (int h = 0; h < 7 && operatingHours > 0; h++) {
        //             blackoutSchedule[index][h] = 1;
        //             operatingHours--;
        //         }
        //         break;
        // }
    }

    void simulation() {
        for (int i = 0; i < 20; ++i) {
            if (electricModes[i] == 'K') {      // Keep - 유지
                fullModeUsing(i);
            } else if (electricModes[i] == 'R') { // Reduce - 절약
                reduceModeUsing(i);
            } else if (electricModes[i] == 'S') { // Stop(순환정전)
                stopModeUsing(i);
            }
        }

        // for (int i = 0; i < 20; ++i) {
        //     cout << "F" << i + 1 << ": " << buildingWorkingPercent[i] * beforeElectricUseAmounts[i] << endl;
        // }

        cout << "=== 전력 공급 시뮬레이션 결과 ===" << endl;
        cout << "건물\t모드\t가동률(%)\t가동시간(h)\t실제전력사용(kWh)\t절약률(%)" << endl;
        cout << "-----------------------------------------------------------" << endl;
        
        double totalElectricUse = 0;
        double totalOriginalUse = 0;
        
        for (int i = 0; i < 20; ++i) {
            double actualElectricUse = beforeElectricUseAmounts[i] * 
                                     (buildingWorkingPercent[i] / 100.0) * 
                                     (workingHours[i] / 24.0);
            totalElectricUse += actualElectricUse;
            totalOriginalUse += beforeElectricUseAmounts[i];
            
            // 절약률 계산 (원래 사용량 대비 줄인 비율)
            double savedRate = ((beforeElectricUseAmounts[i] - actualElectricUse) / 
                               beforeElectricUseAmounts[i]) * 100;
            
            cout << "F" << i + 1 << "\t";
            cout << electricModes[i] << "\t";
            cout << buildingWorkingPercent[i] << "\t";
            cout << workingHours[i] << "\t";
            cout << actualElectricUse << "\t\t";
            cout << savedRate << endl;
        }
        
        cout << "-----------------------------------------------------------" << endl;
        cout << "총 원래 전력 사용량: " << totalOriginalUse << " kWh/일" << endl;
        cout << "총 실제 전력 사용량: " << totalElectricUse << " kWh/일" << endl;
        cout << "전체 절약률: " << ((totalOriginalUse - totalElectricUse) / totalOriginalUse) * 100 << "%" << endl;
    }

};

int main() {
    City callingVar;
    callingVar.simulation();

    return 0;
}