
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

## Map

Map stores the world as a rectangular grid of integer cells.
It does not depend on SDL.

The Map class belongs to namespace wolf.

### Coordinates and cell values

- Cell coordinates are integers.
- X increases to the right.
- Y increases downwards.
- Public methods receive coordinates in the order (x, y).
- Internally, cells are stored as cells[y][x].
- 0 means empty space.
- A positive value is a wall identifier.
- Negative values are not allowed.

Wall identifiers can be used to select wall colors in Lab 1
and wall textures in Lab 2.

### Public operations

- Map(cells) - create a map from a two-dimensional array.
- load(filename) - create a map from a text file.
- get_width() - return the number of columns.
- get_height() - return the number of rows.
- in_bounds(x, y) - check whether the cell exists.
- wall_id(x, y) - return the wall identifier, or 0 for empty space.
- is_wall(x, y) - check whether the cell is an obstacle.

The constructor checks that the map is not empty, all rows
have equal lengths, and cell values are non-negative.

### Boundary behavior

Coordinates outside the map are treated as obstacles:

- in_bounds(x, y) returns false.
- wall_id(x, y) returns 1.
- is_wall(x, y) returns true.

The value 1 outside the map represents a virtual boundary wall.

### Map file format

Maps are stored in assets/wolf/maps/.
The initial map is assets/wolf/maps/map01.txt.

Each non-empty line contains one row.
Cell values are integers separated by whitespace.
Empty lines are ignored.

Example:

    1 1 1 1
    1 0 0 1
    1 0 2 1
    1 1 1 1

load(filename) expects a correctly formatted integer grid.
A relative filename is resolved from the application's
working directory.

### Module boundaries

Map:
- Stores cells and map dimensions.
- Loads map data from a text file.
- Provides read-only access through public methods.
- Does not process input or render the scene.

Raycaster and PlayerController use Map methods instead
of accessing its internal array directly.

## Player and input

Vec2, Player, InputState and PlayerController belong to
namespace wolf and do not depend on SDL.

Player stores:
- position in world units;
- a unit direction vector;
- a perpendicular cameraPlane vector.

One map cell corresponds to one world unit.
The initial position is (1.5, 1.5).
The initial direction is (1, 0).
The horizontal field of view is 60 degrees.

InputState stores movement, rotation and quit requests.
Application fills InputState from SDL input.

PlayerController updates Player using Map, InputState
and dt measured in seconds.

Movement speed is 3 world units per second.
Rotation speed is 2 radians per second.
Diagonal movement is normalized.

Collision checks use an axis-aligned square with
half-size 0.2 around the player position.
The initial player position must fit in empty space.

Movement is checked separately along X and Y.
Long movements are split into smaller steps.

PlayerController does not handle the quit request.
Application handles application shutdown.

## SDL input

read_input() converts SDL events and keyboard state
into InputState.

Controls:
- W / S: move forward / backward.
- A / D: strafe left / right.
- Left / Right arrows: rotate the camera.
- Escape or window close: quit.

SDL input handling is implemented in SdlInput.cpp.
PlayerController remains independent of SDL.

## Application

Application starts SDL and owns SDL_Window and SDL_Renderer.

Scene objects are created inside run_loop():
- Map;
- Player;
- PlayerController;
- FrameBuffer;
- Raycaster;
- SdlPresenter.

Each frame:
1. Read input.
2. Calculate dt in seconds.
3. Update the player.
4. Draw the ceiling and floor.
5. Render walls.
6. Present the completed framebuffer.

dt is limited to 0.1 seconds to avoid large movements
after long pauses.

The framebuffer and window size is 640 x 480.

The initial map is loaded from:
assets/wolf/maps/map01.txt

Run graphics_lab with the repository root as its
working directory.

SdlPresenter is destroyed before SDL_Renderer.
SDL_Renderer is destroyed before SDL_Window.
SDL_Quit is called after SDL resources are released.

Application::run() is called once per application instance.

## Lab 2 conventions

The following conventions define the interfaces for Lab 2.
All new types belong to namespace wolf.

### Responsibilities

Participant 1:
- TextureStore and resource loading.
- Texture identifiers and resource paths.
- Sprite placement and scene preparation.
- Integration in Application.

Participant 2:
- Wall texture coordinates and wall texture rendering.
- Wall depth buffer.
- Compatible depth values for walls and sprites.

Participant 3:
- Texture and texture sampling.
- Sprite and SpriteRenderer.
- Sprite projection, sorting and transparency.
- Fog shared by wall and sprite rendering.

### Pixel format and Texture

Textures use the existing Color type from wolf/core/Color.hpp.
Pixels are stored in CPU memory in row-major order:
pixels[y * width + x].

The texture origin is the top-left corner.
X increases to the right, Y increases downwards.

Texture has the following interface:

```cpp
struct Texture {
    int width = 0;
    int height = 0;
    std::vector<Color> pixels;

    Color sample(int x, int y) const;
};
```

A successfully loaded texture has positive dimensions and
exactly width * height pixels.

sample() returns a pixel by value.
For coordinates outside the texture, it returns
Color{0, 0, 0, 0} without accessing invalid memory.

Texture does not contain SDL objects and does not depend on SDL.
Texture coordinates use nearest-neighbor sampling.

### TextureStore

TextureStore owns the loaded textures.
Its public interface is:

```cpp
class TextureStore {
public:
    void load(int id, const std::string& filename);
    const Texture& get(int id) const;

private:
    std::map<int, Texture> textures_;
};
```

