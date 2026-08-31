#include "ReflectionEngine.h"

#include <cmath>
#include <numbers>

namespace RfSimulation {
    std::complex<double> ReflectionEngine::calcSingleLayerSlabReflection(
            double angleOfIncidence_rad, 
            double thickness_m, 
            double freq_GHz, 
            ITUR_P2040::MaterialClass materialClass, 
            bool isTE) 
    {
        using namespace std::complex_literals;
        
        // Extract complex permittivity from Table 3 in ITU-R P.2040
        std::complex<double> eps_relCmplx = ITUR_P2040::calcComplexPermittivity(materialClass, freq_GHz);
        
        // Common terms used when calculating reflection coefficient
        const double cos_theta = std::cos(angleOfIncidence_rad);
        const double sin_theta = std::sin(angleOfIncidence_rad);
        const double sin2_theta = sin_theta * sin_theta;
        std::complex<double> root_term = std::sqrt(eps_relCmplx - sin2_theta);

        // Calculate R' using simplified equations (assuming that the propagation medium is air)
        std::complex<double> R_prime;
        if (isTE) {
            // Euqation #37a
            R_prime = (cos_theta - root_term) / (cos_theta + root_term);
        } else {
            // Equation #37b
            R_prime = (eps_relCmplx * cos_theta - root_term) / (eps_relCmplx * cos_theta + root_term);
        }

        // Calculate q from Equation #44
        const double wavelength_m = kSpeedOfLight_mPerSec / freq_GHz;
        const std::complex<double> q = (2.0 * std::numbers::pi * thickness_m / wavelength_m) * root_term;

        // Calculate reflection coefficient for a single slab made of a single layer with a known thickness from Equation #43a
        const std::complex<double> exp_term = std::exp(-2.0i * q);
        const std::complex<double> numerator = R_prime * (1.0 - exp_term);
        const std::complex<double> denominator = 1.0 - (R_prime * R_prime * exp_term);
        return numerator / denominator;
    }
}