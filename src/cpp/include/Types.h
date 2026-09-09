#ifndef TYPES_H
#define TYPES_H

#include "ITUR_P2040.h"

#include <Eigen/Dense>
#include <Eigen/StdVector>

#include <vector>
#include <complex>

namespace RfSimulation {
    inline constexpr double kSpeedOfLight_mPerSec = 299792458.0;

    // Represents a wall within the indoor floor plan
    struct Wall {
        Eigen::Vector3d start;
        Eigen::Vector3d end;
        double height_m;
        double thickness_m;
        ITUR_P2040::MaterialClass material;
    };
    using WallVector = std::vector<Wall, Eigen::aligned_allocator<Wall>>;

    // Represents a ray segment at a given point in time
    struct RayState {
        Eigen::Vector3d position;
        Eigen::Vector3d direction;     // k-vector (normalized)
        Eigen::Vector3cd eField;       // Complex 3D Electric Field vector
        double distanceTraveled_m = 0.0;
    };

    /// @brief Represents a virtual transmitter mirrored across a sequence of walls
    struct ImageNode {
        Eigen::Vector3d position;
        int wallIndex;   // Wall where this node was mirrored across (-1 for root Tx)
        int parentIndex; // Parent image node in the flat tree array (-1 for root Tx)
    };
    using ImageNodeVector = std::vector<ImageNode, Eigen::aligned_allocator<ImageNode>>;

    struct RayPath {
        std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d>> nodes;
        WallVector hitWalls; 
    };
    using RayPathVector = std::vector<RayPath, Eigen::aligned_allocator<RayPath>>;
}
#endif // TYPES_H