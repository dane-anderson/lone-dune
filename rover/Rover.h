// Rover.h - the rover's position, heading, battery, and samples.
// Based on William Wright's recitation rover.
#ifndef CSCI1300_CS1_ROVER_H
#define CSCI1300_CS1_ROVER_H
#include <string>
#include "Direction.h"
#include "Location.h"
#include "World.h"
using namespace std;

class Rover {
public:
    Rover(World& theWorld, Location initialLocation, Direction initialHeading, int initialBatteryLevel);
    // ---- Run one command ----
    bool executeCommand(char command);
    string getCommandStatus(char command, bool successful);
    void printCommandStatus(char command, bool successful);
    // ---- Read the rover's condition ----
    Location getLocation();
    Direction getHeading();
    string getHeadingName();
    int getBattery();
    int getSamples();
    int getLastScanCount();
    string getLastResult();
    bool isReset();
    string toString();
private:
    // Share the map: drilling must also change the terrain drawn on screen.
    World& world;
    Location location;
    Direction heading;
    int batteryLevel;
    int samples = 0;
    int lastScanCount = 0;
    string lastResult = "Awaiting first command.";
    string rejectionReason;
    bool canPay(int cost);
    bool move(bool backward);
    bool turn(bool left);
    bool scan();
    bool drill();
    bool charge();
};
#endif