load() loads a BMP file and stores it under the given identifier.
Identifiers must be positive and unique.
Loading an existing identifier is an error.

get() returns a const reference to an existing texture.
It does not load files and does not create missing textures.

An invalid identifier, missing resource, invalid image or
duplicate identifier causes std::runtime_error with a
descriptive message.

Application::run() handles these errors.

SDL loading and conversion are implemented in TextureStore.cpp.
The public header does not expose SDL types.

The loader converts images to SDL_PIXELFORMAT_RGBA32 and
copies rows into Texture, respecting SDL_Surface::pitch.
Temporary SDL surfaces are released after loading,
including error paths.

Textures are loaded before the main loop.
They are not replaced or removed during rendering.
Renderers receive TextureStore through a const reference.

### Resource identifiers and paths

Identifiers 1-99 are reserved for wall textures.
Identifiers 100 and above are reserved for sprite textures.
Zero is not a texture identifier.

Map cell values keep their existing meaning:
- 0: empty space.
- Positive value: wall identifier.

A wall identifier directly selects the same texture identifier.

The current map uses wall identifiers 1-5.
All five textures must be loaded:

- 1: assets/wolf/textures/wall01.bmp
- 2: assets/wolf/textures/wall02.bmp
- 3: assets/wolf/textures/wall03.bmp
- 4: assets/wolf/textures/wall04.bmp
- 5: assets/wolf/textures/wall05.bmp

Initial sprite identifiers:

- 101: assets/wolf/sprites/barrel.bmp
- 102: assets/wolf/sprites/lamp.bmp
- 103: assets/wolf/sprites/medkit.bmp

Resource paths are relative to the repository root.
The application is run with the repository root as
its working directory.

### Sprite transparency

Transparency is stored in Color::a.
TextureStore preserves the alpha channel of source images.

Initial sprite assets use binary alpha:
- 0: fully transparent.
- 255: fully opaque.

SpriteRenderer skips pixels with alpha 0 before applying fog.
Skipped pixels do not modify FrameBuffer.
Partial alpha blending is not required initially.

Sprite assets must provide transparency supported by the loader.
Images without an alpha channel are treated as fully opaque.
No special RGB color is reserved for transparency.

### Sprite data and scene ownership

Sprite has the following interface:

```cpp
struct Sprite {
    Vec2 position;
    int textureId = 0;
    double scale = 1.0;
};
```

position is expressed in the same world coordinates as Player.
One map cell equals one world unit.
The center of cell (x, y) is (x + 0.5, y + 0.5).

scale is a positive finite multiplier.
At scale 1.0, the sprite has a world height of one unit.
Its width follows the texture aspect ratio.

Application owns TextureStore and std::vector<Sprite>.
The initial sprite list is created before the main loop.
A separate scene file loader is not required initially.

Sprites are placed in empty map cells with finite coordinates.
Each sprite references a loaded texture.
Initial sprites do not participate in player collision checks.

### Depth contract

Wall and sprite depth means forward distance along the
unit camera direction, not Euclidean distance to the object.

For a sprite:

```cpp
Vec2 relative{
    sprite.position.x - player.position.x,
    sprite.position.y - player.position.y
};

double depth =
    relative.x * player.direction.x +
    relative.y * player.direction.y;
```

Sprites with depth <= 0.0001 are not rendered.

Raycaster fills one depth value for each framebuffer column.
The depth buffer size equals FrameBuffer::width().
A column without a wall receives positive infinity.

A sprite column is drawn only when:

```cpp
spriteDepth < depthBuffer[x]
```

Sprites are drawn from farthest to nearest using camera depth.

### Fog

Fog has the following interface:

```cpp
struct Fog {
    double start = 5.0;
    double end = 15.0;
    Color color{100, 100, 100, 255};

    Color apply(Color source, double depth) const;
};
```

The configuration must satisfy:
0 <= start < end, with both values finite.

The fog factor is:

```cpp
factor = std::clamp(
    (depth - start) / (end - start),
    0.0,
    1.0
);
```

RGB components are interpolated between source and fog color.
The source alpha component is preserved.

The same Fog object and depth convention are used for
walls and sprites.

### Rendering interfaces

The Lab 2 wall rendering interface is:

```cpp
void Raycaster::render(
    FrameBuffer& frame,
    const Map& map,
    const Player& player,
    const TextureStore& textures,
    const Fog& fog
);
```

Raycaster::getDepthBuffer() keeps its existing interface.

SpriteRenderer provides:

```cpp
void render(
    FrameBuffer& frame,
    const std::vector<Sprite>& sprites,
    const Player& player,
    const TextureStore& textures,
    const std::vector<double>& depthBuffer,
    const Fog& fog
) const;
```

Texture, Sprite, SpriteRenderer, Raycaster and Fog do not
depend on SDL.

### Frame order

Before the main loop:
1. Load the map.
2. Create and validate the player.
3. Load wall and sprite textures.
4. Prepare the sprite list.
5. Create renderers and framebuffer.

Each frame:
1. Process SDL events and read input.
2. Calculate dt.
3. Apply camera rotation and player movement.
4. Draw the ceiling and floor.
5. Render textured walls and fill the depth buffer.
6. Render sprites using the wall depth buffer.
7. Draw the crosshair over the completed scene.
8. Present FrameBuffer through SdlPresenter.

Mouse rotation is applied before calculating movement
for that frame.
