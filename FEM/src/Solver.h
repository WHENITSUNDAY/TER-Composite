#ifndef SOLVER_H
#define SOLVER_H

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <vector>
#include "Mesh.h"
#include "Material.h"

class Solver {
    // Classe permettant de résoudre le système global KU=F avec la méthode du gradient conjugué.
    // Elle assemble également la matrice de rigidité globale K à partir des matrices élémentaires Ke (propre aux éléments)
    // Et applique les conditions aux limites sur le système.

    private:
        Mesh& _mesh;
        Eigen::VectorXd _U;
        Eigen::SparseMatrix<double> _K;
        Eigen::VectorXd _F;
        
        double _tol;
        int _maxIter;
        
    public:
        Solver(Mesh& mesh, double tolerance = 1e-6, int maxIterations = 1000);

        void assemble();
        void applyBC();
        void solveConjugateGradient(); 
        
        Eigen::VectorXd getU() const { return _U; }
        void saveResults(const std::string& filename) const;
};

#endif
