// Mission.h - one route, its goal, and the results along the way.
// Based on William Wright's recitation Mission class.
#ifndef CSCI1300_CS1_MISSION_H
#define CSCI1300_CS1_MISSION_H
#include <string>
#include <vector>
#include "Location.h"
#include "Rover.h"
using namespace std;

class Mission {
public:
    Mission(string theCommands);
    // ---- Choose a destination ----
    bool canChooseGoal();
    void setGoal(Location newGoal, Location start);
    bool hasGoal();
    Location getGoal();
    // ---- Follow the command list ----
    bool runNext(Rover& rover, bool print = true);
    bool isFinished();
    char getNextCommand();
    int getNextIndex();
    string getCommands();
    // ---- Read and save the results ----
    int getSuccessfulCount();
    int getRejectedCount();
    int getSamplesCollected();
    bool wasGoalReached();
    const vector<Location>& getPath();
    string getStatus(Rover& rover);
    string summaryLine(int number, Rover& rover);
private:
    Location goal;
    string commands;
    bool goalSet = false;
    bool goalReached = false;
    int nextCommand = 0;
    int successfulCommands = 0;
    int rejectedCommands = 0;
    int samplesCollected = 0;
    vector<Location> path;
};

// Read the saved command lists without opening a graphics window.
bool loadMissions(string filename, vector<Mission>& missions, string& error);
string getSummaryHeader();
#endif
