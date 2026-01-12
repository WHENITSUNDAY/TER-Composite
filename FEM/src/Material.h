#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>
#include <memory>
#include <Eigen/Dense>

// Interface pour les matériaux
class IMaterial {
public:
    virtual ~IMaterial() = default;
    
    // Méthodes virtuelles pures
    virtual Eigen::Matrix3d getC() const = 0;
    virtual double getDensity() const = 0;
    virtual std::string getName() const = 0;
    virtual std::string getType() const = 0;
    
    // Clone pattern pour copie polymorphique
    virtual std::unique_ptr<IMaterial> clone() const = 0;
};

// Classe de base abstraite pour implémentation commune
class Material : public IMaterial {
protected:
    double _rho;
    std::string _name;

public:
    Material(const std::string& materialName, double density);
    virtual ~Material() = default;

    // Implémentations de l'interface
    double getDensity() const override { return _rho; }
    std::string getName() const override { return _name; }
    
    // Reste abstrait
    virtual Eigen::Matrix3d getC() const = 0;
    virtual std::string getType() const = 0;
    virtual std::unique_ptr<IMaterial> clone() const = 0;
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
    std::unique_ptr<IMaterial> clone() const override;
    
    // Getters spécifiques
    double getYoungModulus() const { return _E; }
    double getPoissonRatio() const { return _nu; }
};

#endif
