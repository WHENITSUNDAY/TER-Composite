#include "Solver.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <chrono>
#include <Eigen/IterativeLinearSolvers>

using namespace std;
using namespace Eigen;

Solver::Solver(Mesh& mesh, double tolerance, int maxIterations)
    : _mesh(mesh), _tol(tolerance), _maxIter(maxIterations) {
    
    int nbDofs = 2 * _mesh.nbNodes();
    
    _U.resize(nbDofs);
    _U.setZero();
    
    _F.resize(nbDofs);
    _F.setZero();
    
    _K.resize(nbDofs, nbDofs);
}

void Solver::setDirichletBC(int nodeId, int dof, double value) {
    int globalDof = 2 * (nodeId - 1) + dof;
    _dirichletBCs[globalDof] = value;
}

void Solver::setNeumannBC(int nodeId, int dof, double value) {
    int globalDof = 2 * (nodeId - 1) + dof;
    _neumannBCs[globalDof] = value;
}

void Solver::clearBCs() {
    _dirichletBCs.clear();
    _neumannBCs.clear();
}

void Solver::assemble() {
    vector<Triplet<double>> triplets;
    
    for (const auto& elem : _mesh.elements) {
        if (elem.Ke.norm() < 1e-20) continue;
        
        vector<int> dofMap;
        for (int node : elem.nodeIds) {
            dofMap.push_back(2*(node-1));
            dofMap.push_back(2*(node-1)+1);
        }
        
        for (int i = 0; i < 6; i++) {
            for (int j = 0; j < 6; j++) {
                triplets.push_back(Triplet<double>(dofMap[i], dofMap[j], elem.Ke(i,j)));
            }
        }
    }
    
    _K.setFromTriplets(triplets.begin(), triplets.end());
    cout << "Assemblage : Matrice " << _K.rows() << "x" << _K.cols() << ", nnz = " << _K.nonZeros() << endl;
}

void Solver::applyBC() {
    // Appliquer les forces (Neumann BC)
    for (const auto& force : _neumannBCs) {
        int dof = force.first;
        double value = force.second;
        _F(dof) += value;
    }
    
    // Appliquer les déplacements imposés (Dirichlet BC)
    for (const auto& disp : _dirichletBCs) {
        int dof = disp.first;
        double value = disp.second;
        
        // Modifier la matrice K: mettre 1 sur la diagonale, 0 ailleurs dans la ligne/colonne
        for (int j = 0; j < _K.cols(); ++j) {
            if (j != dof) {
                _K.coeffRef(dof, j) = 0.0;
                _K.coeffRef(j, dof) = 0.0;
            }
        }
        _K.coeffRef(dof, dof) = 1.0;
        _F(dof) = value;
    }
    
    cout << "CL : " << _dirichletBCs.size() << " déplacements imposés, " 
         << _neumannBCs.size() << " forces appliquées" << endl;
}

void Solver::solveConjugateGradient() {
    cout << "Résolution..." << endl;

    // Gradient conjugué préconditionné avec Incomplete Cholesky
    ConjugateGradient<SparseMatrix<double>, Lower|Upper, IncompleteCholesky<double>> solver;
    solver.setTolerance(1e-12);
    solver.setMaxIterations(10000);
    solver.compute(_K);
    
    if (solver.info() != Success) {
        cerr << "Erreur : échec de l'initialisation du gradient conjugué préconditionné" << endl;
        return;
    }
    
    // lancer le chrono
    
    auto t0 = std::chrono::high_resolution_clock::now();
    _U = solver.solve(_F);
    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = t1 - t0;

    if (solver.info() != Success) {
        cerr << "Erreur : le gradient conjugué préconditionné n'a pas convergé" << endl;
        cerr << "Itérations: " << solver.iterations() << ", erreur: " << solver.error() << endl;
        cerr << "Temps de résolution: " << elapsed.count() << " s" << endl;
        return;
    }

    // Afficher nombre d'itérations et temps
    cout << "Gradient conjugué préconditionné: itérations = " << solver.iterations()
         << ", erreur = " << solver.error()
         << ", temps = " << elapsed.count() << " s" << endl;

    
    cout << "Résolution terminée" << endl;
}

void Solver::saveResults(const string& filename) const {
    ofstream file(filename);
    
    if (!file.is_open()) {
        cerr << "Erreur : impossible d'ouvrir " << filename << endl;
        return;
    }
    
    file << "# Résultats de la simulation FEM\n";
    file << "# NodeID X Y Ux Uy Unorm\n";
    
    for (const auto& node : _mesh.nodes) {
        double ux = _U(2*(node.id-1));
        double uy = _U(2*(node.id-1)+1);
        double unorm = sqrt(ux*ux + uy*uy);
        file << node.id << " " << node.coords.x() << " " << node.coords.y() 
             << " " << ux << " " << uy << " " << unorm << "\n";
    }
    
    file.close();
    cout << "Résultats sauvegardés dans " << filename << endl;
}

void Solver::saveVTK(const string& filename) const {
    ofstream file(filename);
    
    if (!file.is_open()) {
        cerr << "Erreur : impossible d'ouvrir " << filename << endl;
        return;
    }
    
    // En-tête VTK
    file << "# vtk DataFile Version 3.0\n";
    file << "FEM Results\n";
    file << "ASCII\n";
    file << "DATASET UNSTRUCTURED_GRID\n\n";
    
    // Points
    file << "POINTS " << _mesh.nbNodes() << " float\n";
    for (const auto& node : _mesh.nodes) {
        file << node.coords.x() << " " << node.coords.y() << " 0.0\n";
    }
    file << "\n";
    
    // Cellules (triangles)
    file << "CELLS " << _mesh.nbElements() << " " << (4 * _mesh.nbElements()) << "\n";
    for (const auto& elem : _mesh.elements) {
        file << "3 " << (elem.nodeIds[0]-1) << " " << (elem.nodeIds[1]-1) << " " << (elem.nodeIds[2]-1) << "\n";
    }
    file << "\n";
    
    // Types de cellules (5 = triangle)
    file << "CELL_TYPES " << _mesh.nbElements() << "\n";
    for (int i = 0; i < _mesh.nbElements(); i++) {
        file << "5\n";
    }
    file << "\n";
    
    // Données aux noeuds
    file << "POINT_DATA " << _mesh.nbNodes() << "\n";
    
    // Vecteur déplacement
    file << "VECTORS U float\n";
    for (const auto& node : _mesh.nodes) {
        file << _U(2*(node.id-1)) << " " << _U(2*(node.id-1)+1) << " 0.0\n";
    }
    
    file.close();
    cout << "Fichier VTK sauvegardé: " << filename << endl;
}
