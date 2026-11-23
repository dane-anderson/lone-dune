// Mission.cpp - keep mission progress separate from the window.
#include "Mission.h"
#include <fstream>
using namespace std;

// ---- Read the mission file ----
bool loadMissions(string filename, vector<Mission>& missions, string& error) {
    missions.clear();
    error = "";
    ifstream file(filename);
    if (!file.is_open()) {
        error = "Cannot open mission file: " + filename;
        return false;
    }
    string line;
    while (getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;
        // Keep every command, including spaces and unknown letters.
        missions.push_back(Mission(line));
    }
    if (missions.empty()) {
        error = "Mission file contains no missions: " + filename;
        return false;
    }
    return true;
}

string getSummaryHeader() {
    return "mission,commands,goal_x,goal_y,successful,rejected,goal_reached,reset,samples,battery";
}

Mission::Mission(string theCommands) { commands = theCommands; }

// ---- Choose a destination ----
bool Mission::canChooseGoal() { return nextCommand == 0; }
void Mission::setGoal(Location newGoal, Location start) {
    if (!canChooseGoal()) return;
    goal = newGoal;
    goalSet = true;
    goalReached = start.equals(goal);
    path.clear();
    path.push_back(start);
}
bool Mission::hasGoal() { return goalSet; }
Location Mission::getGoal() { return goal; }

// ---- Follow the command list ----
bool Mission::runNext(Rover& rover, bool print) {
    if (!goalSet || isFinished()) return false;
    Location before = rover.getLocation();
    int samplesBefore = rover.getSamples();
    char command = commands[nextCommand];
    bool successful = rover.executeCommand(command);
    if (print) rover.printCommandStatus(command, successful);
    if (successful) successfulCommands++;
    else rejectedCommands++;
    Location after = rover.getLocation();
    if (!after.equals(before)) path.push_back(after);
    if (after.equals(goal)) goalReached = true;
    samplesCollected += rover.getSamples() - samplesBefore;
    nextCommand++;
    return successful;
}
bool Mission::isFinished() { return nextCommand >= static_cast<int>(commands.size()); }
char Mission::getNextCommand() {
    if (isFinished()) return '\0';
    return commands[nextCommand];
}
int Mission::getNextIndex() { return nextCommand; }
string Mission::getCommands() { return commands; }

// ---- Read and save the results ----
int Mission::getSuccessfulCount() { return successfulCommands; }
int Mission::getRejectedCount() { return rejectedCommands; }
int Mission::getSamplesCollected() { return samplesCollected; }
bool Mission::wasGoalReached() { return goalReached; }
const vector<Location>& Mission::getPath() { return path; }
string Mission::getStatus(Rover& rover) {
    string status = "Mission Complete!\n";
    Location end = rover.getLocation();
    status += "Rover ended at coordinates (" + to_string(end.getX()) + "," + to_string(end.getY())
        + ") with heading " + rover.getHeadingName() + " and battery " + to_string(rover.getBattery()) + ".\n";
    status += "Successful commands: " + to_string(successfulCommands)
        + "  Rejected commands: " + to_string(rejectedCommands)
        + "  Samples collected: " + to_string(samplesCollected) + "\n";
    if (goalReached && rover.isReset()) status += "Goal reached and rover reset.";
    else if (goalReached) status += "Goal reached but rover not reset.";
    else if (rover.isReset()) status += "Goal not reached but rover reset.";
    else status += "Goal not reached and rover not reset.";
    return status;
}
string Mission::summaryLine(int number, Rover& rover) {
    // A command string may contain a comma or quote. Quote it only when needed.
    string csvCommands = commands;
    if (commands.find_first_of(",\"") != string::npos) {
        csvCommands = "\"";
        for (char character : commands) {
            if (character == '"') csvCommands += '"';
            csvCommands += character;
        }
        csvCommands += '"';
    }
    string reached = "no", reset = "no";
    if (goalReached) reached = "yes";
    if (rover.isReset()) reset = "yes";
    return to_string(number) + "," + csvCommands + ","
        + to_string(goal.getX()) + "," + to_string(goal.getY()) + ","
        + to_string(successfulCommands) + "," + to_string(rejectedCommands) + ","
        + reached + "," + reset + "," + to_string(samplesCollected) + ","
        + to_string(rover.getBattery());
}
