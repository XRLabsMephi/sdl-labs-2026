#include "interface.h"
#include "framebuffer.h"
#include "settings.h"

#include <iostream>
#include <algorithm>
#include <cstddef>
#include <cstring>

/*
	Файл с реализацией из interface.h
*/

//нарисовать прогресс бар с прогрессом [0:1]
void draw_interface(FrameBuffer& fb, double progress) {
	int start = 0;
	int h = 20;
	for (int i = 0; i <= fb.width() * progress; i++) {
		for (int j = fb.height(); j >= fb.height() - h; j--) {
			fb.setPixel(i, j, Color{ 255, 10, 0, 255 });
		}
	}
}