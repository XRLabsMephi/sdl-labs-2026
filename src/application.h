#pragma once
#include <array>
#include "texturestore.h"

struct SDL_Window;
struct SDL_Renderer;
class Playfield;

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

	std::array<Texture, 64> textures_;


	bool launch_SDL();
	void close_SDL();
	void load_textures();
	void load_hub(Playfield& play_field);
};