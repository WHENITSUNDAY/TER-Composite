#include "Solver.h"
#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;
using namespace Eigen;

Solver::Solver(Mesh& mesh, double tolerance, int maxIterations)
    : _mesh(mesh), _tol(tolerance), _maxIter(maxIterations) {
    
    int nbNodes = _mesh.getNbNodes();
    int nbDdls = 2 * nbNodes; // Nb de DDL, ici 2 par noeud (Ux, Uy)
    
    _U.resize(nbDdls);
    _U.setZero();
    
    _F.resize(nbDdls);
    _F.setZero();
    
    _K.resize(nbDdls, nbDdls);
}

void Solver::assemble() {
    cout << "Assemblage de la matrice de rigidité globale..." << endl;
    
    vector<Triplet<double>> triplets;
    
    // Parcourir tous les éléments et assembler leurs matrices Ke
    for (const auto& elem : _mesh.getElements()) {
        const vector<int>& nodeIds = elem.nodeIds;
        const Matrix<double, 6, 6>& Ke = elem.Ke;
        
        // Vérifier que Ke n'est pas nulle
        if (Ke.norm() < 1e-20) continue;
        
        // Assemblage dans la matrice globale
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                int ddl_i_x = 2 * (nodeIds[i] - 1);
                int ddl_i_y = 2 * (nodeIds[i] - 1) + 1;
                int ddl_j_x = 2 * (nodeIds[j] - 1);
                int ddl_j_y = 2 * (nodeIds[j] - 1) + 1;
                
                triplets.push_back(Triplet<double>(ddl_i_x, ddl_j_x, Ke(2*i, 2*j)));
                triplets.push_back(Triplet<double>(ddl_i_x, ddl_j_y, Ke(2*i, 2*j+1)));
                triplets.push_back(Triplet<double>(ddl_i_y, ddl_j_x, Ke(2*i+1, 2*j)));
                triplets.push_back(Triplet<double>(ddl_i_y, ddl_j_y, Ke(2*i+1, 2*j+1)));
            }
        }
    }
    
    _K.setFromTriplets(triplets.begin(), triplets.end());
    cout << "Assemblage terminé. Taille de K : " << _K.rows() << "x" << _K.cols() << endl;
    cout << "Triplets assemblés: " << triplets.size() << endl;
    cout << "Éléments non-nuls dans K: " << _K.nonZeros() << endl;
}

void Solver::applyBC() {
    cout << "Application des conditions aux limites..." << endl;
    _bc.apply(_K, _F);
    _bc.printSummary();
}

void Solver::solveConjugateGradient() {
    cout << "Résolution par gradient conjugué..." << endl;
    
    // Utiliser SimplicialLDLT qui est plus robuste pour les matrices symétriques
    SimplicialLDLT<SparseMatrix<double>> solver;
    solver.compute(_K);
    
    if(solver.info() != Success) {
        cerr << "Erreur lors de la décomposition de la matrice" << endl;
        return;
    }
    
    _U = solver.solve(_F);
    
    if(solver.info() != Success) {
        cerr << "Erreur lors de la résolution" << endl;
        return;
    }
    
    cout << "Résolution terminée avec succès" << endl;
}

void Solver::saveResults(const string& filename) const {
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
        Vector2d coords = node.coords;
        file << coords.x() << " " << coords.y() << " 0.0\n";
    }
    file << "\n";
    
    // Cellules (triangles)
    int nbElems = _mesh.getNbElements();
    file << "CELLS " << nbElems << " " << (4 * nbElems) << "\n";
    for (const auto& elem : _mesh.getElements()) {
        const vector<int>& nodeIds = elem.nodeIds;
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
    
    // Déplacements (vecteurs)
    file << "VECTORS displacement float\n";
    for (const auto& node : _mesh.getNodes()) {
        int nodeId = node.id;
        int ddl_x = 2 * (nodeId - 1);
        int ddl_y = 2 * (nodeId - 1) + 1;
        
        double ux = _U(ddl_x);
        double uy = _U(ddl_y);
        
        file << ux << " " << uy << " 0.0\n";
    }
    file << "\n";
    
    // Magnitude du déplacement (scalaire)
    file << "SCALARS displacement_magnitude float 1\n";
    file << "LOOKUP_TABLE default\n";
    for (const auto& node : _mesh.getNodes()) {
        int nodeId = node.id;
        int ddl_x = 2 * (nodeId - 1);
        int ddl_y = 2 * (nodeId - 1) + 1;
        
        double ux = _U(ddl_x);
        double uy = _U(ddl_y);
        double magnitude = sqrt(ux*ux + uy*uy);
        
        file << magnitude << "\n";
    }
    
    file.close();
    cout << "Résultats sauvegardés dans " << filename << endl;
}
