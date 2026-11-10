#include <cmath>
#include <array>
#include <random>
#include <algorithm>
#include "game_director.h"

/*
	Файл с реализацией функций из game_director.h
*/


std::pair<std::array<int, 5>, double> Director::generate_input() {
	
	if (last.max_time == -1) {
		last = base_state;
		return { base_state.target_numbers, 30 };
	}

	if (cur.max_time == -1) {
		cur = base_state;
		return { base_state.target_numbers, 30 };
	}

	std::array<int, 5> ret;
	double max_t = cur.max_time;

	//construct ret
	//w, h, max_food, delt, teleport count

	double prev_delt = last.left_time - TARGET_TIME_LEFT;
	double cur_delt = cur.left_time - TARGET_TIME_LEFT;

	// чем ближе к искомой точке, тем меньше надо делать шаги, чтобы 
	// не ломать работующее
	double k = 0.2;
	if (std::abs(cur_delt) - std::abs(prev_delt) < 0) { // Замедляем усложнение
		k *= 0.8;
	}
	//иначе берем k с прошлого шага
	ret = cur.target_numbers;

	double target_k;
	if (cur_delt > 0) { // make harder
		target_k = 1 + k;
		max_t *= (1.0 - k);
	} 
	else { // make easer
		target_k = 1 - k;
		max_t *= (1.0 + k);
	}
	max_t = std::clamp(max_t, 15.0, 60.0);

	ret[0] = std::max(6, static_cast<int>(std::round(ret[0] * target_k)));
	ret[1] = std::max(6, static_cast<int>(std::round(ret[1] * target_k)));
	ret[0] = std::min(50, ret[0]);
	ret[1] = std::min(50, ret[1]);

	double food_k = 1.0 + (target_k - 1.0) * 0.5;
	if (int(ret[2] * food_k) == ret[2]) {
		if (cur_delt > 0) {
			ret[2]++;
		}
		else {
			ret[2]--;
		}
	}
	else {
		ret[2] = ret[2] * food_k;
	}
	ret[3] = 0;

	if (target_k < 0.2) {
		ret[4]++;
	}

	ret[2] = std::max(1, ret[2]); // Хотя бы 1 еда
	while (target_k < 1) {
		target_k *= 10;
		ret[4]++;
	}
	ret[4] = std::clamp(ret[4], 1, 3);
	
	//push ret into temporary
	last = cur;
	
	cur.target_numbers = ret;
	cur.max_time = max_t;
	cur.left_time = -1;
	return { ret, max_t };
}


void Director::push_result(double remain_t) {
	cur.left_time = remain_t;
}

void Director::clear() {
	cur = state();
	last = state();
}