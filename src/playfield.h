#pragma once
#include <vector>
#include <iostream>
#include "board.h"
#include "sprite.h"
/*
	файл c определением класса Playfield

	представляет собой 2мерный массив, поддерживающий в себе карту расположения обьектов

	0 - пустота
	1 - стенка; текстура 1 (доски)
	2 - стенка; текстура 2 (бревно)
	3 - стенка; текстура 3 (обтесання древесина)
	4 - спрайт; спрайт 1
	5 - спрайт; спрайт 2
	P - spawn
	E - точка выхода
*/

class Playfield {
public:
	
	std::vector<Board> boards; //массив всех уже созданых коробок
	std::vector<Sprite> sprites; // массив всех уже созданных спрайтов

	std::vector<Sprite> teleports;
	
	//конструктор с заданием размеров
	// !!! ПО УМОЛЧАНИЮ ЗАПОЛНЯЕТ ГРАНИЦЫ ПОЛЯ СТЕНАМИ !!!
	Playfield(int _w, int _h, int _k); 

	//установаить в прямоугольнике с заданными параметрами заданный тип 
	//координаты включительно, в 1-индексанции
	//TO_DO на данный момент можно только добавлять стенки, но не убирать
	void set_rectangle(int _x1, int _y1, int _x2, int _y2, int tp);
	std::pair<int, int> get_player_coords();
	//загрузка карты из файла txt
	void load_map(const std::string& file);

	//выводит игровое поле в консоль
	void print();

	std::vector<std::vector<int>> getField();
	int getHight();
	int getWidth();
	int getMax_food();
	std::pair<int, int> getPlayer_coords();
private:
	int max_food = 0;
	int h; //высота
	int w; //ширина
	int k; //длина стороны клетки
	std::pair<int, int> player_coords;
	std::vector<std::vector<int>> field; //поле в 2-мерном виде
};
