#pragma once
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

    Board(double CoordX, double CoordY, double CoordZ,
        double LenghtX, double LenghtY, double LenghtZ);

    //проверка принадлежности точки коробке
    bool CheckConflict(double x, double y);
};
