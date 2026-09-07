#ifndef INDOOR_SIMULATOR_H
#define INDOOR_SIMULATOR_H

#include <string>
#include <vector>

#include "Types.h"

namespace RfSimulation {

    class IndoorSimulator {
    public:
        /// @brief Constructs the simulation environment from a JSON floor plan
        /// @param layoutJson Stringified JSON array of wall segments
        /// @param roomHeight_m Height of all walls (m)
        IndoorSimulator(const std::string& layoutJson, double roomHeight_m = 3.0);

        /// @brief Executes ray-tracing to calculate received power across heatmap
        /// @param txX Transmitter X coordinate (m)
        /// @param txY Transmitter Y coordinate (m)
        /// @param txZ Transmitter Z coordinate (m)
        /// @param freq_GHz Operating frequency (GHz)
        /// @param txPower_dBm Transmit power (dBm)
        /// @param gridWidth Number of grid points in heatmap along the X axis
        /// @param gridHeight Number of grid points in heatmap along the Y axis
        /// @param resolution_m Physical distance between grid points (m)
        /// @return A flat 1D array representing the 2D heatmap of received power (dBm)
        std::vector<double> generateHeatmap(
            double txX, double txY, double txZ, 
            double freq_GHz, double txPower_dBm, 
            int gridWidth, int gridHeight, double resolution_m);

    private:
        std::vector<Wall> m_walls;
        double m_roomHeight_m;
        
        /// @brief Parses the JSON payload and populates walls
        void parseEnvironment(const std::string& jsonStr);
    };
}

#endif // INDOOR_SIMULATOR_H