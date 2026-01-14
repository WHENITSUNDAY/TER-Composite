#ifndef MATERIAL_H
#define MATERIAL_H

#include <Eigen/Dense>

// Classe simple pour matériau isotrope
class Material {
public:
    double E;   // Module de Young
    double nu;  // Coefficient de Poisson
    double rho; // Densité
    
    Material(double E_val, double nu_val, double rho_val);
    
    // Calcule la matrice de rigidité en contraintes planes
    Eigen::Matrix3d getC() const;
};

#endif
