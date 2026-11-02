#pragma once
/*
	файл c определением класса Player

	нужен для реализации коллизий игрока

	игрок представляет собой окружность, по которой распределено 4 точки, которые будут проверяться на коллизию
	и корректировать направление движения
*/
class Playfield;

class Player {
public:
	
	//конструктор по умолчанию
	Player(double x, double y, double A, double r);
	
	//проверка касания игроком коллизий
	bool CheckCollision(double x, double y, Playfield& pf);

	//смещение вперед
	void forward(Playfield& pf, double deltaTime);

	//смещение назад
	void back(Playfield& pf, double deltaTime);

	//повернуть налево
	void turn_left(double deltaTime);

	//повернуть направо
	void turn_right(double deltaTime);

	//налево
	void left(Playfield& pf, double deltaTime);

	//направо
	void right(Playfield& pf, double deltaTime);

	double get_A();
	double get_X();
	double get_Y();

private:
	double x, y; // нынешнее положение
	double A; // нынешнее направление

	double r; // радиус круга, описывающий игрока
};
