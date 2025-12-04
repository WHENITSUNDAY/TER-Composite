#include "BoundaryConditions.h"
#include <iostream>
#include <algorithm>

using namespace std;
using namespace Eigen;

void BoundaryConditions::addDirichlet(int ddl, double value) {
    _dirichletDdls.push_back(ddl);
    _dirichletValues.push_back(value);
}

void BoundaryConditions::fixNode(int nodeId) {
    int ddl_x = 2 * (nodeId - 1);
    int ddl_y = 2 * (nodeId - 1) + 1;
    addDirichlet(ddl_x, 0.0);
    addDirichlet(ddl_y, 0.0);
}

void BoundaryConditions::fixNodeX(int nodeId) {
    int ddl_x = 2 * (nodeId - 1);
    addDirichlet(ddl_x, 0.0);
}

void BoundaryConditions::fixNodeY(int nodeId) {
    int ddl_y = 2 * (nodeId - 1) + 1;
    addDirichlet(ddl_y, 0.0);
}

void BoundaryConditions::addForce(int nodeId, const Vector2d& force) {
    _neumannNodes.push_back(nodeId);
    _neumannForces.push_back(force);
}

void BoundaryConditions::addForceX(int nodeId, double fx) {
    addForce(nodeId, Vector2d(fx, 0.0));
}

void BoundaryConditions::addForceY(int nodeId, double fy) {
    addForce(nodeId, Vector2d(0.0, fy));
}

void BoundaryConditions::apply(SparseMatrix<double>& K, VectorXd& F) const {
    // Appliquer les forces (Neumann)
    for (size_t i = 0; i < _neumannNodes.size(); i++) {
        int nodeId = _neumannNodes[i];
        int ddl_x = 2 * (nodeId - 1);
        int ddl_y = 2 * (nodeId - 1) + 1;
        
        F(ddl_x) += _neumannForces[i].x();
        F(ddl_y) += _neumannForces[i].y();
    }
    
    // Appliquer les conditions de Dirichlet
    // Créer un set pour accès rapide
    vector<bool> isBlocked(K.rows(), false);
    for (size_t i = 0; i < _dirichletDdls.size(); i++) {
        isBlocked[_dirichletDdls[i]] = true;
    }
    
    // Reconstruire K en mettant les lignes/colonnes bloquées à l'identité
    vector<Triplet<double>> triplets;
    triplets.reserve(K.nonZeros());
    
    for (int k = 0; k < K.outerSize(); ++k) {
        for (SparseMatrix<double>::InnerIterator it(K, k); it; ++it) {
            int row = it.row();
            int col = it.col();
            
            if (isBlocked[row] || isBlocked[col]) {
                if (row == col) {
                    triplets.push_back(Triplet<double>(row, col, 1.0));
                }
            } else {
                triplets.push_back(Triplet<double>(row, col, it.value()));
            }
        }
    }
    
    K.setFromTriplets(triplets.begin(), triplets.end());
    
    // Mettre le second membre aux valeurs imposées
    for (size_t i = 0; i < _dirichletDdls.size(); i++) {
        F(_dirichletDdls[i]) = _dirichletValues[i];
    }
}

void BoundaryConditions::clear() {
    _dirichletDdls.clear();
    _dirichletValues.clear();
    _neumannNodes.clear();
    _neumannForces.clear();
}

void BoundaryConditions::printSummary() const {
    cout << "Conditions aux limites:" << endl;
    cout << "  - Dirichlet (DDL bloqués): " << _dirichletDdls.size() << endl;
    cout << "  - Neumann (forces): " << _neumannNodes.size() << " noeuds" << endl;
    
    // Calculer force totale
    Vector2d totalForce(0, 0);
    for (const auto& f : _neumannForces) {
        totalForce += f;
    }
    cout << "  - Force totale: (" << totalForce.x() << ", " << totalForce.y() << ") N" << endl;
}

