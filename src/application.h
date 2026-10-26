#pragma once
#include <array>
#include "texturestore.h"

struct SDL_Window;
struct SDL_Renderer;

class Application {
public:
	Application();
	~Application();

	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	int run();

private:
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;

	std::array<Texture, 8> textures_;
	std::vector<double> depthBuffer_;
};