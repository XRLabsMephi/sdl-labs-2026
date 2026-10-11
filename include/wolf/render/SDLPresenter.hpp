#pragma once

#include "wolf/render/FrameBuffer.hpp"
#include <SDL3/SDL.h>
#include <string>

/*
SDLPresenter - класс, который отвечает за взаимодействие с SDL3 и графическим окном.
Он связывает FrameBuffer с аппаратным ускорением видеокарты,
создавая окно, рендерер и текстуру для вывода готового кадра на экран.
*/

class SDLPresenter {
    SDL_Window* window_ = nullptr; //указатель на окно SDL
    SDL_Renderer* renderer_ = nullptr; //указатель на аппаратный рендерер SDL
    SDL_Texture* texture_ = nullptr; //текстура в GPU

    size_t width_ = 0;
    size_t height_ = 0;

public:
    SDLPresenter(const std::string& title, size_t width, size_t height); //создаёт окно заданного размера
    ~SDLPresenter();

    SDLPresenter(const SDLPresenter&) = delete;
    SDLPresenter& operator=(const SDLPresenter&) = delete;

    void present(const FrameBuffer& buffer); //переносит пиксели из буфера в текстуру SDL

    [[nodiscard]] size_t get_width() const { return width_; }
    [[nodiscard]] size_t get_height() const { return height_; }
};