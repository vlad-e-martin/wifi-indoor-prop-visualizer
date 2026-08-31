#include <gtest/gtest.h>

#include "ReflectionEngine.h"

#include <cmath>
#include <numbers>

using namespace RfSimulation;
using namespace ITUR_P2040;

class ReflectionEngineTest : public ::testing::Test {
protected:
    // Calculate magnitude of the reflection coefficient (|Gamma|)
    double getReflectionMagnitude(
        double angleOfIncidence_rad,
        double thickness_m,
        double freq_GHz,
        MaterialClass material,
        bool isTE
    ) {
        std::complex<double> reflCoeff_cmplx = ReflectionEngine::calcSingleLayerSlabReflection(
            angleOfIncidence_rad, thickness_m, freq_GHz, material, isTE);
        return std::abs(reflCoeff_cmplx);
    }
};

TEST_F(ReflectionEngineTest, DenserMaterialsReflectMore) {
    // 5 GHz, normal incidence (0 deg), 15cm thickness, TE polarized
    double gammaMag_concrete = getReflectionMagnitude(0.0, 0.15, 5.0, MaterialClass::Concrete, true);
    double gammaMag_brick = getReflectionMagnitude(0.0, 0.15, 5.0, MaterialClass::Brick, true);
    double gammaMag_drywall = getReflectionMagnitude(0.0, 0.15, 5.0, MaterialClass::Plasterboard, true);
    double gammaMag_wood = getReflectionMagnitude(0.0, 0.15, 5.0, MaterialClass::Wood, true);

    // Expect denser materials with higher relative permittivity to reflect more (all else equal)
    EXPECT_GT(gammaMag_concrete, gammaMag_brick);
    EXPECT_GT(gammaMag_brick, gammaMag_drywall);
    EXPECT_GT(gammaMag_drywall, gammaMag_wood);

    // Sanity bounds checking
    EXPECT_LT(gammaMag_concrete, 1.0);
    EXPECT_GT(gammaMag_wood, 0.0);
}

TEST_F(ReflectionEngineTest, AnglesCloserToParallelReflectMore) {
    // 2.4 GHz, concrete material, 15cm thickness, TE polarized
    double gammaMag_headOn  = getReflectionMagnitude(0.0, 0.15, 2.4, MaterialClass::Concrete, true);
    // pi = 180 --> pi/4 = 45 deg
    double gammaMag_oblique = getReflectionMagnitude(std::numbers::pi / 4.0, 0.15, 2.4, MaterialClass::Concrete, true);
    // pi = 180 --> 17 * pi / 36 = 85 deg
    double gammaMag_grazing = getReflectionMagnitude(17 * std::numbers::pi / 36, 0.15, 2.4, MaterialClass::Concrete, true);

    // As angle increases from normal (0) to grazing (pi/2), TE reflection increases
    EXPECT_LT(gammaMag_headOn, gammaMag_oblique);
    EXPECT_LT(gammaMag_oblique, gammaMag_grazing);
}

TEST_F(ReflectionEngineTest, BrewsterAngleDipForTM) {
    // For TM polarization, reflection magnitude drops significantly near Brewster's angle
    // Brewster's angle is ~= atan(sqrt(eps_r))
    // Conrete has eps_r = 5.24 --> atan(sqrt(5.24)) = 1.165 rad --> 66.76 degrees
    
    double brewsterAngle_rad = 1.165;
    
    double gammaMag_TE = getReflectionMagnitude(brewsterAngle_rad, 0.15, 5.0, MaterialClass::Concrete, true);
    double gammaMag_TM = getReflectionMagnitude(brewsterAngle_rad, 0.15, 5.0, MaterialClass::Concrete, false);

    // TM reflection should be substantially lower than TE at this angle
    EXPECT_LT(gammaMag_TM, gammaMag_TE);
    
    // TM reflection at the Brewster angle should be at a minimum, so it should be lower than any other angle for TM reflection 
    double gammaMag_TM_headOn = getReflectionMagnitude(0.0, 0.15, 5.0, MaterialClass::Concrete, false);
    double gammaMag_TM_belowBrewster = getReflectionMagnitude(0.5, 0.15, 5.0, MaterialClass::Concrete, false);
    double gammaMag_TM_aboveBrewster = getReflectionMagnitude(1.5, 0.15, 5.0, MaterialClass::Concrete, false);
    EXPECT_LT(gammaMag_TM, gammaMag_TM_headOn);
    EXPECT_LT(gammaMag_TM, gammaMag_TM_belowBrewster);
    EXPECT_LT(gammaMag_TM, gammaMag_TM_aboveBrewster);
}

TEST_F(ReflectionEngineTest, ThicknessAltersReflectionMagnitude) {
    // 5 GHz, brick material, normal angle, TE polarized
    // 5 GHz wavelength is ~6cm, pick thickness values around this 
    double gammaMag_1mm = getReflectionMagnitude(0.0, 0.001, 5.0, MaterialClass::Brick, true);
    double gammaMag_1cm = getReflectionMagnitude(0.0, 0.01, 5.0, MaterialClass::Brick, true);
    double gammaMag_10cm = getReflectionMagnitude(0.0, 0.10, 5.0, MaterialClass::Brick, true);
    double gammaMag_1m = getReflectionMagnitude(0.0, 1.0, 5.0, MaterialClass::Brick, true);

    // Reflection magnitudes should be lower for thinner slabs because they will allow more energy to pass through
    EXPECT_LT(gammaMag_1mm, gammaMag_1cm);
    EXPECT_LT(gammaMag_1cm, gammaMag_10cm);
    EXPECT_LT(gammaMag_10cm, gammaMag_1m);
    

}