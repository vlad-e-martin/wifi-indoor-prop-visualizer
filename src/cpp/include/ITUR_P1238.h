#ifndef ITUR_P1238_H
#define ITUR_P1238_H

#include "Types.h"

#include <array>
#include <cmath>
#include <numbers>
#include <optional>
#include <algorithm>

namespace ITUR_P1238 {
    enum class EnvironmentType {
        Office,
        Corridor,
        Industrial,
        ConferenceRoom
    };

   struct LossCoefficients {
        EnvironmentType envType;
        bool hasLoS;
        double minFreq_GHz;
        double maxFreq_GHz;
        double minDist_m;
        double maxDist_m;
        double alpha;
        double beta;
        double gamma;
        double sigma;
    };

    // Table 2 from ITU-R P.1238-13
    inline constexpr std::array<LossCoefficients, 8> kLossCoeffsTable {{
        {EnvironmentType::Office, true, 0.3, 294.0, 2.0, 27.0, 1.47, 34.17, 2.08, 3.68},
        {EnvironmentType::Office, false, 0.3, 255.0, 4.0, 30.0, 2.39, 30.13, 2.40, 5.01},
        {EnvironmentType::Corridor, true, 0.3, 300.0, 2.0, 160.0, 1.57, 29.46, 2.24, 3.77},
        {EnvironmentType::Corridor, false, 0.625, 159.0, 3.0, 94.0, 2.78, 28.62, 2.54, 7.58},
        {EnvironmentType::Industrial, true, 0.625, 294.0, 2.0, 102.0, 2.27, 24.79, 2.10, 2.62},
        {EnvironmentType::Industrial, false, 0.625, 255.0, 3.0, 110.0, 2.80, 23.55, 2.16, 5.70},
        {EnvironmentType::ConferenceRoom, true, 0.45, 300.0, 2.0, 21.0, 1.56, 30.47, 2.23, 2.92},
        {EnvironmentType::ConferenceRoom, false, 0.45, 159.0, 4.0, 25.0, 1.40, 39.53, 2.37, 3.33}
    }};

    // All required outputs for site-general loss formula
    struct SiteGeneralLoss {
        double basicLoss_dB;
        double meanExcessLoss_dB;
    };

    /// @brief Implements a site-general model applicable to situations where both the transmitting and receiving stations 
    ///        are located on the same floor
    /// @param freq_GHz operating frequency (GHz)
    /// @param distance3d_m 3D direct distance between the transmitting & receiving stations (m)
    /// @param hasLoS Indicates whether there is a LoS path between the transmitting & receiving stations
    /// @param envType Indicates the type of environment the stations are located within
    /// @return Basic transmission loss between transmitting and receiving stations on the same floor (dB)
    [[nodiscard]] inline std::optional<SiteGeneralLoss> calcSiteGeneralLoss_dB(
        const double freq_GHz, 
        const double distance3d_m, 
        const bool hasLoS, 
        const EnvironmentType envType) 
    {
        // Find the matching transmission coefficients
        auto it = std::find_if(kLossCoeffsTable.begin(), kLossCoeffsTable.end(),
            [envType, hasLoS](const LossCoefficients& coeffs) {
                return coeffs.envType == envType && coeffs.hasLoS == hasLoS;
            });

        if (it == kLossCoeffsTable.end()) {
            return std::nullopt; // Configuration not found
        }

        const auto& coeff = *it;

        // Enforce coefficient distance/frequency bounds
        if (freq_GHz < coeff.minFreq_GHz || freq_GHz > coeff.maxFreq_GHz || 
            distance3d_m < coeff.minDist_m || distance3d_m > coeff.maxDist_m) {
            return std::nullopt; 
        }

        // Equation #1 from ITU-R P.1238-13
        const double basicLoss_dB = coeff.alpha * 10.0 * std::log10(distance3d_m) + 
                                    coeff.beta + 
                                    coeff.gamma * 10.0 * std::log10(freq_GHz);
    
        const double fspl_dB = 20.0 * std::log10(4.0e9 * std::numbers::pi * distance3d_m * freq_GHz / kSpeedOfLight_mPerSec);
        
        // To be used to generate random normal distribution to calculate excess loss in NLoS Monte Carlo simulations
        const double meanExcessLoss_dB = basicLoss_dB - fspl_dB;

        return SiteGeneralLoss{basicLoss_dB, meanExcessLoss_dB};
    }
}

#endif // ITUR_P1238_H