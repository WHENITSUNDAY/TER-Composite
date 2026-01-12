#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include <Eigen/Dense>

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
        const Eigen::Matrix<double, 6, 6>& getKe() const { return _Ke; }

        void computeArea(const Node& n1, const Node& n2, const Node& n3);
        void computeKe(const Node& n1, const Node& n2, const Node& n3);
};

class Mesh {
    // Classe globale représentant le maillage en utilisant des listes d'objets Node et Element.
    // Permet de charger un maillage depuis un fichier Gmsh et de le manipuler dans le programme.
    private:
        std::vector<Node> _nodes; 
        std::vector<Element> _elements;

    public:
        Mesh();

        void addNode(const Node& node);
        void addElement(const Element& element);

        const std::vector<Node>& getNodes() const { return _nodes; }
        const std::vector<Element>& getElements() const { return _elements; }  

        Node& getNode(int id);
        Element& getElement(int id);
        std::vector<Element>& getElementsRef() { return _elements; }

        int getNbNodes() const { return _nodes.size(); }
        int getNbElements() const { return _elements.size(); }
        
        void loadFromGmsh(const std::string& filename);
        void initializeElements();
};

#endif
