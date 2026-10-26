#include "sdl_presenter.h"

#include <SDL3/SDL.h>

#include "framebuffer.h"

SdlPresenter::SdlPresenter(SDL_Renderer* renderer, int width, int height)
	: renderer_(renderer) 
{
	texture_ = SDL_CreateTexture(
		renderer_, // то кто будет отрисовывать
		SDL_PIXELFORMAT_ARGB8888, // формат в котором будет храниться каждый пиксель
		SDL_TEXTUREACCESS_STREAMING, 
		// формат работы с данными в SDL. В силу частных обновлений, которые нельзя получить
		// "дорисовыванием" прошлого кадра пользуемся streaming.
		width, // ширина
		height // высота
	);

	if (texture_ != nullptr) {
		// хотим чтобы новый кадр полностью перекрывал предыдущий, а не смешивался
		SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_NONE); 
	}
}

SdlPresenter::~SdlPresenter() {
	if (texture_ != nullptr) SDL_DestroyTexture(texture_);
}

bool SdlPresenter::present(const FrameBuffer& fb) {
	if (texture_ == nullptr) return false;
	if (!SDL_UpdateTexture(texture_, nullptr, fb.data(), fb.pitch())) return false;
	SDL_RenderTexture(renderer_, texture_, nullptr, nullptr);
	SDL_RenderPresent(renderer_);
	return true;
}