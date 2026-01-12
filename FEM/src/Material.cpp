#include "Material.h"
#include <iostream>

using namespace Eigen;
using namespace std;

Material::Material(const string& materialName, double density) 
    : _rho(density), _name(materialName) {}


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

unique_ptr<IMaterial> IsotropicMaterial::clone() const {
    return unique_ptr<IMaterial>(new IsotropicMaterial(_E, _nu, _rho));
}
