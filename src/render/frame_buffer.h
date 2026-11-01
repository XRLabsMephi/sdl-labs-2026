#ifndef SOFTWAREGRAPHICSLABS_RENDER_FRAME_BUFFER_H_
#define SOFTWAREGRAPHICSLABS_RENDER_FRAME_BUFFER_H_

#include <vector>
#include <cstdint>
#include "core/color.h"

class FrameBuffer {
public:
    FrameBuffer(int width, int height);

    void Clear(Color ceiling_color, Color floor_color);
    void SetPixel(int x, int y, Color c);
    void DrawVerticalLine(int x, int start_y, int end_y, Color c);

    int GetWidth() const;
    int GetHeight() const;

    // Возвращает сырой указатель на массив uint32_t для передачи в SDL_UpdateTexture
    const uint32_t* GetPixelData() const;

private:
    int width_;
    int height_;
    // Храним сырые байты, чтобы SDL было удобно их читать
    std::vector<uint32_t> pixels_;
};

#endif  // SOFTWAREGRAPHICSLABS_RENDER_FRAME_BUFFER_H_