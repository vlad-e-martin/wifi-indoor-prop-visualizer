#ifndef INDOOR_SIMULATOR_H
#define INDOOR_SIMULATOR_H

#include <string>
#include <vector>
#include "SimulationEngine.h"

namespace RfSimulation {
    class IndoorSimulator {
    public:
        IndoorSimulator(const std::string& layoutJson, double roomHeight_m = 3.0);

        std::vector<double> generateHeatmap(
            double txX, double txY, double txZ, 
            double freq_GHz, double txPower_dBm, 
            int gridWidth, int gridHeight, double resolution_m);

    private:
        std::vector<Wall> m_walls;
        double m_roomHeight_m;
        
        void parseEnvironment(const std::string& jsonStr);
    };
}

namespace RfSimulation {
    using json = nlohmann::json;
    // Map JSON string materials to our Enum
    ITUR_P2040::MaterialClass getMaterialFromString(const std::string& materialStr) {
        if (materialStr == "CONCRETE") return ITUR_P2040::MaterialClass::Concrete;
        if (materialStr == "BRICK") return ITUR_P2040::MaterialClass::Brick;
        if (materialStr == "WOOD") return ITUR_P2040::MaterialClass::Wood;
        return ITUR_P2040::MaterialClass::Plasterboard; // Default to Drywall
    }

    std::vector<RF_Sim::Wall> parseWallsFromJson(const std::string& jsonString) {
        std::vector<RF_Sim::Wall> walls;
        auto jsonData = json::parse(jsonString);

        for (const auto& jsonEntry : jsonData) {
            RF_Sim::Wall currentWall;
            // Map 2D UI coordinates to 3D space (Z=0 at floor)
            currentWall.start = Eigen::Vector3d(jsonEntry["x1"], jsonEntry["y1"], 0.0);
            currentWall.end = Eigen::Vector3d(jsonEntry["x2"], jsonEntry["y2"], 0.0);
            
            // Standard wall height and thickness
            currentWall.height_m = 3.0; 
            currentWall.thickness_m = 0.15; 
            currentWall.material = getMaterialFromString(jsonEntry["material"]);
            
            walls.push_back(currentWall);
        }
        return walls;
    }

    class IndoorSimulator {
    public:
        // Constructor parses the JSON layout and builds the wall objects
        IndoorSimulator(const std::string& layoutJson, double roomHeight_m = 3.0);

        // Executes the ray tracer across the grid and returns the flat heatmap array
        std::vector<double> generateHeatmap(
            const std::string& wallsJson, 
            double txX, double txY, double txZ, 
            double freq_GHz, double txPower_dBm, 
            int gridWidth, int gridHeight, double resolution_m) 
        {
            // Setup Environment
            std::vector<RF_Sim::Wall> walls = parseWallsFromJson(wallsJson);
            Eigen::Vector3d txPos(txX, txY, txZ);
            
            // Convert txPower from dBm to linear Watts for initial E-field amplitude
            double txPower_W = std::pow(10.0, (txPower_dBm - 30.0) / 10.0);
            
            // Flat array to hold the output dBm values for the frontend
            std::vector<double> heatmap(gridWidth * gridHeight, -100.0); 

            // 2. Iterate over every pixel/grid point in the room
            for (int y = 0; y < gridHeight; ++y) {
                for (int x = 0; x < gridWidth; ++x) {
                    
                    Eigen::Vector3d rxPos(x * resolution_m, y * resolution_m, 1.5); // Rx at 1.5m height
                    
                    // A. Generate valid geometric paths (Tx -> Wall Intersections -> Rx)
                    // (Assume ImageRayTracer::computePaths returns paths with valid intersection nodes)
                    std::vector<RF_Sim::RayPath> validPaths = imageTracer.computePaths(txPos, rxPos, walls);
                    
                    std::complex<double> totalEField_Rx(0.0, 0.0);

                    // B. Walk the ray through each valid path
                    for (const auto& path : validPaths) {
                        
                        // Initialize ray pointing towards the first node
                        Eigen::Vector3d launchDir = path.nodes[1] - txPos;
                        RF_Sim::RayState ray = RF_Sim::SimulationEngine::initializeTxRay(txPos, launchDir);

                        // Process each reflection bounce
                        for (size_t i = 1; i < path.nodes.size() - 1; ++i) {
                            const Eigen::Vector3d& hitPoint = path.nodes[i];
                            const RF_Sim::Wall& hitWall = path.hitWalls[i-1]; 
                            
                            // Normal vector of the wall
                            Eigen::Vector3d wallVec = hitWall.end - hitWall.start;
                            Eigen::Vector3d surfaceNormal(-wallVec.y(), wallVec.x(), 0.0);

                            RF_Sim::SimulationEngine::processReflection(
                                ray, hitPoint, surfaceNormal, hitWall.thickness_m, freq_GHz, hitWall.material);
                        }

                        // Final leg to the Rx
                        ray.distanceTraveled_m += (rxPos - ray.position).norm();
                        
                        // Apply Free Space Path Loss (FSPL) and Phase Shift to the complex E-Field
                        double lambda = 0.299792458 / freq_GHz;
                        double phaseShift = (2.0 * std::numbers::pi / lambda) * ray.distanceTraveled_m;
                        
                        std::complex<double> propagationFactor = std::exp(std::complex<double>(0, -phaseShift)) / ray.distanceTraveled_m;
                        
                        // Superposition: Add this path's field to the total received field
                        totalEField_Rx += ray.eField * propagationFactor;
                    }

                    // C. Convert Total E-Field back to dBm
                    double receivedPower_W = std::norm(totalEField_Rx) * txPower_W; // Simplified scaling
                    double receivedPower_dBm = 10.0 * std::log10(receivedPower_W) + 30.0;
                    
                    heatmap[y * gridWidth + x] = receivedPower_dBm;
                }
            }

            return heatmap;
        }

    private:
        std::vector<Wall> m_walls;
        double m_roomHeight_m;
        
        // Internal helper to deserialize the incoming JSON string
        void parseEnvironment(const std::string& jsonStr);
    };

} 
#endif