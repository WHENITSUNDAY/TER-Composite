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
    
    int nbNodes = _mesh.getNbNodes();
    int nbDofs = 2 * nbNodes; // Nb de DDL, ici 2 par noeud (Ux, Uy)
    
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
    
    // Parcourir tous les éléments et assembler leurs matrices Ke
    for (const auto& elem : _mesh.getElements()) {
        const vector<int>& nodeIds = elem.getNodeIds();
        const Matrix<double, 6, 6>& Ke = elem.getKe();
        if (Ke.norm() < 1e-20) continue;
        
        // Assemblage dans la matrice globale
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                int dof_i_x = 2 * (nodeIds[i] - 1);
                int dof_i_y = 2 * (nodeIds[i] - 1) + 1;
                int dof_j_x = 2 * (nodeIds[j] - 1);
                int dof_j_y = 2 * (nodeIds[j] - 1) + 1;
                
                triplets.push_back(Triplet<double>(dof_i_x, dof_j_x, Ke(2*i, 2*j)));
                triplets.push_back(Triplet<double>(dof_i_x, dof_j_y, Ke(2*i, 2*j+1)));
                triplets.push_back(Triplet<double>(dof_i_y, dof_j_x, Ke(2*i+1, 2*j)));
                triplets.push_back(Triplet<double>(dof_i_y, dof_j_y, Ke(2*i+1, 2*j+1)));
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
    file << "# NodeID X Y Ux Uy\n";
    
    for (const auto& node : _mesh.getNodes()) {
        int nodeId = node.getId();
        Vector2d coords = node.getCoords();
        
        int dof_x = 2 * (nodeId - 1);
        int dof_y = 2 * (nodeId - 1) + 1;
        
        double ux = _U(dof_x);
        double uy = _U(dof_y);
        
        file << nodeId << " " << coords.x() << " " << coords.y() 
             << " " << ux << " " << uy << "\n";
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
    int nbNodes = _mesh.getNbNodes();
    file << "POINTS " << nbNodes << " float\n";
    for (const auto& node : _mesh.getNodes()) {
        Vector2d coords = node.getCoords();
        file << coords.x() << " " << coords.y() << " 0.0\n";
    }
    file << "\n";
    
    // Cellules (triangles)
    int nbElems = _mesh.getNbElements();
    file << "CELLS " << nbElems << " " << (4 * nbElems) << "\n";
    for (const auto& elem : _mesh.getElements()) {
        const vector<int>& nodeIds = elem.getNodeIds();
        file << "3 " << (nodeIds[0] - 1) << " " << (nodeIds[1] - 1) << " " << (nodeIds[2] - 1) << "\n";
    }
    file << "\n";
    
    // Types de cellules (5 = triangle)
    file << "CELL_TYPES " << nbElems << "\n";
    for (int i = 0; i < nbElems; i++) {
        file << "5\n";
    }
    file << "\n";
    
    // Données aux noeuds
    file << "POINT_DATA " << nbNodes << "\n";
    
    // Déplacement Ux (scalaire)
    file << "SCALARS Ux float 1\n";
    file << "LOOKUP_TABLE default\n";
    for (const auto& node : _mesh.getNodes()) {
        int nodeId = node.getId();
        int dof_x = 2 * (nodeId - 1);
        double ux = _U(dof_x);
        file << ux << "\n";
    }
    file << "\n";
    
    // Déplacement Uy (scalaire)
    file << "SCALARS Uy float 1\n";
    file << "LOOKUP_TABLE default\n";
    for (const auto& node : _mesh.getNodes()) {
        int nodeId = node.getId();
        int dof_y = 2 * (nodeId - 1) + 1;
        double uy = _U(dof_y);
        file << uy << "\n";
    }
    
    file.close();
    cout << "Fichier VTK sauvegardé: " << filename << endl;
}
