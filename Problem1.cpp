#include <iostream>
using namespace std;

class City {
private:
    char cityGround[100][100];
    int score = 100;

    void getCityGround() {
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j)
                cout << cityGround[i][j];
            cout << endl;
        }
    }

public:
    City() {
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
    }

    void usingCity() {
        getCityGround();
    }

};

int main() {
    City callingVar;
    callingVar.usingCity();
}