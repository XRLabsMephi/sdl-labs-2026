#pragma once
/*
	Глобальные настройки

	по возможности, сюда надо вытаскивать все глобальные константы для более удобной настройки

	map_scale		- Размер карты
	rotate_scale	- Скорость поворота камеры
	moving_scale	- Скорость перемещения
	screenWidth		- Ширина окна, в котором все отрисовывается
	screenHeight	- Высота окна, в котором все отрисовывается
*/

namespace settings {
	constexpr double map_scale      = 100;
	constexpr double rotation_scale = 1;
	constexpr double moving_scale   = 2;
	constexpr double screenWidth	= 800;
	constexpr double screenHeight   = 600;
}