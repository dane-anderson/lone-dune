// tests.cpp - check the rover rules without opening a window.
// Run from the project folder: bash tools/check.sh
#include <iostream>
#include <fstream>
#include <string>
#include "../rover/Location.h"
#include "../rover/Direction.h"
#include "../rover/World.h"
#include "../rover/Rover.h"
#include "../rover/Mission.h"
using namespace std;

int testsPassed = 0;
int testsFailed = 0;

void check(bool condition, string description) {
    if (condition) {
        cout << "PASS: " << description << endl;
        testsPassed++;
    } else {
        cout << "FAIL: " << description << endl;
        testsFailed++;
    }
}

World loadWorld(string filename = "valid_world.txt") {
    World world;
    string error;
    check(world.load("data/" + filename, error), "load " + filename);
    return world;
}

bool at(Rover& rover, int x, int y) {
    Location location = rover.getLocation();
    return location.getX() == x && location.getY() == y;
}

// ---- Check coordinates ----
void testCoordinates() {
    Location a(2, 3), b(2, 3), c(3, 2);
    check(a.equals(b) && !a.equals(c), "compare coordinates");
    Location inside(19, 19), outside(20, 19), negative(-1, 0);
    check(inside.isWithinBounds(20, 20) && !outside.isWithinBounds(20, 20)
          && !negative.isWithinBounds(20, 20), "check all coordinate limits");

}

// ---- Reject invalid map files ----
void testMapFiles() {
    string invalid[] = {"invalid_size.txt", "invalid_zero.txt", "invalid_number.txt",
        "invalid_extra_size.txt", "invalid_huge_number.txt", "invalid_missing_row.txt", "invalid_extra_row.txt",
        "invalid_short_row.txt", "invalid_long_row.txt", "invalid_terrain.txt",
        "invalid_no_charger.txt", "file_that_does_not_exist.txt"};
    for (string filename : invalid) {
        World world;
        string error;
        check(!world.load("data/" + filename, error) && !error.empty() && world.getWidth() == 0,
              "reject " + filename);
    }
    World world = loadWorld("other_start.txt");
    Location first = world.getFirstChargingStation();
    check(first.getX() == 1 && first.getY() == 1, "start at the first charger in reading order");
    world = loadWorld("maximum_world.txt");
    check(world.getWidth() == 20 && world.getHeight() == 20, "accept a 20 x 20 map");
    world = loadWorld("one_cell.txt");
    check(world.getWidth() == 1 && world.getHeight() == 1, "accept a 1 x 1 map");
    world = loadWorld("windows_world.txt");
    check(world.getWidth() == 5, "accept Windows line endings");
}

// ---- Check movement costs and rejection order ----
void testMovement() {
    World world = loadWorld();
    Rover rover(world, Location(0, 0), Direction::EAST, 40);
    check(rover.executeCommand('F') && at(rover, 1, 0) && rover.getBattery() == 36,
          "forward into sand costs four");
    check(!rover.executeCommand('F') && at(rover, 1, 0) && rover.getBattery() == 36
          && rover.getLastResult().find("blocked by rock") != string::npos, "rock blocks movement without a cost");
    Rover backward(world, Location(0, 0), Direction::WEST, 40);
    check(backward.executeCommand('B') && at(backward, 1, 0) && backward.getBattery() == 35
          && backward.getHeading() == Direction::WEST, "backward into sand costs five and keeps the heading");
    Rover low(world, Location(0, 0), Direction::EAST, 3);
    check(!low.executeCommand('F') && at(low, 0, 0) && low.getBattery() == 3,
          "insufficient sand energy leaves position and battery unchanged");
    Rover exact(world, Location(0, 0), Direction::EAST, 4);
    check(exact.executeCommand('F') && exact.getBattery() == 0, "exact battery payment is allowed");
    Rover edge(world, Location(0, 0), Direction::NORTH, 0);
    check(!edge.executeCommand('F') && edge.getLastResult().find("out of bounds") != string::npos,
          "check map bounds before battery");
    Rover rock(world, Location(1, 0), Direction::EAST, 0);
    check(!rock.executeCommand('F') && rock.getLastResult().find("blocked by rock") != string::npos,
          "check rocks before battery");
    Rover south(world, Location(2, 2), Direction::NORTH, 40);
    check(south.executeCommand('F') && at(south, 2, 1), "north subtracts one from y");
    check(south.executeCommand('B') && at(south, 2, 2) && south.getBattery() == 35,
          "backward on open ground costs three");
    for (int i = 0; i < 4; i++) south.executeCommand('R');
    check(south.getHeading() == Direction::NORTH && south.getBattery() == 31, "four right turns return north");
    for (int i = 0; i < 4; i++) south.executeCommand('L');
    check(south.getHeading() == Direction::NORTH && south.getBattery() == 27, "four left turns return north");
}

