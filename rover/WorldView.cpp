// WorldView.cpp - draw a simple Dune-themed rover expedition.
#include "WorldView.h"
using namespace std;

// ---- Fit the whole map into its space ----
WorldView::WorldView(World& theWorld, Rover& theRover)
    : world(theWorld), rover(theRover), roverImage("arrakis_rover.png") {
    cellSize = 624.0 / world.getWidth();
    double heightLimit = 480.0 / world.getHeight();
    if (heightLimit < cellSize) cellSize = heightLimit;
    gridX = 36 + (624 - world.getWidth() * cellSize) / 2;
    gridY = 210 + (480 - world.getHeight() * cellSize) / 2;
    roverPosition = cellToPixel(rover.getLocation());
}

// ---- Convert between map squares and screen positions ----
Point WorldView::cellToPixel(Location cell) {
    return Point(gridX + cell.getX() * cellSize, gridY + cell.getY() * cellSize);
}
bool WorldView::mouseToCell(Point mouse, Location& cell) {
    // Check the edges before converting a decimal position to a whole square.
    if (mouse.x < gridX || mouse.y < gridY
        || mouse.x >= gridX + world.getWidth() * cellSize
        || mouse.y >= gridY + world.getHeight() * cellSize) return false;
    cell = Location(static_cast<int>((mouse.x - gridX) / cellSize),
                    static_cast<int>((mouse.y - gridY) / cellSize));
    return true;
}

// ---- Begin a short slide after a successful move ----
void WorldView::startMove(Location previousCell) {
    Location nextCell = rover.getLocation();
    if (previousCell.equals(nextCell)) return;
    moveStart = cellToPixel(previousCell);
    moveEnd = cellToPixel(nextCell);
    roverPosition = moveStart;
    moveProgress = 0;
}

// ---- Advance the picture using elapsed seconds, not the number of frames ----
void WorldView::updateAnimation(double seconds) {
    if (!isMoving()) return;
    if (seconds < 0) seconds = 0;
    moveProgress += seconds / moveDuration;
    if (moveProgress > 1) moveProgress = 1;
    // At 0 the picture is at the old cell; at 1 it is exactly at the new cell.
    roverPosition.x = moveStart.x + (moveEnd.x - moveStart.x) * moveProgress;
    roverPosition.y = moveStart.y + (moveEnd.y - moveStart.y) * moveProgress;
}
bool WorldView::isMoving() { return moveProgress < 1; }

// ---- Read every command in a long route ----
void WorldView::changeRoutePage(int direction, Mission& mission) {
    int length = static_cast<int>(mission.getCommands().size());
    int lastPage = ((length - 1) / commandsPerPage) * commandsPerPage;
    routePageStart += direction * commandsPerPage;
    if (routePageStart < 0) routePageStart = 0;
    if (routePageStart > lastPage) routePageStart = lastPage;
}
void WorldView::showNextCommand(Mission& mission) {
    int index = mission.getNextIndex();
    if (mission.isFinished()) index--;
    routePageStart = (index / commandsPerPage) * commandsPerPage;
}

// ---- Draw the title, map, and information panel ----
void WorldView::draw(Window& window, Mission& mission, int number, int total, bool automatic) {
    window.clear(background);
    box(window, 20, 20, 1040, 126, Color(49, 39, 35), gold);
    words(window, "LONE DUNE", 36, 35, 4, paper);
    words(window, "THE LAST SIGNAL", 38, 84, 2, gold);
    words(window, "Arrakis survey rover", 38, 120, 1, paper);
    words(window, "ROVER EXPEDITION", 790, 58, 2, gold);
    words(window, "Follow the saved survey commands.", 790, 98, 1, paper);

    box(window, 20, 166, 656, 566, panel, border);
    words(window, "ARRAKIS / SURVEY MAP", 36, 182, 1, gold);
    words(window, to_string(world.getWidth()) + " x " + to_string(world.getHeight()), 590, 182, 1, paper);
    drawMap(window);
    drawPath(window, mission);
    drawGoal(window, mission);
    drawHover(window, mission);
    drawRover(window);
    words(window, ". Ground   # Rock   ~ Sand   C Charger   * Revealed sample", 36, 712, 1, paper);
    drawPanel(window, mission, number, total, automatic);
    words(window, "Click chooses a goal. The saved command letters control the rover's route.", 32, 756, 1, gold);
    words(window, "Only F and B change squares. Turning, scanning, drilling, charging, and photos happen in place.",
          32, 778, 1, muted);
}

