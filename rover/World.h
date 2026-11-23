// World.h - the map and what has been discovered on it.
// Based on the final-project starter by William Wright and Andy Carle.
#ifndef CSCI1300_CS1_WORLD_H
#define CSCI1300_CS1_WORLD_H
#include <string>
#include "Location.h"
using namespace std;

class World {
public:
    World();
    // ---- Read and check the map ----
    bool load(string filename, string& error);
    int getWidth();
    int getHeight();
    Location getFirstChargingStation();
    // ---- Ask about a square ----
    bool isOutOfBounds(Location& location);
    bool isLocationAChargingStation(Location& location);
    char getTerrain(int x, int y);
    bool isRevealed(int x, int y);
    bool isKnownSample(Location location);
    // ---- Discover and collect samples ----
    int revealNearby(Location center);
    bool collectSample(Location location);
private:
    bool readDimension(string text, int& number);
    int width = 0;
    int height = 0;
    char terrain[20][20];
    bool revealed[20][20];
    Location firstCharger;
};
#endif
