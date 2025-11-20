#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>
#include <Eigen/Dense>

class Material {
    // Classe virtuelle de base pour les matériaux et leurs propriétés
    protected:
        double _rho;
        std::string _name;

    public:
        Material(const std::string& materialName, double density);
        virtual ~Material() = default;

        virtual Eigen::Matrix3d getC() const = 0;

        double getDensity() const { return _rho; }
        std::string getName() const { return _name; }
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
};

class Matrix : public Material {
    // Matrice : matériau isotrope avec 2 paramètres indépendants (Young et Poisson)
    private:
        double _E;
        double nu;

    public:
        Matrix(double E, double nu, double rho);
        
        Eigen::Matrix3d getC() const override;
};

#endif
