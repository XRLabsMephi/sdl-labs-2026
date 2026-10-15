#pragma once
#include <cstddef>

/*
RayHit - структура для получения результата пуска луча.
*/

struct RayHit {
    bool hit=false; //попал ли луч в стену
    double depth=0.0; //глубина поверхности
    int mapX; //координата по x
    int mapY; //координата по y
    int wall_id=-1; //id стены
    short side; //какая сторона стены была пересечена лучом: 0 - левая, 1 - правая, 2 - верхняя, 3 - нижняя

    double wall_u=0.0; //координата попадания по поверхности стены
};