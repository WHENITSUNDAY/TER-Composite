#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>
#include <memory>
#include <Eigen/Dense>

// Classe de base abstraite pour implémentation commune
class Material {
protected:
    double _rho;
    std::string _name;

public:
    Material(const std::string& materialName, double density);
    virtual ~Material() = default;

    // Méthodes implémentées
    double getDensity() const { return _rho; }
    std::string getName() const { return _name; }
    
    // Méthodes virtuelles pures
    virtual Eigen::Matrix3d getC() const = 0;
    virtual std::string getType() const = 0;
};

class Fiber : public Material {
    // Fibre : matériau orthotrope avec 5 paramètres indépendants
private:
    double _E1, _E2;
    double _nu1, _nu2;
    double _G12;

public:
    Fiber(double E1, double E2, double nu1, double nu2, double G12, double rho);
    
    Eigen::Matrix3d getC() const override;
    std::string getType() const override { return "Orthotropic"; }
    
    // Getters spécifiques
    double getE1() const { return _E1; }
    double getE2() const { return _E2; }
    double getNu1() const { return _nu1; }
    double getNu2() const { return _nu2; }
    double getG12() const { return _G12; }
};

class IsotropicMaterial : public Material {
    // Matrice : matériau isotrope avec 2 paramètres indépendants (Young et Poisson)
private:
    double _E;
    double _nu;

public:
    IsotropicMaterial(double E, double nu, double rho);
    
    Eigen::Matrix3d getC() const override;
    std::string getType() const override { return "Isotropic"; }
    
    // Getters spécifiques
    double getE() const { return _E; }
    double getNu() const { return _nu; }
};

#endif
