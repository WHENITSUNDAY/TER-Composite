#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include <map>
#include <set>
#include <fstream>
#include <Eigen/Dense>

// Structure pour stocker les informations d'une arête
struct Edge {
    int node1, node2;
    int tag;  // 11 pour fibre-matrice, 12 pour bord
    
    Edge(int n1, int n2, int t);
    
    bool operator<(const Edge& other) const;
};

class Node {
    // Classe représentant un noeud du maillage (coordonnées et numéro)
    private:
        int _id;
        Eigen::Vector2d _coords;

    public:
        Node(int nodeId, const Eigen::Vector2d& coords);
        int getId() const { return _id; }
        Eigen::Vector2d getCoords() const { return _coords; }
        void setCoords(const Eigen::Vector2d& c) { _coords = c; }
};

class Element {
    // Classe représentant un élément (triangle P1) du maillage avec ses noeuds et le matériau associé.
    private:
        int _id;
        // Les noeuds sont référencés par leurs indices, ce qui évite de dupliquer des objets Node inutilement,
        // Sachant que les elements partagent des noeuds entre eux.
        std::vector<int> _nodeIds; 
        
        class Material* _mat;
        double _area;
        Eigen::Matrix<double, 6, 6> _Ke; // Matrice de rigidité élémentaire propre à l'élément P1     

    public:
        Element(int elemId, const std::vector<int>& nodeIds, Material* mat);    
        
        int getId() const { return _id; }
        double getArea() const { return _area; }
        Material* getMaterial() const { return _mat; }
        const std::vector<int>& getNodeIds() const { return _nodeIds; }

        void setArea(double A) { _area = A; }
        void computeKe();
};

class Mesh {
    // Classe globale représentant le maillage en utilisant des listes d'objets Node et Element.
    // Permet de charger un maillage depuis un fichier Gmsh et de le manipuler dans le programme.
    // Intègre également la lecture du fichier et la gestion des arêtes (anciennement dans MeshReader).
    private:
        std::vector<Node> _nodes; 
        std::vector<Element> _elements;
        std::map<int, Material*> _materialMap;  // tag -> Material
        std::set<Edge> _edges;

        // Méthodes privées pour la lecture du fichier Gmsh
        void readNodes(std::ifstream& file);
        void readElements(std::ifstream& file);
        void printStatistics() const;

    public:
        Mesh();

        void addNode(const Node& node);
        void addElement(const Element& element);

        const std::vector<Node>& getNodes() const { return _nodes; }
        const std::vector<Element>& getElements() const { return _elements; }  

        Node& getNode(int id);
        Element& getElement(int id);

        int getNbNodes() const { return _nodes.size(); }
        int getNbElements() const { return _elements.size(); }
        
        // Associer un matériau à un tag physique (doit être appelé avant loadFromGmsh)
        void setMaterial(int tag, Material* mat);
        
        // Charger le maillage depuis un fichier Gmsh
        void loadFromGmsh(const std::string& filename);
        
        // Accéder aux arêtes
        const std::set<Edge>& getEdges() const;
        std::vector<Edge> getEdgesByTag(int tag) const;
};

#endif