// ---- Draw the terrain and revealed samples ----
void WorldView::drawMap(Window& window) {
    // Number the columns and rows so a goal such as (4, 1) is easy to find.
    for (int x = 0; x < world.getWidth(); x++) {
        Point corner = cellToPixel(Location(x, 0));
        words(window, to_string(x), corner.x + cellSize / 2 - 3, gridY - 14, 1, muted);
    }
    for (int y = 0; y < world.getHeight(); y++) {
        Point corner = cellToPixel(Location(0, y));
        words(window, to_string(y), gridX - 15, corner.y + cellSize / 2 - 6, 1, muted);
    }
    for (int y = 0; y < world.getHeight(); y++) {
        for (int x = 0; x < world.getWidth(); x++) {
            char terrain = world.getTerrain(x, y);
            Color fill(126, 104, 73);
            string mark;
            if (terrain == '#') { fill = Color(57, 61, 57); mark = "#"; }
            if (terrain == '~') { fill = Color(181, 131, 69); mark = "~"; }
            if (terrain == 'C') { fill = Color(49, 86, 88); mark = "C"; }
            if (terrain == '*' && world.isRevealed(x, y)) mark = "*";
            // An unrevealed sample uses the same color and no mark, just like ground.
            Point corner = cellToPixel(Location(x, y));
            box(window, corner.x, corner.y, cellSize, cellSize, fill, background);
            words(window, mark, corner.x + cellSize / 2 - 3, corner.y + cellSize / 2 - 6, 1, paper);
        }
    }
}

// ---- Join the squares visited during this mission ----
void WorldView::drawPath(Window& window, Mission& mission) {
    const vector<Location>& path = mission.getPath();
    for (int i = 1; i < static_cast<int>(path.size()); i++) {
        Point start = cellToPixel(path[i - 1]);
        Point end = cellToPixel(path[i]);
        // The newest path segment follows the sliding picture.
        if (isMoving() && i == static_cast<int>(path.size()) - 1) end = roverPosition;
        Line line(Point(start.x + cellSize / 2, start.y + cellSize / 2),
                  Point(end.x + cellSize / 2, end.y + cellSize / 2), paper);
        line.draw(window);
    }
}

// ---- Mark the chosen goal ----
void WorldView::drawGoal(Window& window, Mission& mission) {
    if (!mission.hasGoal()) return;
    Point corner = cellToPixel(mission.getGoal());
    Circle goal(Point(corner.x + cellSize / 2, corner.y + cellSize / 2), cellSize * .38, Color::Transparent);
    goal.setOutlineColor(blue);
    goal.draw(window);
}

// ---- Preview a valid goal while the user is still allowed to choose one ----
void WorldView::drawHover(Window& window, Mission& mission) {
    if (!mission.canChooseGoal()) return;
    Location cell;
    if (!mouseToCell(window.getMousePosition(), cell)) return;
    if (world.getTerrain(cell.getX(), cell.getY()) == '#') return;
    Point corner = cellToPixel(cell);
    box(window, corner.x + 2, corner.y + 2, cellSize - 4, cellSize - 4,
        blue.withAlpha(35), blue);
}

// ---- Draw the Arrakis scout facing north, east, south, or west ----
void WorldView::drawRover(Window& window) {
    int frame = 0;
    switch (rover.getHeading()) {
        case Direction::NORTH: frame = 0; break;
        case Direction::EAST: frame = 1; break;
        case Direction::SOUTH: frame = 2; break;
        case Direction::WEST: frame = 3; break;
    }
    // The sheet has four pictures across, with spare transparent space above and below.
    int frameSize = roverImage.getWidth() / 4;
    int frameTop = (roverImage.getHeight() - frameSize) / 2;
    double size = cellSize * 0.94;
    double margin = (cellSize - size) / 2;
    Sprite scout(roverImage, Point(roverPosition.x + margin, roverPosition.y + margin), size, size);
    scout.setFrame(frame * frameSize, frameTop, frameSize, frameSize);
    scout.draw(window);
}

