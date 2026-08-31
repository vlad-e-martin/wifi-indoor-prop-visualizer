#ifndef SIMULATION_ENGINE_H
#define SIMULATION_ENGINE_H

#include "Types.h"
#include "ITUR_P2040.h"

#include <Eigen/Dense>

namespace RfSimulation {
    class SimulationEngine {
    public:
        // Core pathfinding execution for a single Rx coordinate
        static std::vector<RayPath> computeValidPaths(
            const Eigen::Vector3d& txPos, 
            const Eigen::Vector3d& rxPos, 
            const std::vector<Wall>& walls, 
            int maxBounces = 10);

    private:
        // Recursive geometric mirroring to build the virtual Tx tree
        static void buildImageTree();

        // Fallback helper to count direct wall penetrations
        static int countWallIntersections(
            const Eigen::Vector3d& p1, 
            const Eigen::Vector3d& p2, 
            const std::vector<Wall>& walls);

        // Validates a back-traced line segment against physical obstructions
        static bool isPathObstructed(
            const Eigen::Vector3d& p1, const Eigen::Vector3d& p2, 
            const std::vector<Wall>& walls, const Wall* ignoreWall);
    };
}

namespace RfSimulation {
    class SimulationEngine {
    public:
        static RayState initializeTxRay(const Eigen::Vector3d& txPos, const Eigen::Vector3d& launchDir);
        
        static void processReflection(
            RayState& ray, 
            const Eigen::Vector3d& intersectionPt, 
            const Eigen::Vector3d& surfaceNormal, 
            double wallThickness_m, 
            double freq_GHz, 
            ITUR_P2040::MaterialClass materialClass);
    };
}

#endif // SIMULATION_ENGINE_H