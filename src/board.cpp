#include "board.h"

/*
    файл с реализацией функций из board.h
*/


Board::Board(double CoordX, double CoordY, double CoordZ,
    double LenghtX, double LenghtY, double LenghtZ)
    : CoordX(CoordX), CoordY(CoordY), CoordZ(CoordZ),
    LenghtX(LenghtX), LenghtY(LenghtY), LenghtZ(LenghtZ) {
}

    
bool Board::CheckConflict(double x, double y) {
    return x >= CoordX &&
        x <= CoordX + LenghtX &&
        y >= CoordY &&
        y <= CoordY + LenghtY;
}