// ---- Explain each command in everyday words ----
string WorldView::commandName(char command) {
    switch (command) {
        case 'F': return "Move forward";
        case 'B': return "Move backward";
        case 'L': return "Turn left";
        case 'R': return "Turn right";
        case 'S': return "Scan nearby squares";
        case 'D': return "Collect this sample";
        case 'P': return "Take a photograph";
        case 'C': return "Recharge";
        default: return "Unknown command";
    }
}

// ---- Show the mission, battery, result, and controls on the right ----
void WorldView::drawPanel(Window& window, Mission& mission, int number, int total, bool automatic) {
    box(window, 696, 166, 364, 566, panel, border);
    words(window, "MISSION " + to_string(number) + " OF " + to_string(total), 716, 186, 2, paper);
    string goal = "Goal: click a square on the map";
    if (mission.hasGoal()) goal = "Goal: " + mission.getGoal().toString();
    words(window, goal, 716, 222, 1, blue);

    words(window, "SAVED COMMANDS", 716, 252, 1, muted);
    string commands = mission.getCommands();
    string visible = commands.substr(routePageStart, commandsPerPage);
    words(window, visible, 716, 272, 2, gold);
    if (static_cast<int>(commands.size()) > commandsPerPage) {
        words(window, "Left/Right: " + to_string(routePageStart + 1) + "-"
            + to_string(routePageStart + static_cast<int>(visible.size())) + " of " + to_string(commands.size()),
            716, 300, 1, muted);
    }
    string next = "Mission complete";
    if (!mission.isFinished()) {
        next = string("Next: ") + mission.getNextCommand() + " (" + to_string(mission.getNextIndex() + 1)
            + "/" + to_string(commands.size()) + ") " + commandName(mission.getNextCommand());
    }
    words(window, next, 716, 329, 1, paper);
    words(window, "Position " + rover.getLocation().toString() + " / " + rover.getHeadingName(), 716, 356, 1, muted);

    words(window, "BATTERY", 716, 391, 1, muted);
    words(window, to_string(rover.getBattery()) + " / 40", 922, 383, 2, paper);
    box(window, 716, 416, 324, 12, background, border);
    if (rover.getBattery() > 0) box(window, 718, 418, rover.getBattery() * 8, 8, blue, blue);
    words(window, "SAMPLES COLLECTED: " + to_string(rover.getSamples()), 716, 444, 1, paper);
    string result = "Awaiting first command.";
    if (mission.getNextIndex() > 0) result = rover.getLastResult();
    words(window, result, 716, 475, 1, paper);

    words(window, "WHAT TO DO NEXT", 716, 520, 1, blue);
    string instruction = "CLICK A GOAL";
    string explanation = "Then press Enter to run the saved route.";
    if (mission.hasGoal()) {
        instruction = "PRESS ENTER";
        explanation = "Run the route, or use Space for one step.";
    }
    if (automatic) {
        instruction = "RUNNING ROUTE";
        explanation = "One saved command every 0.3 seconds.";
    }
    if (mission.isFinished()) {
        instruction = "PRESS N";
        explanation = "Next mission: choose a goal, then Enter.";
        if (number == total) {
            instruction = "JOURNEY COMPLETE";
            explanation = "Press R to start a new journey.";
        }
    }
    if (isMoving()) {
        instruction = "ROVER MOVING";
        explanation = "Finishing this step across the sand.";
    }
    words(window, instruction, 716, 542, 2, paper);
    words(window, explanation, 716, 566, 1, paper);
    if (mission.isFinished()) words(window, mission.wasGoalReached() ? "Goal reached." : "Goal not reached.", 716, 587, 1, gold);

    words(window, "YOUR CONTROLS", 716, 608, 1, gold);
    words(window, "Hover to preview. Click to choose a goal.", 716, 630, 1, paper);
    words(window, "Space: next command   Enter: run the rest", 716, 651, 1, paper);
    words(window, "N: next mission when this one finishes", 716, 672, 1, paper);
    words(window, "R: restart expedition   Escape: close", 716, 693, 1, paper);
}

// ---- Small drawing helpers ----
void WorldView::box(Window& window, double x, double y, double width, double height, Color fill, Color outline) {
    Rectangle rectangle(Point(x, y), width, height, fill);
    rectangle.setOutlineColor(outline);
    rectangle.draw(window);
}
void WorldView::words(Window& window, string message, double x, double y, int size, Color color) {
    Text text(message, Point(x, y), color, size);
    text.draw(window);
}
