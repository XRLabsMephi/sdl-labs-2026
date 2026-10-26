#pragma once
#include <vector>
/*
    файл c определением класса Board

    представляет собой параллелепипед, заданный в 3ех измерениях
*/

//Класс произвольной коробки, доступной для отрисовки
class Board {
public:
    double CoordX;
    double CoordY;
    double CoordZ;
    double LenghtX;
    double LenghtY;
    double LenghtZ;

    int wallId = 1; // id текстуры которой будет краситься параллелепипед

    std::vector<int> cur_col = { 0, 0, 0, 0 };// - рудимент до добавления текстурирования

    Board(double CoordX, double CoordY, double CoordZ,
        double LenghtX, double LenghtY, double LenghtZ);

    //проверка принадлежности точки коробке
    bool CheckConflict(double x, double y);
};
