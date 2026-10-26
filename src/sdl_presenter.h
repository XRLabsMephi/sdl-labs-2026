#pragma once

struct SDL_Renderer;
struct SDL_Texture;
class FrameBuffer;

/*
	Есть кадр, сформированный в FrameBuffer, хотим вывести через Present.

	Для этого получаем массив пикселей из буфера и выводим их SDL-ем.
*/
class SdlPresenter {
public:
	SdlPresenter(SDL_Renderer* renderer, int width, int height);
	~SdlPresenter();

	SdlPresenter(const SdlPresenter&) = delete;
	SdlPresenter& operator=(const SdlPresenter&) = delete;

	// вывести на экран наш кадр, который был честно собран в буфере
	// true — кадр успешно выведен.
	bool present(const FrameBuffer& fb);

private:
	SDL_Renderer* renderer_ = nullptr;
	SDL_Texture* texture_ = nullptr;
};