
# Software Rendering - Lab 1

This module is responsible for storing and displaying the
software-rendered image.

## Coordinate system

- Origin: top-left corner
- X increases to the right
- Y increases downwards
- Image size: width x height
- Pixel (x, y) is stored at index y * width + x

## Color

Pixels use 8-bit RGBA components.

The same Color type is used by FrameBuffer and SDL
presentation. All colors in Lab 1 are opaque.

## FrameBuffer

FrameBuffer stores the image in CPU memory.

Public operations:

- clear(color) - fill the entire image
- setPixel(x, y, color) - write one pixel
- fillRect(x, y, w, h, color) - fill a rectangle
- drawColumn(x, yBegin, yEnd, color) - draw a wall column
- data() - access the final pixel array

Drawing coordinates outside the image are clipped.

Rectangles use width and height.
Vertical columns use [yBegin, yEnd).

## SdlPresenter

SdlPresenter receives an existing SDL_Renderer and
copies FrameBuffer pixels into an SDL_Texture.

Application owns SDL_Window and SDL_Renderer.
SdlPresenter owns its SDL_Texture.

The framebuffer and SDL texture must have equal sizes.

## Module boundaries

Raycaster:
- Calculates wall geometry
- Writes wall columns into FrameBuffer
- Does not depend on SDL

Application:
- Processes SDL events
- Updates the player
- Calls Raycaster
- Calls SdlPresenter

FrameBuffer:
- Stores pixels
- Implements basic drawing operations
- Does not depend on SDL
