
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
