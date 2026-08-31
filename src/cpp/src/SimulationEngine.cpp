#include "SimulationEngine.h"
#include "ReflectionEngine.h"

#include <cmath>

namespace RfSimulation {
    // Helper for 2D cross product of 3D vectors (ignoring Z)
    namespace {
        double crossProduct2D(const Eigen::Vector3d& v, const Eigen::Vector3d& w) {
            return v.x() * w.y() - v.y() * w.x();
        }
    }

    std::optional<Eigen::Vector3d> SimulationEngine::getIntersection(
        const Eigen::Vector3d& p1, const Eigen::Vector3d& p2, const Wall& wall) 
    {
        Eigen::Vector3d rayVec = p2 - p1;
        Eigen::Vector3d wallVec = wall.end - wall.start;
        Eigen::Vector3d w1_minus_p1 = wall.start - p1;

        double denominator = crossProduct2D(rayVec, wallVec);

        // If denominator is very close to 0, lines are parallel or collinear
        if (std::abs(denominator) < 1e-8) {
            return std::nullopt; 
        }

        double t = crossProduct2D(w1_minus_p1, wallVec) / denominator;
        double u = crossProduct2D(w1_minus_p1, rayVec) / denominator;

        // Check if the intersection happens strictly within both line segments
        if (t >= 0.0 && t <= 1.0 && u >= 0.0 && u <= 1.0) {
            // Calculate the point of intersection
            // NOTE: Interpolation of the intersection point's Z-coordinate is feasible because the wall is vertical
            Eigen::Vector3d intersectionPt = p1 + t * rayVec;
            return intersectionPt;
        }

        return std::nullopt;
    }

    int SimulationEngine::countWallIntersections(
        const Eigen::Vector3d& startPt, const Eigen::Vector3d& endPt, const std::vector<Wall>& walls) 
    {
        int count = 0;
        for (const auto& wall : walls) {
            if (getIntersection(startPt, endPt, wall).has_value()) {
                count++;
            }
        }
        return count;
    }

    // NOTE: Assumes a vertically oriented dipole antenna (Z-axis)
    static RayState SimulationEngine::initializeTxRay(const Eigen::Vector3d& txPos, const Eigen::Vector3d& launchDir) {
        RayState ray;
        ray.position = txPos;
        ray.direction = launchDir.normalized();
        
        // We assume the WiFi antennae are vertically-oriented, so the E-field starts purely vertical (Z)
        // NOTE: An EM wave's E-field MUST be orthogonal to its travel direction
        // We find the initial E-field by forcing it to be orthogonal to the ray's direction
        Eigen::Vector3d zAxis(0.0, 0.0, 1.0);
        Eigen::Vector3d initialEField = zAxis - (zAxis.dot(ray.direction)) * ray.direction;
        
        // Normalize and convert to complex vector to apply complex reflection coefficient
        initialEField.normalize();
        ray.eField = initialEField.cast<std::complex<double>>();
        
        return ray;
    }

    // Process a single bounce against a wall/floor/ceiling
    static void SimulationEngine::processReflection(
        RayState& ray, 
        const Eigen::Vector3d& intersectionPt, 
        const Eigen::Vector3d& surfaceNormal, 
        double wallThickness_m, 
        double freq_GHz, 
        ITUR_P2040::MaterialClass materialClass) 
    {
        using namespace std::complex_literals;

        // Update distance and position
        ray.distanceTraveled_m += (intersectionPt - ray.position).norm();
        ray.position = intersectionPt;

        // Calculate angle of incidence of the ray and the surface it is bouncing off
        double cosTheta = std::abs(ray.direction.dot(surfaceNormal));
        double angleOfIncidence_rad = std::acos(cosTheta);

        // Calculate reflection coefficients for TE & TM components of the ray
        std::complex<double> R_TE = ReflectionEngine::calcSlabReflection(
            angleOfIncidence_rad, wallThickness_m, freq_GHz, materialClass, true);
        std::complex<double> R_TM = ReflectionEngine::calcSlabReflection(
            angleOfIncidence_rad, wallThickness_m, freq_GHz, materialClass, false);

        // Define Basis Vectors
        Eigen::Vector3d rayDir = ray.direction;
        Eigen::Vector3d surfaceNormal = surfaceNormal.normalized();
        
        Eigen::Vector3d u_TE = rayDir.cross(surfaceNormal);
        
        // Handle normal incidence edge case (ray hits exactly head-on)
        if (u_TE.norm() < 1e-6) {
            // If head on, TE and TM are arbitrary orthogonal vectors on the surface
            u_TE = Eigen::Vector3d(1, 0, 0).cross(surfaceNormal);
            if (u_TE.norm() < 1e-6) u_TE = Eigen::Vector3d(0, 1, 0).cross(surfaceNormal);
        }
        u_TE.normalize();

        Eigen::Vector3d u_TMinc = u_TE.cross(rayDir).normalized();

        // Use Snell's law to calculate ray's new reflected direciton
        Eigen::Vector3d reflectedDir = rayDir - 2.0 * (rayDir.dot(surfaceNormal)) * surfaceNormal;
        reflectedDir.normalize();
        Eigen::Vector3d u_TMrefl = u_TE.cross(reflectedDir).normalized();

        // E. Decompose incoming E-field
        std::complex<double> E_TE = ray.eField.dot(u_TE.cast<std::complex<double>>());
        std::complex<double> E_TM = ray.eField.dot(u_TMinc.cast<std::complex<double>>());

        // Scale the ray's electric field by the outgoing TE & TM components
        std::complex<double> E_TE_ref = E_TE * R_TE;
        std::complex<double> E_TM_ref = E_TM * R_TM;
        ray.eField = E_TE_ref * u_TE.cast<std::complex<double>>() + 
                        E_TM_ref * u_TMrefl.cast<std::complex<double>>();
        
        ray.direction = reflectedDir;
    }
}