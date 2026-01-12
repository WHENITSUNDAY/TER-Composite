#include "Mesh.h"
#include "Material.h"
#include "Solver.h"
#include "MeshReader.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <Eigen/Dense>

using namespace std;
using Eigen::Vector2d;

using namespace std;
using Eigen::Vector2d;

// Fonction principale pour exécuter le test de traction
void runTractionTest(const string& meshFile) {
    cout << "Simulation Traction Simple FEM :" << endl;
    cout << "Maillage: " << meshFile << endl;
    cout << "Matériau: E=200 GPa, nu=0.3" << endl;
    
    // Créer un matériau isotrope simple pour test, Acier: E=200 GPa, nu=0.3
    IsotropicMaterial* material = new IsotropicMaterial(200.0e9, 0.3, 7850.0);

    Mesh mesh;
    MeshReader reader(&mesh);
    reader.setMaterial(1, material);  // Tag 1 = matériau unique
    
    try {
        reader.readGmshFile(meshFile);
        mesh.initializeElements();
        cout << "Maillage: " << mesh.getNbNodes() << " noeuds, " << mesh.getNbElements() << " éléments" << endl;
        cout << endl;
    } catch (const exception& e) {
        cerr << "Erreur lors de la lecture du maillage : " << e.what() << endl;
        delete material;
        return;
    }
    
    if (mesh.getNbElements() == 0) {
        cerr << "Erreur : aucun élément chargé!" << endl;
        delete material;
        return;
    }

    Solver solver(mesh);
    
    // Assembler la matrice de rigidité
    solver.assemble();
    cout << endl;
    
    // Configuration des conditions aux limites pour traction simple
    double xMax = 0.0;
    double yMax = 0.0;
    for (const auto& node : mesh.getNodes()) {
        Vector2d coords = node.getCoords();
        xMax = max(xMax, coords.x());
        yMax = max(yMax, coords.y());
    }
    
    vector<int> leftNodes, rightNodes, bottomNodes, topNodes;
    for (const auto& node : mesh.getNodes()) {
        Vector2d coords = node.getCoords();
        int nodeId = node.getId();
    
        if (abs(coords.x()) < 1e-6) {
            leftNodes.push_back(nodeId);
        }
        if (abs(coords.x() - xMax) < 1e-6) {
            rightNodes.push_back(nodeId);
        }
        if (abs(coords.y()) < 1e-6) {
            bottomNodes.push_back(nodeId);
        }
        if (abs(coords.y() - yMax) < 1e-6) {
            topNodes.push_back(nodeId);
        }
    }
    
    double totalForce = 1000.0; // N
    double forcePerNodeLeft = -totalForce / leftNodes.size();
    double forcePerNodeRight = totalForce / rightNodes.size();
    
    for (int nodeId : leftNodes) {
        solver.setNeumannBC(nodeId, 0, forcePerNodeLeft); // Force en x
    }
    for (int nodeId : rightNodes) {
        solver.setNeumannBC(nodeId, 0, forcePerNodeRight); // Force en x
    }
    
    // Appliquer les conditions aux limites
    solver.applyBC();
    cout << endl;
    
    // Résoudre le système
    solver.solveConjugateGradient();
    cout << endl;
    
    // Sauvegarder les résultats
    solver.saveResults("../results/displacement_traction.txt");
    solver.saveVTK("../results/results_traction.vtk");
    
    // Calcul des allongements réels
    Eigen::VectorXd U = solver.getU();
    double sumUxLeft = 0.0, sumUxRight = 0.0;
    double sumUyBottom = 0.0, sumUyTop = 0.0;
    
    for (int nodeId : leftNodes) {
        int dof_x = 2 * (nodeId - 1);
        sumUxLeft += U(dof_x);
    }
    for (int nodeId : rightNodes) {
        int dof_x = 2 * (nodeId - 1);
        sumUxRight += U(dof_x);
    }
    for (int nodeId : bottomNodes) {
        int dof_y = 2 * (nodeId - 1) + 1;
        sumUyBottom += U(dof_y);
    }
    for (int nodeId : topNodes) {
        int dof_y = 2 * (nodeId - 1) + 1;
        sumUyTop += U(dof_y);
    }
    
    double avgUxLeft = sumUxLeft / leftNodes.size();
    double avgUxRight = sumUxRight / rightNodes.size();
    double avgUyBottom = sumUyBottom / bottomNodes.size();
    double avgUyTop = sumUyTop / topNodes.size();
    
    double deltaL_horizontal = avgUxRight - avgUxLeft;
    double deltaL_vertical = avgUyTop - avgUyBottom;
    
    // Calcul théorique de vérification
    // Pour traction simple: epsilon = sigma/E = F/(A*E)
    // Allongement: delta_L = epsilon * L = (F * L) / (A * E)
    double F = 1000.0;  // Force totale en N
    double L = xMax;    // Longueur réelle en m
    double H = yMax;    // Hauteur réelle en m
    double A = H * 1.0; // Section = H * épaisseur (épaisseur = 1m en 2D)
    double E = 200.0e9; // Module de Young en Pa
    
    double epsilon_theo = F / (A * E);
    double delta_L_theo_horizontal = epsilon_theo * L;
    double delta_L_theo_vertical = -epsilon_theo * H * 0.3; // Poisson = 0.3
    
    cout << "Allongement horizontal: " << deltaL_horizontal << " m (théorique: " << delta_L_theo_horizontal << " m)" << endl;
    cout << "Allongement vertical: " << deltaL_vertical << " m (théorique: " << delta_L_theo_vertical << " m)" << endl;
    
    cout << "Simulation terminée" << endl;
    
    // Nettoyage
    delete material;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <mesh_file.msh>" << endl;
        return 1;
    }
    
    string meshFile = argv[1];
    runTractionTest(meshFile);
    
    return 0;
}
