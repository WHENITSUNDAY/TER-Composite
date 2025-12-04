#include "Material.h"
#include <iostream>

using namespace Eigen;
using namespace std;

Material::Material(const string& materialName, double density) 
    : _rho(density), _name(materialName) {}


Fiber::Fiber(double E1, double E2, double nu1, double nu2, double G12, double rho)
    : Material("Fiber", rho), _E1(E1), _E2(E2), _nu1(nu1), _nu2(nu2), _G12(G12) {}

Matrix3d Fiber::getC() const {
    double nu21 = _nu2 * (_E2 / _E1);
    double delta = 1.0 - _nu1 * nu21;
    
    Matrix3d C;
    C(0, 0) = _E1 / delta;
    C(0, 1) = _nu1 * _E2 / delta;
    C(0, 2) = 0.0;
    
    C(1, 0) = _nu1 * _E2 / delta;
    C(1, 1) = _E2 / delta;
    C(1, 2) = 0.0;
    
    C(2, 0) = 0.0;
    C(2, 1) = 0.0;
    C(2, 2) = _G12;
    
    return C;
}

IsotropicMaterial::IsotropicMaterial(double E, double nu_val, double rho)
    : Material("Isotropic", rho), _E(E), _nu(nu_val) {}

Matrix3d IsotropicMaterial::getC() const {
    // Matrice de rigidité pour matériau isotrope en contraintes planes
    double factor = _E / (1.0 - _nu * _nu);
    
    Matrix3d C;
    C(0, 0) = factor;
    C(0, 1) = factor * _nu;
    C(0, 2) = 0.0;
    
    C(1, 0) = factor * _nu;
    C(1, 1) = factor;
    C(1, 2) = 0.0;
    
    C(2, 0) = 0.0;
    C(2, 1) = 0.0;
    C(2, 2) = factor * (1.0 - _nu) / 2.0;
    
    return C;
}
