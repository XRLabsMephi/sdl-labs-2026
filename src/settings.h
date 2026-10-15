#pragma once

/*
	Глобальные настройки

	по возможности, сюда надо вытаскивать все глобальные константы для более удобной настройки

	map_scale - Размер карты
	rotate_scale - Скорость поворота камеры
	moving_scale - Скорость перемещения
*/

namespace settings {

	double map_scale = 100;
	double rotation_scale = 0.5;
	double moving_scale = 0.2;
}