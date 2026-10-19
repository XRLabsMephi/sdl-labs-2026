//
// Created by Nikita on 19.10.2026.
//

#ifndef SOFTWAREGRAPHICSLABS_MAP_H
#define SOFTWAREGRAPHICSLABS_MAP_H


#include <string>
#include <vector>

class Map {
private:
    int width;
    int height;
    std::vector<int> cells;
public:
    Map();
    bool loadFromFile(const std::string& filename);

    int getWidth() const;
    int getHeight() const;

    bool isInside(int x, int y) const;
    int getCell(int x, int y) const;
    bool isWall(int x, int y) const;
};


#endif //SOFTWAREGRAPHICSLABS_MAP_H