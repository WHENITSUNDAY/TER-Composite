#ifndef BOUNDARYCONDITIONS_H
#define BOUNDARYCONDITIONS_H

#include <vector>
#include <Eigen/Dense>
#include <Eigen/Sparse>

using namespace std;
using namespace Eigen;

enum class BCType {
    DIRICHLET,  // Déplacement imposé
    NEUMANN     // Force imposée
};

class BoundaryConditions {
private:
    // Conditions de Dirichlet (déplacements imposés)
    vector<int> _dirichletDofs;
    vector<double> _dirichletValues;
    
    // Conditions de Neumann (forces imposées)
    vector<int> _neumannNodes;
    vector<Vector2d> _neumannForces;
    
public:
    BoundaryConditions() = default;
    
    // Ajouter une condition de Dirichlet sur un DDL
    void addDirichlet(int dof, double value);
    
    // Bloquer complètement un noeud (ux=0, uy=0)
    void fixNode(int nodeId);
    
    // Bloquer seulement en X ou Y
    void fixNodeX(int nodeId);
    void fixNodeY(int nodeId);
    
    // Ajouter une force sur un noeud
    void addForce(int nodeId, const Vector2d& force);
    void addForceX(int nodeId, double fx);
    void addForceY(int nodeId, double fy);
    
    // Appliquer les conditions aux limites sur K et F
    void apply(SparseMatrix<double>& K, VectorXd& F) const;
    
    // Getters pour info
    int getNbDirichlet() const { return _dirichletDofs.size(); }
    int getNbNeumann() const { return _neumannNodes.size(); }
    
    void clear();
    void printSummary() const;
};

#endif
