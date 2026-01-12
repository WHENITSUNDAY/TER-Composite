#include "Mesh.h"
#include "MeshReader.h"
#include "Material.h"
#include <iostream>
#include <cmath>

using namespace std;
using namespace Eigen;


Node::Node(int nodeId, const Vector2d& coords) 
    : _id(nodeId), _coords(coords) {}


Element::Element(int elemId, const vector<int>& nodeIds, Material* mat)
    : _id(elemId), _nodeIds(nodeIds), _mat(mat), _area(0.0) {
    _Ke.setZero();
}

void Element::computeArea(const Node& n1, const Node& n2, const Node& n3) {
    Vector2d p1 = n1.getCoords();
    Vector2d p2 = n2.getCoords();
    Vector2d p3 = n3.getCoords();
    
    _area = 0.5 * abs((p2.x() - p1.x()) * (p3.y() - p1.y()) - 
                      (p3.x() - p1.x()) * (p2.y() - p1.y()));
}

void Element::computeKe(const Node& n1, const Node& n2, const Node& n3) {
    Vector2d p1 = n1.getCoords();
    Vector2d p2 = n2.getCoords();
    Vector2d p3 = n3.getCoords();
    
    if (_area < 1e-12) {
        cerr << "Attention : élément " << _id << " dégénéré (aire ~ 0)" << endl;
        _Ke.setZero();
        return;
    }
    
    // Matrice B pour élément triangulaire P1
    Matrix<double, 3, 6> B;
    B.setZero();
    
    double b1 = p2.y() - p3.y();
    double b2 = p3.y() - p1.y();
    double b3 = p1.y() - p2.y();
    
    double c1 = p3.x() - p2.x();
    double c2 = p1.x() - p3.x();
    double c3 = p2.x() - p1.x();
    
    B(0, 0) = b1;  B(0, 2) = b2;  B(0, 4) = b3;
    B(1, 1) = c1;  B(1, 3) = c2;  B(1, 5) = c3;
    B(2, 0) = c1;  B(2, 2) = c2;  B(2, 4) = c3;
    B(2, 1) = b1;  B(2, 3) = b2;  B(2, 5) = b3;
    
    B /= (2.0 * _area);
    
    // Matrice de rigidité du matériau
    Matrix3d C = _mat->getC();
    
    // Matrice de rigidité élémentaire
    _Ke = _area * B.transpose() * C * B;
}

Mesh::Mesh() {}

void Mesh::addNode(const Node& node) {
    _nodes.push_back(node);
}

void Mesh::addElement(const Element& element) {
    _elements.push_back(element);
}

Node& Mesh::getNode(int id) {
    // Les noeuds sont stockés avec des IDs qui commencent à 1
    // Mais l'index dans le vecteur commence à 0
    for (auto& node : _nodes) {
        if (node.getId() == id) {
            return node;
        }
    }
    cerr << "Erreur : noeud " << id << " introuvable!" << endl;
    return _nodes[0];
}

Element& Mesh::getElement(int id) {
    for (auto& elem : _elements) {
        if (elem.getId() == id) {
            return elem;
        }
    }
    cerr << "Erreur : élément " << id << " introuvable!" << endl;
    return _elements[0];
}

void Mesh::loadFromGmsh(const string& filename) {
    MeshReader reader(this);
    reader.readGmshFile(filename);
    
    // Initialiser les éléments (calcul de l'aire et Ke)
    initializeElements();
}

void Mesh::initializeElements() {
    int countValid = 0;
    double totalArea = 0.0;
    
    for (auto& elem : _elements) {
        const vector<int>& nodeIds = elem.getNodeIds();
        
        Node& n1 = getNode(nodeIds[0]);
        Node& n2 = getNode(nodeIds[1]);
        Node& n3 = getNode(nodeIds[2]);
        
        // Calculer l'aire
        elem.computeArea(n1, n2, n3);
        
        if (elem.getArea() > 1e-12) {
            // Calculer la matrice de rigidité élémentaire
            elem.computeKe(n1, n2, n3);
            countValid++;
            totalArea += elem.getArea();
        }
    }
}