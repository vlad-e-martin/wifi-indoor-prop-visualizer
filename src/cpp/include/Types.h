#ifndef TYPES_H
#define TYPES_H

#include "ITUR_P2040.h"

#include <Eigen/Dense>
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

    // Represents a ray segment at a given point in time
    struct RayState {
        Eigen::Vector3d position;
        Eigen::Vector3d direction;     // k-vector (normalized)
        Eigen::Vector3cd eField;       // Complex 3D Electric Field vector
        double distanceTraveled_m = 0.0;
    };

    struct RayPath {
        std::vector<Eigen::Vector3d> nodes;
        std::vector<Wall> hitWalls; 
    };
}
#endif // TYPES_H