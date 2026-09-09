#include <gtest/gtest.h>
#include "SimulationEngine.h"

using namespace RfSimulation;
using namespace Eigen;

class SimulationEngineTest : public ::testing::Test {
protected:
    WallVector walls;

    void SetUp() override {
        // Material is largely irrelevant for purely geometric tests, so we default to Plasterboard
        ITUR_P2040::MaterialClass mat = ITUR_P2040::MaterialClass::Plasterboard;

        // Outer perimeter of the 10x10 house
        walls.push_back({Vector3d(0, 0, 0),  Vector3d(10, 0, 0),  3.0, 0.15, mat}); // Bottom
        walls.push_back({Vector3d(0, 10, 0), Vector3d(10, 10, 0), 3.0, 0.15, mat}); // Top
        walls.push_back({Vector3d(0, 0, 0),  Vector3d(0, 10, 0),  3.0, 0.15, mat}); // Left
        walls.push_back({Vector3d(10, 0, 0), Vector3d(10, 10, 0), 3.0, 0.15, mat}); // Right

        // Middle dividing wall at x = 5
        // Doorway is a 2-meter gap from y = 4 to y = 6
        walls.push_back({Vector3d(5, 0, 0), Vector3d(5, 4, 0),  3.0, 0.15, mat}); // Solid bottom half
        walls.push_back({Vector3d(5, 6, 0), Vector3d(5, 10, 0), 3.0, 0.15, mat}); // Solid top half
    }
};

TEST_F(SimulationEngineTest, UnobstructedPath_LoS) {
    Vector3d txPos(2.5, 5.0, 1.5); // Center of Room A (y=5 aligns with doorway)
    Vector3d rxPos(7.5, 5.0, 1.5); // Center of Room B

    // Verify that there are no wall intersections between Tx & Rx
    const auto numIntersects = SimulationEngine::getIntersectedWalls(txPos, rxPos, walls).size();
    EXPECT_EQ(numIntersects, 0);

    // Generate a shallow tree (maximum of 1 reflection is enough since we know this is a LoS path)
    auto shallowImageTree = SimulationEngine::generateImageTree(txPos, walls, 1);
    auto validPaths = SimulationEngine::computeValidPaths(txPos, rxPos, walls, shallowImageTree);

    bool foundLoS = false;
    for (const auto& path : validPaths) {
        if (path.hitWalls.empty()) {
            foundLoS = true;
            // The path should consist only of the Tx and Rx nodes
            EXPECT_EQ(path.nodes.size(), 2);
            break;
        }
    }
    
    // Should have found at least one path that does not hit any walls
    EXPECT_TRUE(foundLoS);
}

TEST_F(SimulationEngineTest, BlockedByWall_NLoS) {
    Vector3d txPos(2.5, 2.0, 1.5); // Room A, hidden behind a wall
    Vector3d rxPos(7.5, 2.0, 1.5); // Room B, directly opposite also behind a wall

    // There should be 1 wall in the way between the Tx & Rx
    const auto numIntersects = SimulationEngine::getIntersectedWalls(txPos, rxPos, walls).size();
    EXPECT_EQ(numIntersects, 1);

    auto imageTree = SimulationEngine::generateImageTree(txPos, walls, 1);
    auto validPaths = SimulationEngine::computeValidPaths(txPos, rxPos, walls, imageTree);

    for (const auto& path : validPaths) {
        // Because the direct path is obstructed, that should not be counted as a valid path
        EXPECT_FALSE(path.hitWalls.empty());
    }
}

TEST_F(SimulationEngineTest, SingleBounceThroughDoorway) {
    Vector3d txPos(2.5, 5.0, 1.5); // Center of Room A (y=5 aligns with doorway)
    Vector3d rxPos(7.5, 5.0, 1.5); // Center of Room B

    auto imageTree = SimulationEngine::generateImageTree(txPos, walls, 1);
    auto validPaths = SimulationEngine::computeValidPaths(txPos, rxPos, walls, imageTree);

    // We expect a valid path to exist such that the ray leaves the Tx towards the outer wall 
    // and then bounces directly back pass the Tx and towards the Rx
    // Step 1. |<-- Tx      Rx
    // Step 2. |--> Tx      Rx
    // Step 3. |    Tx  --> Rx 
    
    bool foundLeftWallBounce = false;
    
    for (const auto& path : validPaths) {
        if (path.hitWalls.size() == 1) {
            // Check if this path hits the left outer wall (x=0)
            if (path.hitWalls[0].start.x() == 0.0 && path.hitWalls[0].end.x() == 0.0) {
                foundLeftWallBounce = true;
                
                // Reflection point is at (x=0, y=5)
                EXPECT_DOUBLE_EQ(path.nodes[1].x(), 0.0);
                EXPECT_DOUBLE_EQ(path.nodes[1].y(), 5.0);
                
                // After this reflection, the final node is the Rx: Tx -> Wall -> Rx
                EXPECT_EQ(path.nodes.size(), 3);
            }
        }
    }
    
    EXPECT_TRUE(foundLeftWallBounce);
}