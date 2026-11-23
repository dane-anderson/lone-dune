// WorldView.h - everything drawn on the screen lives here.
#ifndef LONE_DUNE_WORLDVIEW_H
#define LONE_DUNE_WORLDVIEW_H
#include "graphics/Window.h"
#include "graphics/Rectangle.h"
#include "graphics/Circle.h"
#include "graphics/Line.h"
#include "graphics/Sprite.h"
#include "graphics/Text.h"
#include "World.h"
#include "Rover.h"
#include "Mission.h"

class WorldView {
public:
    WorldView(World& theWorld, Rover& theRover);
    // ---- Convert between map squares and screen positions ----
    Point cellToPixel(Location cell);
    bool mouseToCell(Point mouse, Location& cell);
    // ---- Read the saved route ----
    void changeRoutePage(int direction, Mission& mission);
    void showNextCommand(Mission& mission);
    // ---- Slide the picture between cells without changing the rover's rules ----
    void startMove(Location previousCell);
    void updateAnimation(double seconds);
    bool isMoving();
    // ---- Draw one frame ----
    void draw(Window& window, Mission& mission, int number, int total, bool automatic);
private:
    World& world;
    Rover& rover;
    double cellSize;
    double gridX;
    double gridY;
    int routePageStart = 0;
    const int commandsPerPage = 20;

    // Load the four directions once. Only the picture moves between grid cells.
    Image roverImage;
    Point roverPosition;
    Point moveStart;
    Point moveEnd;
    double moveProgress = 1;
    const double moveDuration = 0.22;

    // ---- Choose the colors ----
    Color background = Color(15, 23, 27);
    Color panel = Color(23, 33, 37);
    Color paper = Color(235, 224, 201);
    Color gold = Color(231, 172, 93);
    Color blue = Color(109, 203, 209);
    Color muted = Color(151, 157, 149);
    Color border = Color(59, 70, 68);

    // ---- Draw the required parts ----
    void drawMap(Window& window);
    void drawPath(Window& window, Mission& mission);
    void drawGoal(Window& window, Mission& mission);
    void drawHover(Window& window, Mission& mission);
    void drawRover(Window& window);
    void drawPanel(Window& window, Mission& mission, int number, int total, bool automatic);
    string commandName(char command);

    // ---- Draw a box or a line of text ----
    void box(Window& window, double x, double y, double width, double height, Color fill, Color outline);
    void words(Window& window, string message, double x, double y, int size, Color color);
};
#endif
