// Lone Dune: The Last Signal
// Read this file from top to bottom to follow one expedition.
#include <iostream>
#include <fstream>
#include <vector>
#include "graphics/Window.h"
#include "World.h"
#include "Rover.h"
#include "Mission.h"
#include "WorldView.h"
using namespace std;

// Check both files before opening the window, and reload them on restart.
bool loadExpedition(World& world, vector<Mission>& missions) {
    string error;
    if (!world.load("world.txt", error) || !loadMissions("missions.txt", missions, error)) {
        cerr << error << endl;
        return false;
    }
    cout << "Loaded " << missions.size() << " missions." << endl;
    return true;
}

// Start with a full battery, mission 1, and a new report.
// Return 2 to restart, 0 to close, or 1 if saving fails.
int runExpedition(Window& window, World& world, vector<Mission>& missions) {
    ofstream summary("mission_summary.csv");
    summary << getSummaryHeader() << "\n";
    summary.flush();
    if (!summary.good()) {
        cerr << "Cannot create mission_summary.csv" << endl;
        return 1;
    }

    // ---- Start the rover and set up the drawing ----
    Rover rover(world, world.getFirstChargingStation(), Direction::EAST, 40);
    WorldView view(world, rover);
    int currentMission = 0;
    bool automatic = false;
    int pendingSteps = 0;
    double timeSinceCommand = 0;
    bool savedWaitingPicture = false;
    bool savedMissionPicture = false;

    while (window.isOpen()) {
        double seconds = window.getDeltaTime();
        view.updateAnimation(seconds);
        if (window.wasKeyPressed(Key::Escape)) break;
        if (window.wasKeyPressed('R')) {
            // Clear this key press before starting the new expedition.
            window.update();
            return 2;
        }

        // ---- Start the next mission only after this one finishes ----
        if (window.wasKeyPressed('N') && missions[currentMission].isFinished() && !view.isMoving()
            && currentMission + 1 < static_cast<int>(missions.size())) {
            currentMission++;
            automatic = false;
            pendingSteps = 0;
            timeSinceCommand = 0;
            view.showNextCommand(missions[currentMission]);
        }
        Mission& mission = missions[currentMission];

        // ---- Choose a goal with the mouse ----
        if (window.wasMouseClicked() && mission.canChooseGoal()) {
            Location goal;
            if (view.mouseToCell(window.getMousePosition(), goal)
                && world.getTerrain(goal.getX(), goal.getY()) != '#') {
                mission.setGoal(goal, rover.getLocation());
            }
        }

        // ---- Read a long route without moving the rover ----
        if (window.wasKeyPressed(Key::Left)) view.changeRoutePage(-1, mission);
        if (window.wasKeyPressed(Key::Right)) view.changeRoutePage(1, mission);

        // ---- Run one command, or run automatically every 0.3 seconds ----
        bool runCommand = false;
        if (mission.hasGoal() && !mission.isFinished()) {
            if (window.wasKeyPressed(Key::Enter) && !automatic) {
                automatic = true;
                timeSinceCommand = 0;
            }
            // Remember each Space press, even if the picture is still sliding.
            // Holding the key adds only one step because we use wasKeyPressed.
            if (window.wasKeyPressed(Key::Space)) pendingSteps++;
            if (automatic) timeSinceCommand += seconds;

            // Finish one slide before starting the next command or photograph.
            if (!view.isMoving() && (pendingSteps > 0 || (automatic && timeSinceCommand >= 0.3))) {
                runCommand = true;
                if (pendingSteps > 0) pendingSteps--;
                timeSinceCommand = 0;
            }
        }

        string photoFilename;
        if (runCommand) {
            char command = mission.getNextCommand();
            Location previousCell = rover.getLocation();
            bool successful = mission.runNext(rover);
            view.startMove(previousCell);
            view.showNextCommand(mission);
            if (successful && command == 'P') {
                photoFilename = "photo_" + to_string(currentMission + 1) + "_"
                    + to_string(mission.getNextIndex()) + ".png";
            }

            // ---- Save exactly one report row for a completed mission ----
            if (mission.isFinished()) {
                automatic = false;
                pendingSteps = 0;
                cout << mission.getStatus(rover) << endl;
                summary << mission.summaryLine(currentMission + 1, rover) << "\n";
                summary.flush();
                if (!summary.good()) {
                    cerr << "Cannot save the completed mission summary." << endl;
                    return 1;
                }
            }
        }

        // ---- Draw the map and the instructions on the right ----
        view.draw(window, mission, currentMission + 1, static_cast<int>(missions.size()), automatic);

        // ---- Save photos and screenshots of the mission ----
        if (!photoFilename.empty() && !window.saveScreenshot(photoFilename)) {
            cerr << "Cannot save photograph: " << photoFilename << endl;
            return 1;
        }
        if (!savedWaitingPicture && !mission.hasGoal()) {
            savedWaitingPicture = window.saveScreenshot("waiting_for_goal.png");
        }
        if (!savedMissionPicture && mission.getNextIndex() > 0 && !mission.isFinished() && !view.isMoving()) {
            savedMissionPicture = window.saveScreenshot("during_mission.png");
        }
        window.update();
    }
    return 0;
}

int main() {
    World world;
    vector<Mission> missions;
    if (!loadExpedition(world, missions)) return 1;

    Window window(1080, 800, "Lone Dune: The Last Signal");
    if (!window.isOpen()) return 1;
    int result;
    do {
        result = runExpedition(window, world, missions);
        if (result == 2 && window.isOpen() && !loadExpedition(world, missions)) result = 1;
    } while (result == 2 && window.isOpen());
    window.close();
    return result == 1 ? 1 : 0;
}
