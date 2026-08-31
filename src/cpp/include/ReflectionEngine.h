#ifndef REFLECTION_ENGINE_H
#define REFLECTION_ENGINE_H

#include "ITUR_P2040.h"
#include <complex>

namespace RfSimulation {
    constexpr double kSpeedOfLight_mPerSec = 299792458.0;

    class ReflectionEngine {
    public:
        /// @brief Calculates the reflection coefficient associated with a slab of a given material
        /// @param angleOfIncidence_rad The angle of incidence of the ray hitting the slab (rad)
        /// @param thickness_m The thickness of the slab the ray is incident on (m)
        /// @param freq_GHz Frequency (GHz)
        /// @param materialClass Class of material of interest
        /// @param isTE Indicates whether the incident electric vector is transverse electric polarization 
        ///             (perpendicular to the plane of incidence) or not
        /// @return Reflection coefficient applied to a ray incident on a specific material
        static std::complex<double> calcSingleLayerSlabReflection(
            double angleOfIncidence_rad, 
            double thickness_m, 
            double freq_GHz, 
            ITUR_P2040::MaterialClass materialClass, 
            bool isTE);
    };
}
#endif // REFLECTION_ENGINE_H