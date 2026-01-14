#include "Material.h"

using namespace Eigen;

Material::Material(double E_val, double nu_val, double rho_val)
    : E(E_val), nu(nu_val), rho(rho_val) {}

Matrix3d Material::getC() const {
    // Matrice de rigidité pour matériau isotrope en contraintes planes
    double factor = E / (1.0 - nu * nu);
    
    Matrix3d C;
    C(0, 0) = factor;
    C(0, 1) = factor * nu;
    C(0, 2) = 0.0;
    
    C(1, 0) = factor * nu;
    C(1, 1) = factor;
    C(1, 2) = 0.0;
    
    C(2, 0) = 0.0;
    C(2, 1) = 0.0;
    C(2, 2) = factor * (1.0 - nu) / 2.0;
    
    return C;
}
