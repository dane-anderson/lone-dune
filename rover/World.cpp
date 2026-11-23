// World.cpp - read the terrain and keep track of sample sites.
#include "World.h"
#include <fstream>
#include <sstream>
using namespace std;

World::World() {
    for (int y = 0; y < 20; y++) {
        for (int x = 0; x < 20; x++) {
            terrain[y][x] = '.';
            revealed[y][x] = false;
        }
    }
}

// ---- Check a map size before using it ----
bool World::readDimension(string text, int& number) {
    if (text.empty()) return false;
    number = 0;
    for (char digit : text) {
        if (digit < '0' || digit > '9') return false;
        number = number * 10 + (digit - '0');
        // Stop before a large input can overflow an integer.
        if (number > 20) return false;
    }
    return number >= 1;
}

// ---- Read and check the map ----
bool World::load(string filename, string& error) {
    width = 0;
    height = 0;
    error = "";
    ifstream file(filename);
    if (!file.is_open()) {
        error = "Cannot open world file: " + filename;
        return false;
    }
    string line, widthText, heightText, extra;
    getline(file, line);
    istringstream sizeLine(line);
    int newWidth = 0, newHeight = 0;
    if (!(sizeLine >> widthText >> heightText) || (sizeLine >> extra)
        || !readDimension(widthText, newWidth) || !readDimension(heightText, newHeight)) {
        error = "World line 1: width and height must be whole numbers from 1 to 20.";
        return false;
    }
    bool foundCharger = false;
    for (int y = 0; y < newHeight; y++) {
        int lineNumber = y + 2;
        if (!getline(file, line)) {
            error = "World line " + to_string(lineNumber) + ": missing map row.";
            return false;
        }
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (static_cast<int>(line.size()) != newWidth) {
            error = "World line " + to_string(lineNumber) + ": row must have "
                  + to_string(newWidth) + " characters.";
            return false;
        }
        for (int x = 0; x < newWidth; x++) {
            char tile = line[x];
            if (tile != '.' && tile != '#' && tile != '~' && tile != 'C' && tile != '*') {
                error = "World line " + to_string(lineNumber) + ": unknown terrain '" + tile + "'.";
                return false;
            }
            terrain[y][x] = tile;
            revealed[y][x] = false;
            if (tile == 'C' && !foundCharger) {
                firstCharger = Location(x, y);
                foundCharger = true;
            }
        }
    }
    if (getline(file, line)) {
        error = "World line " + to_string(newHeight + 2) + ": extra map row.";
        return false;
    }
    if (!foundCharger) {
        error = "World: the map needs at least one charging station (C).";
        return false;
    }
    width = newWidth;
    height = newHeight;
    return true;
}

// ---- Ask about a square ----
int World::getWidth() { return width; }
int World::getHeight() { return height; }
Location World::getFirstChargingStation() { return firstCharger; }
bool World::isOutOfBounds(Location& location) {
    return !location.isWithinBounds(width, height);
}
char World::getTerrain(int x, int y) {
    Location location(x, y);
    if (isOutOfBounds(location)) return '#';
    return terrain[y][x];
}
bool World::isLocationAChargingStation(Location& location) {
    return !isOutOfBounds(location) && getTerrain(location.getX(), location.getY()) == 'C';
}
bool World::isRevealed(int x, int y) {
    Location location(x, y);
    return !isOutOfBounds(location) && revealed[y][x];
}
bool World::isKnownSample(Location location) {
    return getTerrain(location.getX(), location.getY()) == '*'
        && isRevealed(location.getX(), location.getY());
}

// ---- Discover and collect samples ----
int World::revealNearby(Location center) {
    int found = 0;
    // Include diagonals and the rover's own square.
    for (int y = center.getY() - 1; y <= center.getY() + 1; y++) {
        for (int x = center.getX() - 1; x <= center.getX() + 1; x++) {
            if (getTerrain(x, y) == '*' && !isRevealed(x, y)) {
                revealed[y][x] = true;
                found++;
            }
        }
    }
    return found;
}
bool World::collectSample(Location location) {
    if (!isKnownSample(location)) return false;
    terrain[location.getY()][location.getX()] = '.';
    revealed[location.getY()][location.getX()] = false;
    return true;
}
