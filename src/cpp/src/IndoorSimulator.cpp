#include "IndoorSimulator.h"
#include "SimulationEngine.h"

#include <nlohmann/json.hpp>

#include <cmath>
#include <iostream>
#include <numbers>

namespace RfSimulation {
    // Configuration constants
    // NOTE: Max # of reflections is 5 to minimize the number of nodes in the tree
    constexpr int kMaxBounces = 5;
    // Default constants
    constexpr double kAvgWallPenetrationLoss_dB = 4.0;

    namespace {
        ITUR_P2040::MaterialClass getMaterialFromString(const std::string& materialStr) {
            if (materialStr == "CONCRETE") return ITUR_P2040::MaterialClass::Concrete;
            if (materialStr == "BRICK") return ITUR_P2040::MaterialClass::Brick;
            if (materialStr == "WOOD") return ITUR_P2040::MaterialClass::Wood;
            return ITUR_P2040::MaterialClass::Plasterboard; 
        }

        // Maps material types to standard physical thicknesses in meters
        double getThicknessForMaterial(ITUR_P2040::MaterialClass material) {
            switch (material) {
                case ITUR_P2040::MaterialClass::Concrete:
                case ITUR_P2040::MaterialClass::Brick:
                    return 0.254; // ~10 inches
                case ITUR_P2040::MaterialClass::Wood:
                    return 0.2032; // ~8 inches
                case ITUR_P2040::MaterialClass::Plasterboard:
                default:
                    return 0.1524; // ~6 inches
            }
        }

        // Maps ITU-R materials to COST-231 empirical wall attenuation values (dB)
        // NOTE: Values pulled from Table 1 in this paper: https://thesai.org/Downloads/Volume12No4/Paper_94-Integrating_Cost_231_Multiwall_Propagation.pdf
        double getCost231WallLoss(ITUR_P2040::MaterialClass material) {
            switch (material) {
                case ITUR_P2040::MaterialClass::Concrete:
                case ITUR_P2040::MaterialClass::Brick:
                    return 12.0; // From Table 1: Brick, concrete, concrete block
                case ITUR_P2040::MaterialClass::Wood:
                case ITUR_P2040::MaterialClass::Plasterboard:
                default:
                    return 3.0;  // From Table 1: Wooden door / Drywall
            }
        }
    }

    IndoorSimulator::IndoorSimulator(const std::string& layoutJson, double roomHeight_m) 
        : m_roomHeight_m(roomHeight_m) {
        parseEnvironment(layoutJson);
    }

    void IndoorSimulator::parseEnvironment(const std::string& jsonString) {
        using json = nlohmann::json;
        auto jsonData = json::parse(jsonString);

        for (const auto& jsonEntry : jsonData) {
            Wall currentWall;
            // Map 2D UI coordinates to 3D space (assuming floor is Z=0)
            currentWall.start = Eigen::Vector3d(jsonEntry["x1"], jsonEntry["y1"], 0.0);
            currentWall.end = Eigen::Vector3d(jsonEntry["x2"], jsonEntry["y2"], 0.0);
            
            currentWall.height_m = m_roomHeight_m; 
            // Extract material, so that we can use it to set physical thickness
            currentWall.material = getMaterialFromString(jsonEntry["material"]);
            currentWall.thickness_m = getThicknessForMaterial(currentWall.material);
            
            m_walls.push_back(currentWall);
        }
    }