// ---- Check scanning and collecting ----
void testSamples() {
    World world = loadWorld();
    Rover rover(world, Location(0, 0), Direction::EAST, 40);
    check(rover.executeCommand('S') && rover.getLastScanCount() == 1 && world.isRevealed(1, 1)
          && !world.isRevealed(3, 2), "edge scan includes diagonals and stays within its 3 x 3 square");
    check(rover.executeCommand('S') && rover.getLastScanCount() == 0 && rover.getBattery() == 34,
          "repeat scan succeeds, costs three, and only counts new sites");
    Rover collector(world, Location(1, 1), Direction::EAST, 40);
    check(collector.executeCommand('D') && collector.getSamples() == 1 && collector.getBattery() == 35
          && world.getTerrain(1, 1) == '.' && !world.isRevealed(1, 1), "drill removes a revealed sample from the shared map");
    check(!collector.executeCommand('D') && collector.getSamples() == 1 && collector.getBattery() == 35,
          "a sample cannot be collected twice");
    World hiddenWorld = loadWorld();
    Rover hidden(hiddenWorld, Location(1, 1), Direction::EAST, 0);
    check(!hidden.executeCommand('D') && hidden.getLastResult().find("no known sample site here") != string::npos
          && hiddenWorld.getTerrain(1, 1) == '*', "hidden drill rejected before the battery check");
    Rover weakScan(hiddenWorld, Location(0, 0), Direction::EAST, 2);
    check(!weakScan.executeCommand('S') && !hiddenWorld.isRevealed(1, 1) && weakScan.getBattery() == 2,
          "failed scan does not reveal a sample");
    hiddenWorld.revealNearby(Location(1, 1));
    Rover weakDrill(hiddenWorld, Location(1, 1), Direction::EAST, 4);
    check(!weakDrill.executeCommand('D') && weakDrill.getSamples() == 0
          && hiddenWorld.isKnownSample(Location(1, 1)) && weakDrill.getBattery() == 4,
          "failed drill leaves the revealed sample and battery unchanged");
}

// ---- Check charging, photography, and unknown commands ----
void testOtherCommands() {
    World world = loadWorld();
    Rover rover(world, Location(4, 1), Direction::WEST, 0);
    check(rover.executeCommand('C') && rover.getBattery() == 40 && rover.isReset(),
          "charge from zero at a station other than the starting station");
    check(rover.executeCommand('P') && rover.getBattery() == 39, "photograph costs one");
    for (char command : string("Xf TV")) {
        check(!rover.executeCommand(command) && rover.getBattery() == 39 && at(rover, 4, 1),
              string("unknown command preserves state: '") + command + "'");
    }
    Rover away(world, Location(3, 1), Direction::EAST, 0);
    check(!away.executeCommand('C') && away.getBattery() == 0, "cannot charge away from a station");
    check(!away.executeCommand('P') && away.getBattery() == 0, "cannot photograph with an empty battery");
}

// ---- Follow a mission from beginning to end ----
void testMissions() {
    World world = loadWorld();
    Rover rover(world, Location(0, 0), Direction::SOUTH, 40);
    Mission mission("FXB");
    check(!mission.runNext(rover, false) && mission.getNextIndex() == 0, "a mission waits for a goal");
    mission.setGoal(Location(0, 0), rover.getLocation());
    check(mission.wasGoalReached(), "a goal counts at the mission's starting position");
    mission.runNext(rover, false);
    mission.setGoal(Location(4, 3), rover.getLocation());
    check(mission.getGoal().getX() == 0, "the goal locks after the first command");
    mission.runNext(rover, false);
    mission.runNext(rover, false);
    check(mission.isFinished() && mission.getSuccessfulCount() == 2 && mission.getRejectedCount() == 1
          && at(rover, 0, 0) && mission.getPath().size() == 3, "unknown command in the middle does not stop a mission");
    check(!mission.runNext(rover, false) && mission.getSuccessfulCount() == 2,
          "finished missions cannot run an extra command");
    Mission second("F");
    second.setGoal(Location(0, 1), rover.getLocation());
    second.runNext(rover, false);
    check(rover.getBattery() == 33 && second.getSuccessfulCount() == 1 && second.wasGoalReached(),
          "next mission keeps rover state and starts fresh counts");
}

