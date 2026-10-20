#pragma once
/*
	файл c определением класса Player

	нужен для реализации коллизий игрока

	игрок представляет собой окружность, по которой распределено 4 точки, которые будут проверяться на коллизию
	и корректировать направление движения
*/

class Player {
public:
	
	//конструктор по умолчанию
	Player(double x, double y, double A, double r);

	//проверка касания игроком коллизий
	bool CheckCollision(double x, double y, Playfield& pf);

	//смещение вперед
	void forward(Playfield& pf);

	//смещение назад
	void back(Playfield& pf);

	//поворт налево
	void left();

	//поворот направо
	void right();

	double get_A();
	double get_X();
	double get_Y();

private:
	double x, y; // нынешнее положение
	double A; // нынешнее направление

	int r; // радиус круга, описывающий игрока
};
