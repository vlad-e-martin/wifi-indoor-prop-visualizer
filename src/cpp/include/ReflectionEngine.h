#ifndef REFLECTION_ENGINE_H
#define REFLECTION_ENGINE_H

#include "ITUR_P2040.h"
#include <complex>

namespace RfSimulation {
    class ReflectionEngine {
    public:
        /// @brief Calculates the complex reflection coefficient for a single-layer slab (e.g., a wall).
        /// 
        /// This implements ITU-R P.2040 to determine how much of an electric wave reflects off a wall (rather than penetrating it) 
        /// It accounts for the initial bounce off the front face and simplifies the math for the infinite series 
        /// of internal bounces occurring inside the slab that eventually transmit back out of the front face.
        ///
        /// @param angleOfIncidence_rad Angle between the incoming ray and the surface normal (0 = head-on)
        /// @param thickness_m Physical thickness of the wall slab (m)
        /// @param freq_GHz Frequency of the transmitting wave (GHz)
        /// @param materialClass Material classification (as categorized by the ITU-R P.2040)
        /// @param isTE True for Transverse Electric (E-field perpendicular to plane of incidence), False for TM
        /// @return Complex reflection coefficient (magnitude and phase shift)
        static std::complex<double> calcSingleLayerSlabReflection(
            double angleOfIncidence_rad, 
            double thickness_m, 
            double freq_GHz, 
            ITUR_P2040::MaterialClass materialClass, 
            bool isTE);
    };
}
#endif // REFLECTION_ENGINE_H