#ifndef ITUR_P2040_H
#define ITUR_P2040_H

#include <complex>
#include <cmath>
#include <numbers>
#include <map>

namespace ITUR_P2040 {

    // Material classes pulled from Table 3 in ITU-R P.2040-4
    enum class MaterialClass {
        Vacuum,
        Concrete,
        Brick,
        Plasterboard, // Equivalent to drywall
        Wood
    };

    // Columns defined by Table 3 in ITU-R P.2040-4
    struct MaterialParams {
        double a;
        double b;
        double c;
        double d;
        double minFreq_GHz;
        double maxFreq_GHz;
    };

    // Table 3 data (using the 1-100 GHz range entries for standard Wi-Fi)
    // NOTE: Only included select materials I expect indoor walls to be made of
    inline const std::map<MaterialClass, MaterialParams> kMaterialTable = {
        {MaterialClass::Vacuum,       {1.0,  0.0, 0.0,    0.0}},
        {MaterialClass::Concrete,     {5.24, 0.0, 0.0462, 0.7822, 1.0, 100.0}},
        {MaterialClass::Concrete,     {5.17, 0.0, 0.0145, 1.0900, 110.0, 330.0}},
        {MaterialClass::Brick,        {3.91, 0.0, 0.0238, 0.1600, 1.0, 40.0}},
        {MaterialClass::Brick,        {4.15, 0.0, 0.0006, 1.5712, 110.0, 330.0}},
        {MaterialClass::Plasterboard, {2.73, 0.0, 0.0085, 0.9395, 1.0, 100.0}},
        {MaterialClass::Plasterboard, {2.56, 0.0, 0.0001, 1.7799, 110.0, 330.0}},
        {MaterialClass::Plasterboard, {2.65, 0.0, 0.0002, 1.5980, 100.0, 400.0}},
        {MaterialClass::Wood,         {1.99, 0.0, 0.0047, 1.0718, 0.001, 100.0}},
        {MaterialClass::Wood,         {1.82, 0.0, 0.0040, 1.0761, 110.0, 330.0}}
        {MaterialClass::Wood,         {2.1183, 0.0, 0.0055, 1.1113, 100.0, 400.0}}
    };

    /// @brief Calculates the real & imaginary parts of relative permittivity for a given building material
    /// @param materialClass Class of material of interest
    /// @param freq_GHz Frequency (GHz)
    /// @return Complex relative permittivity of the material of interest
    inline std::complex<double> calcComplexPermittivity(MaterialClass materialClass, double freq_GHz) {
        const auto& params = kMaterialTable.at(materialClass);

        // Equations #57
        double eps_r_real = params.a * std::pow(freq_GHz, params.b);
        // Equation #58
        double sigma = params.c * std::pow(freq_GHz, params.d);

        // Equation (59)
        double eps_r_imag = (17.98 * sigma) / freq_GHz;

        // Epsilon_rc = Epsilon' - j*Epsilon''
        return std::complex<double>(eps_r_real, -eps_r_imag);
    }
}
#endif // ITUR_P2040