// ---- Match the assignment's full three-mission example ----
void testAssignmentExample() {
    World world = loadWorld("example_world.txt");
    Rover rover(world, world.getFirstChargingStation(), Direction::EAST, 40);
    string routes[] = {"FSRFDLFFFP", "LFLFFFFRFC", "RFFRFFBXS"};
    Location goals[] = {Location(3, 1), Location(0, 1), Location(2, 2)};
    string expected[] = {
        "1,FSRFDLFFFP,3,1,10,0,yes,no,1,19",
        "2,LFLFFFFRFC,0,1,9,1,yes,yes,0,40",
        "3,RFFRFFBXS,2,2,8,1,yes,no,0,22"
    };
    string console;
    for (int i = 0; i < 3; i++) {
        Mission mission(routes[i]);
        mission.setGoal(goals[i], rover.getLocation());
        while (!mission.isFinished()) {
            char command = mission.getNextCommand();
            bool successful = mission.runNext(rover, false);
            console += rover.getCommandStatus(command, successful) + "\n";
        }
        console += mission.getStatus(rover) + "\n";
        check(mission.summaryLine(i + 1, rover) == expected[i],
              "exact assignment summary for mission " + to_string(i + 1));
    }
    // Compare with the printed example, not with strings made by the rover.
    ifstream reference("data/assignment_console.txt");
    string line, expectedConsole;
    check(reference.is_open(), "open the assignment's console reference");
    while (getline(reference, line)) expectedConsole += line + "\n";
    check(console == expectedConsole, "all three missions match the assignment's console example exactly");
}

// ---- Read missions and check the report format ----
void testMissionFiles() {
    vector<Mission> missions;
    string error;
    check(!loadMissions("data/missing_missions.txt", missions, error), "missing mission file rejected");
    check(!loadMissions("data/empty_missions.txt", missions, error), "comment-only mission file rejected");
    check(loadMissions("data/test_missions.txt", missions, error) && missions.size() == 2,
          "mission loader ignores empty and comment lines");
    check(loadMissions("data/literal_missions.txt", missions, error)
          && missions[0].getCommands() == "F x", "spaces and lowercase letters survive loading");
    check(getSummaryHeader() == "mission,commands,goal_x,goal_y,successful,rejected,goal_reached,reset,samples,battery",
          "the report header matches the assignment");
    World world = loadWorld();
    Rover rover(world, world.getFirstChargingStation(), Direction::EAST, 40);
    Mission quoted("F,\"x");
    quoted.setGoal(Location(0, 0), rover.getLocation());
    check(quoted.summaryLine(1, rover).find("\"F,\"\"x\"") != string::npos,
          "CSV quotes command strings containing commas and quotes");
}

// ---- Follow the three routes on our own map ----
void testLoneDuneRoutes() {
    World world;
    string error;
    check(world.load("../rover/world.txt", error), "load Lone Dune's own map");
    Rover rover(world, world.getFirstChargingStation(), Direction::EAST, 40);
    vector<Mission> missions;
    check(loadMissions("../rover/missions.txt", missions, error) && missions.size() == 3,
          "load the three Lone Dune missions");
    if (missions.size() != 3) return;
    Location goals[] = {Location(4, 1), Location(3, 4), Location(9, 7)};
    string expected[] = {
        "1,FSRFDLFFFP,4,1,10,0,yes,no,1,17",
        "2,FRFFFRFFC,3,4,9,0,yes,yes,0,40",
        "3,BBLFLFSFDRFFLFFPCX,9,7,17,1,yes,yes,1,40"
    };
    ofstream report("results/route_check.csv");
    report << getSummaryHeader() << "\n";
    for (int i = 0; i < 3; i++) {
        missions[i].setGoal(goals[i], rover.getLocation());
        while (!missions[i].isFinished()) missions[i].runNext(rover, false);
        string row = missions[i].summaryLine(i + 1, rover);
        report << row << "\n";
        check(row == expected[i], "own-map mission " + to_string(i + 1) + " has the expected report row");
    }
    report.flush();
    check(report.good(), "save the route-check report");
    check(at(rover, 9, 7) && rover.getSamples() == 2 && rover.getBattery() == 40,
          "the same rover carries its state through all three missions");
}

int main() {
    testCoordinates();
    testMapFiles();
    testMovement();
    testSamples();
    testOtherCommands();
    testMissions();
    testAssignmentExample();
    testMissionFiles();
    testLoneDuneRoutes();
    cout << "\n" << testsPassed << " passed, " << testsFailed << " failed.\n";
    return testsFailed == 0 ? 0 : 1;
}
