// Rover.cpp - check an action completely before changing the rover.
#include "Rover.h"
#include <iostream>
using namespace std;

Rover::Rover(World& theWorld, Location initialLocation, Direction initialHeading, int initialBatteryLevel)
    : world(theWorld), location(initialLocation), heading(initialHeading), batteryLevel(initialBatteryLevel) {}

// ---- Check the battery ----
bool Rover::canPay(int cost) {
    if (batteryLevel < cost) {
        rejectionReason = "insufficient battery";
        return false;
    }
    return true;
}

// ---- Move across the ground ----
bool Rover::move(bool backward) {
    Location next = location;
    int step = 1;
    if (backward) step = -1;
    switch (heading) {
        case Direction::NORTH: next = Location(location.getX(), location.getY() - step); break;
        case Direction::SOUTH: next = Location(location.getX(), location.getY() + step); break;
        case Direction::EAST: next = Location(location.getX() + step, location.getY()); break;
        case Direction::WEST: next = Location(location.getX() - step, location.getY()); break;
    }
    if (world.isOutOfBounds(next)) {
        rejectionReason = "out of bounds";
        return false;
    }
    char terrain = world.getTerrain(next.getX(), next.getY());
    if (terrain == '#') {
        rejectionReason = "blocked by rock";
        return false;
    }
    int cost = 2;
    if (backward) cost = 3;
    if (terrain == '~') cost += 2;
    if (!canPay(cost)) return false;
    location = next;
    batteryLevel -= cost;
    return true;
}

// ---- Turn in place ----
bool Rover::turn(bool left) {
    if (!canPay(1)) return false;
    if (left) {
        switch (heading) {
            case Direction::NORTH: heading = Direction::WEST; break;
            case Direction::WEST: heading = Direction::SOUTH; break;
            case Direction::SOUTH: heading = Direction::EAST; break;
            case Direction::EAST: heading = Direction::NORTH; break;
        }
    } else {
        switch (heading) {
            case Direction::NORTH: heading = Direction::EAST; break;
            case Direction::EAST: heading = Direction::SOUTH; break;
            case Direction::SOUTH: heading = Direction::WEST; break;
            case Direction::WEST: heading = Direction::NORTH; break;
        }
    }
    batteryLevel--;
    return true;
}

// ---- Discover and collect samples ----
bool Rover::scan() {
    if (!canPay(3)) return false;
    lastScanCount = world.revealNearby(location);
    batteryLevel -= 3;
    return true;
}
bool Rover::drill() {
    if (!world.isKnownSample(location)) {
        rejectionReason = "no known sample site here";
        return false;
    }
    if (!canPay(5)) return false;
    world.collectSample(location);
    samples++;
    batteryLevel -= 5;
    return true;
}

// ---- Recharge at a station ----
bool Rover::charge() {
    if (!world.isLocationAChargingStation(location)) {
        rejectionReason = "not at charging station";
        return false;
    }
    batteryLevel = 40;
    return true;
}

// ---- Run one command ----
bool Rover::executeCommand(char command) {
    bool successful = false;
    rejectionReason = "";
    lastScanCount = 0;
    switch (command) {
        case 'F': successful = move(false); break;
        case 'B': successful = move(true); break;
        case 'L': successful = turn(true); break;
        case 'R': successful = turn(false); break;
        case 'S': successful = scan(); break;
        case 'D': successful = drill(); break;
        case 'C': successful = charge(); break;
        case 'P':
            successful = canPay(1);
            if (successful) batteryLevel--;
            // main.cpp takes the photograph; the rover never uses graphics.
            break;
        default: rejectionReason = "unknown command";
    }
    lastResult = string("Command ") + command;
    if (successful) lastResult += " succeeded";
    else lastResult += " rejected: " + rejectionReason;
    return successful;
}
// Keep the required console wording in one place, so tests can check it too.
string Rover::getCommandStatus(char command, bool successful) {
    if (!successful) {
        return string("Command ") + command + " rejected: " + rejectionReason;
    }
    string status = string("Command ") + command + " succeeded. Current " + toString();
    if (command == 'S') {
        status += "\nScan found " + to_string(lastScanCount) + " new sample site";
        if (lastScanCount != 1) status += "s";
        status += ".";
    }
    if (command == 'D') status += "\nSample collected. Total samples: " + to_string(samples);
    return status;
}
void Rover::printCommandStatus(char command, bool successful) {
    cout << getCommandStatus(command, successful) << endl;
}

// ---- Read the rover's condition ----
Location Rover::getLocation() { return location; }
Direction Rover::getHeading() { return heading; }
int Rover::getBattery() { return batteryLevel; }
int Rover::getSamples() { return samples; }
int Rover::getLastScanCount() { return lastScanCount; }
string Rover::getLastResult() { return lastResult; }
bool Rover::isReset() { return batteryLevel == 40 && world.isLocationAChargingStation(location); }
string Rover::getHeadingName() {
    switch (heading) {
        case Direction::NORTH: return "North";
        case Direction::EAST: return "East";
        case Direction::SOUTH: return "South";
        case Direction::WEST: return "West";
    }
    return "Unknown";
}
string Rover::toString() {
    return "coordinates: (" + to_string(location.getX()) + "," + to_string(location.getY())
        + ") Heading: " + getHeadingName() + " Battery: " + to_string(batteryLevel);
}
