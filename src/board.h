#pragma once
#include <vector>
/*
    файл c определением класса Board

    представл€ет собой параллелепипед, заданный в 3ех измерени€х
*/

// ласс произвольной коробки, доступной дл€ отрисовки
class Board {
public:
    double CoordX;
    double CoordY;
    double CoordZ;
    double LenghtX;
    double LenghtY;
    double LenghtZ;

    std::vector<int> cur_col = { 0, 0, 0, 0 };

    Board(double CoordX, double CoordY, double CoordZ,
        double LenghtX, double LenghtY, double LenghtZ);

    //проверка принадлежности точки коробке
    bool CheckConflict(double x, double y);
};
