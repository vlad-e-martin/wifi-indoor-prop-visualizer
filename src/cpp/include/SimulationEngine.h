#ifndef SIMULATION_ENGINE_H
#define SIMULATION_ENGINE_H

#include "Types.h"
#include "ITUR_P2040.h"

#include <Eigen/Dense>

#include <vector>
#include <optional>

namespace RfSimulation {
    class SimulationEngine {
    public:
        /// @brief Initializes a ray originating from a vertically oriented dipole transmitter.
        static RayState initializeTxRay(const Eigen::Vector3d& txPos, const Eigen::Vector3d& launchDir);
        
        /// @brief Processes the EM physics of a ray bouncing off a boundary, updating its E-field and direction.
        static void processReflection(
            RayState& ray, 
            const Eigen::Vector3d& intersectionPt, 
            const Eigen::Vector3d& surfaceNormal, 
            double wallThickness_m, 
            double freq_GHz, 
            ITUR_P2040::MaterialClass materialClass);

        /// @brief Counts the number of walls physically intersected by a direct line-of-sight path.
        static int countWallIntersections(
            const Eigen::Vector3d& startPt, 
            const Eigen::Vector3d& endPt, 
            const std::vector<Wall>& walls);

        /// @brief Calculates the 2D intersection point between a ray segment and a wall segment.
        /// @return The 3D intersection point (with interpolated Z) if an intersection occurs, otherwise std::nullopt.
        static std::optional<Eigen::Vector3d> getIntersection(
            const Eigen::Vector3d& p1, 
            const Eigen::Vector3d& p2, 
            const Wall& wall);
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