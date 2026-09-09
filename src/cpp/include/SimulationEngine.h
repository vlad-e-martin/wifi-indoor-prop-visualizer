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
        /// @brief Initializes a ray originating from a vertically oriented dipole transmitter
        static RayState initializeTxRay(const Eigen::Vector3d& txPos, const Eigen::Vector3d& launchDir);
        
        /// @brief Processes the EM physics of a ray bouncing off a boundary, updating its E-field and direction
        static void processReflection(
            RayState& ray, 
            const Eigen::Vector3d& intersectionPt, 
            const Eigen::Vector3d& surfaceNormal, 
            double wallThickness_m, 
            double freq_GHz, 
            ITUR_P2040::MaterialClass materialClass);

        /// @brief Returns a list of all walls physically intersected by a direct line-of-sight path.
        static WallVector getIntersectedWalls(
            const Eigen::Vector3d& startPt, 
            const Eigen::Vector3d& endPt, 
            const WallVector& walls);

        /// @brief Calculates the 2D intersection point between a ray segment and a wall segment
        /// @return The 3D intersection point (with interpolated Z) if an intersection occurs, otherwise std::nullopt
        static std::optional<Eigen::Vector3d> getIntersection(
            const Eigen::Vector3d& p1, 
            const Eigen::Vector3d& p2, 
            const Wall& wall);

        /// @brief Recursively generates the virtual transmitter image tree (only runs once per simulation)
        /// @param txPos The transmitter location
        /// @param walls The list of all walls in the environment
        /// @param maxBounces The maximum number of bounces before we stop considering this path
        /// @return A flat array representing the hierarchical Image Tree
        static ImageNodeVector generateImageTree(
            const Eigen::Vector3d& txPos, 
            const WallVector& walls, 
            int maxBounces);

        /// @brief Traces paths backward from Rx to Tx using the pre-computed Image Tree
        /// @param txPos The transmitter location
        /// @param rxPos The target grid point receiver location
        /// @param walls The list of all walls in the environment
        /// @param imageTree The pre-computed virtual image tree
        /// @return A list of geometrically valid paths
        static RayPathVector computeValidPaths(
            const Eigen::Vector3d& txPos, 
            const Eigen::Vector3d& rxPos, 
            const WallVector& walls, 
            const ImageNodeVector& imageTree);
    private:
        /// @brief Mirror a point across a 2.5D wall plane
        static Eigen::Vector3d mirrorPoint(const Eigen::Vector3d& pt, const Wall& wall);
        
        /// @brief Check if a valid path segment hits any obstructing walls
        static bool isObstructed(
            const Eigen::Vector3d& p1, 
            const Eigen::Vector3d& p2, 
            const WallVector& walls, 
            int ignoreWallIdx1, 
            int ignoreWallIdx2);
    };
}

#endif // SIMULATION_ENGINE_H