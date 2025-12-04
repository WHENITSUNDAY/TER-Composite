#include "Mesh.h"
#include "Material.h"
#include "Solver.h"
#include "MeshReader.h"
#include "BoundaryConditions.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <mesh_file.msh>" << endl;
        return 1;
    }
    
    string meshFile = argv[1];
    
    cout << "=== Solveur EF pour Composites ===" << endl;
    cout << "Fichier de maillage : " << meshFile << endl;
    
    // Créer un matériau isotrope simple pour test
    // Acier: E=200 GPa, nu=0.3
    IsotropicMaterial* material = new IsotropicMaterial(200.0e9, 0.3, 7850.0);
    
    cout << "\nMatériau utilisé:" << endl;
    cout << "  E = 200 GPa" << endl;
    cout << "  nu = 0.3" << endl;
    
    // Créer le maillage
    Mesh mesh;
    
    // Lire le fichier Gmsh avec MeshReader
    MeshReader reader(&mesh);
    reader.setMaterial(1, material);  // Tag 1 = matériau unique
    
    try {
        reader.readGmshFile(meshFile);
        
        // Initialiser les éléments (calcul aire et Ke)
        mesh.initializeElements();
    } catch (const exception& e) {
        cerr << "Erreur lors de la lecture du maillage : " << e.what() << endl;
        delete material;
        return 1;
    }
    
    if (mesh.getNbElements() == 0) {
        cerr << "Erreur : aucun élément chargé!" << endl;
        delete material;
        return 1;
    }
    
    // Créer le solveur
    Solver solver(mesh);
    
    // Définir les conditions aux limites
    BoundaryConditions bc;
    
    // Trouver les dimensions du domaine
    double xMax = 0.0;
    double yMax = 0.0;
    for (const auto& node : mesh.getNodes()) {
        xMax = max(xMax, node.coords.x());
        yMax = max(yMax, node.coords.y());
    }
    
    cout << "Dimensions du domaine: L=" << xMax << " m, H=" << yMax << " m" << endl;
    
    // Bloquer le bord gauche (x ~ 0)
    for (const auto& node : mesh.getNodes()) {
        if (abs(node.coords.x()) < 1e-6) {
            bc.fixNode(node.id);
        }
    }
    
    // Appliquer une force sur le bord droit (x ~ xMax)
    vector<int> rightNodes;
    for (const auto& node : mesh.getNodes()) {
        if (abs(node.coords.x() - xMax) < 1e-6) {
            rightNodes.push_back(node.id);
        }
    }
    
    double totalForce = 1000.0;
    double forcePerNode = totalForce / rightNodes.size();
    for (int nodeId : rightNodes) {
        bc.addForceX(nodeId, forcePerNode);
    }
    
    // Définir les CL dans le solveur
    solver.setBoundaryConditions(bc);
    
    solver.assemble();
    solver.applyBC();
    solver.solveConjugateGradient();
    solver.saveResults("../results/results.vtk");
    
    // Calcul théorique de vérification
    // Pour traction simple: epsilon = sigma/E = F/(A*E)
    // Allongement: delta_L = epsilon * L = (F * L) / (A * E)
    cout << "\nVérification (traction simple) :" << endl;
    double F = 1000.0; // Force appliquée en N
    double L = xMax;   // Longueur initiale en m
    double A = yMax;   // Aire de la section en m² (hauteur * épaisseur=1)
    double E = material->getE(); // Module de Young en Pa
    double delta_L = (F * L) / (A * E);
    cout << "Allongement théorique sous " << F << " N : " << delta_L << " m" << endl; 
    cout << "Allongement numérique obtenu : " << solver.getU().maxCoeff() << " m" << endl;
    
    // Nettoyage
    delete material;
    
    return 0;
}