    std::vector<double> IndoorSimulator::generateHeatmap(
        double txX, double txY, double txZ, double freq_GHz, double txPower_dBm, 
        int gridWidth, int gridHeight, double resolution_m) 
    {
        std::cout << "Entered [generateHeatmap]" << std::endl;

        Eigen::Vector3d txPos(txX, txY, txZ);
        double txPower_W = std::pow(10.0, (txPower_dBm - 30.0) / 10.0);
        
        std::vector<double> heatmap(gridWidth * gridHeight, -100.0); 
        
        std::cout << "Allocated heatmap vector (size = " << heatmap.size() << ")" << std::endl;

        // Generate image tree associated with the current Tx position within the current floor plan
        // Max # of reflections is 5 to minimize the number of nodes in the tree
        std::vector<ImageNode> imageTree = SimulationEngine::generateImageTree(txPos, m_walls, kMaxBounces);

        const double lambda = kSpeedOfLight_mPerSec / (freq_GHz * 1e9);

        for (int y = 0; y < gridHeight; ++y) {
            if (y == 0) {
                std::cout << "Entered for loop to begin heatmap calculations" << std::endl;
            }
            if (y == 19) {
                std::cout << "Reached 20th row of heatmap calculations" << std::endl;
            }
            for (int x = 0; x < gridWidth; ++x) {
                // Receiver height is standard user device level (1.5m)
                Eigen::Vector3d rxPos(x * resolution_m, y * resolution_m, 1.5); 

                // Retrieve all valid paths from the pre-computed tree
                std::vector<RayPath> validPaths = SimulationEngine::computeValidPaths(txPos, rxPos, m_walls, imageTree);
                
                if (y == 0 && x == 0) {
                    std::cout << "Successfully calculated valid paths for the first cell in the heatmap" << std::endl;
                }

                if (!validPaths.empty()) {
                    // Use ray tracing to superimpose all valid rays 
                    // into the final received E-field (thus accounting for multipath effects)
                    Eigen::Vector3cd totalEField_Rx = Eigen::Vector3cd::Zero();

                    for (const auto& path : validPaths) {
                        // The Tx and Rx should always be nodes in a valid path
                        if (path.nodes.size() < 2) continue;

                        Eigen::Vector3d launchDir = path.nodes[1] - txPos;
                        RayState ray = SimulationEngine::initializeTxRay(txPos, launchDir);

                        // Update the ray and its E-field to account for each reflection along the path
                        for (size_t i = 1; i < path.nodes.size() - 1; ++i) {
                            const Eigen::Vector3d& hitPoint = path.nodes[i];
                            const Wall& hitWall = path.hitWalls[i-1]; 
                            
                            Eigen::Vector3d wallVec = hitWall.end - hitWall.start;
                            Eigen::Vector3d surfaceNormal(-wallVec.y(), wallVec.x(), 0.0);

                            SimulationEngine::processReflection(
                                ray, hitPoint, surfaceNormal, hitWall.thickness_m, freq_GHz, hitWall.material);
                        }

                        // Account for distance traveled along final reflected leg to the Rx
                        ray.distanceTraveled_m += (rxPos - ray.position).norm();
                        
                        double phaseShift = (2.0 * std::numbers::pi / lambda) * ray.distanceTraveled_m;
                        
                        // Apply FSPL via propagation factor as well as appropriate phase shift
                        std::complex<double> propagationFactor = std::exp(std::complex<double>(0, -phaseShift)) / ray.distanceTraveled_m;
                        
                        totalEField_Rx += ray.eField * propagationFactor;
                    }

                    // Convert Total E-Field magnitude squared back to dBm
                    double receivedPower_W = totalEField_Rx.squaredNorm() * txPower_W;
                    heatmap[y * gridWidth + x] = 10.0 * std::log10(receivedPower_W) + 30.0;
                    
                } else {
                    // Fall-back on COST-231 Multi-Wall Model
                    double distance3d_m = (rxPos - txPos).norm();
                    WallVector penetratedWalls = SimulationEngine::getIntersectedWalls(txPos, rxPos, m_walls);
                    
                    // Calculate base Free Space Path Loss
                    double fspl_dB = 20.0 * std::log10(4.0 * std::numbers::pi * distance3d_m / lambda);
                    
                    // Sum the empirical attenuation for each specific wall penetrated
                    double totalWallLoss_dB = 0.0;
                    for (const auto& wall : penetratedWalls) {
                        totalWallLoss_dB += getCost231WallLoss(wall.material);
                    }

                    // COST-231 L_MW = FSPL + Sum(n_w,i=1){L_wi} + n_f * L_f
                    // TODO: Update floor loss (L_f) to track intersections through floors later
                    double empiricalLoss_dB = fspl_dB + totalWallLoss_dB;
                    heatmap[y * gridWidth + x] = txPower_dBm - empiricalLoss_dB;
                }
            }
        }

        std::cout << "Finished heatmap calculations" << std::endl;
        return heatmap;
    }
}