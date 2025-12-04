#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include <Eigen/Dense>

struct Node {
    int id;
    Eigen::Vector2d coords;

    Node(int nodeId, const Eigen::Vector2d& c) : id(nodeId), coords(c) {}
};

struct Element {
    int id;
    std::vector<int> nodeIds; 
    class Material* mat;
    double area;
    Eigen::Matrix<double, 6, 6> Ke;

    Element(int elemId, const std::vector<int>& nIds, Material* m) : id(elemId), nodeIds(nIds), mat(m), area(0.0) {
        Ke.setZero();
    }

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
