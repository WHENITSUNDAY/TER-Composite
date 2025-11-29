#include "Mesh.h"
#include <iostream>
#include <sstream>
#include <algorithm>

// ==================== Edge ====================

Edge::Edge(int n1, int n2, int t) 
    : node1(std::min(n1, n2)), node2(std::max(n1, n2)), tag(t) {}

bool Edge::operator<(const Edge& other) const {
    if (node1 != other.node1) return node1 < other.node1;
    return node2 < other.node2;
}

// ==================== Node ====================

Node::Node(int nodeId, const Eigen::Vector2d& coords) 
    : _id(nodeId), _coords(coords) {}

// ==================== Element ====================

Element::Element(int elemId, const std::vector<int>& nodeIds, Material* mat)
    : _id(elemId), _nodeIds(nodeIds), _mat(mat), _area(0.0) {}

// ==================== Mesh ====================

Mesh::Mesh() {}

void Mesh::addNode(const Node& node) {
    _nodes.push_back(node);
}

void Mesh::addElement(const Element& element) {
    _elements.push_back(element);
}

Node& Mesh::getNode(int id) {
    for (auto& node : _nodes) {
        if (node.getId() == id) {
            return node;
        }
    }
    throw std::runtime_error("Node not found: " + std::to_string(id));
}

Element& Mesh::getElement(int id) {
    for (auto& elem : _elements) {
        if (elem.getId() == id) {
            return elem;
        }
    }
    throw std::runtime_error("Element not found: " + std::to_string(id));
}

void Mesh::setMaterial(int tag, Material* mat) {
    _materialMap[tag] = mat;
}

void Mesh::loadFromGmsh(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erreur : impossible d'ouvrir " << filename << std::endl;
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("$Nodes") != std::string::npos) {
            readNodes(file);
        }
        else if (line.find("$Elements") != std::string::npos) {
            readElements(file);
        }
    }
    
    file.close();
    printStatistics();
}

void Mesh::readNodes(std::ifstream& file) {
    std::string line;
    std::getline(file, line);
    
    int numNodes;
    std::istringstream iss(line);
    
    // Détecter le format Gmsh
    if (line.find_first_of("0123456789") == 0) {
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token) {
            tokens.push_back(token);
        }
        
        if (tokens.size() == 4) {
            // Format Gmsh 4.x : numEntityBlocks numNodes minNodeTag maxNodeTag
            int numEntityBlocks = std::stoi(tokens[0]);
            numNodes = std::stoi(tokens[1]);
            
            for (int i = 0; i < numEntityBlocks; i++) {
                std::getline(file, line);
                std::istringstream blockHeader(line);
                int entityDim, entityTag, parametric, numNodesInBlock;
                blockHeader >> entityDim >> entityTag >> parametric >> numNodesInBlock;
                
                // Lire les IDs des noeuds
                std::vector<int> nodeIds;
                for (int j = 0; j < numNodesInBlock; j++) {
                    std::getline(file, line);
                    nodeIds.push_back(std::stoi(line));
                }
                
                // Lire les coordonnées
                for (int j = 0; j < numNodesInBlock; j++) {
                    std::getline(file, line);
                    std::istringstream coords(line);
                    double x, y, z;
                    coords >> x >> y >> z;
                    
                    Eigen::Vector2d pos(x, y);
                    addNode(Node(nodeIds[j], pos));
                }
            }
        }
        else {
            // Format Gmsh 2.2 : simple numNodes
            numNodes = std::stoi(tokens[0]);
            for (int i = 0; i < numNodes; i++) {
                std::getline(file, line);
                std::istringstream nodeStream(line);
                int id;
                double x, y, z;
                nodeStream >> id >> x >> y >> z;
                
                Eigen::Vector2d pos(x, y);
                addNode(Node(id, pos));
            }
        }
    }
    
    std::getline(file, line);  // $EndNodes
    std::cout << "Noeuds lus : " << getNbNodes() << std::endl;
}

void Mesh::readElements(std::ifstream& file) {
    std::string line;
    std::getline(file, line);
    
    int numElements;
    std::istringstream iss(line);
    
    int elemIdCounter = 1;
    int numTrianglesMatrix = 0;
    int numTrianglesFiber = 0;
    int numEdgesFiberMatrix = 0;
    int numEdgesBoundary = 0;
    
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    
    if (tokens.size() == 4) {
        // Format Gmsh 4.x : numEntityBlocks numElements minTag maxTag
        int numEntityBlocks = std::stoi(tokens[0]);
        numElements = std::stoi(tokens[1]);
        
        for (int i = 0; i < numEntityBlocks; i++) {
            std::getline(file, line);
            std::istringstream blockHeader(line);
            int entityDim, entityTag, elementType, numElementsInBlock;
            blockHeader >> entityDim >> entityTag >> elementType >> numElementsInBlock;
            
            for (int j = 0; j < numElementsInBlock; j++) {
                std::getline(file, line);
                std::istringstream elemStream(line);
                int elemId;
                elemStream >> elemId;
                
                if (elementType == 2) {  // Triangle
                    int n1, n2, n3;
                    elemStream >> n1 >> n2 >> n3;
                    
                    std::vector<int> nodeIds = {n1, n2, n3};
                    Material* mat = _materialMap[entityTag];
                    
                    if (mat == nullptr) {
                        std::cerr << "Attention : matériau non défini pour le tag " 
                                  << entityTag << std::endl;
                    }
                    
                    addElement(Element(elemIdCounter++, nodeIds, mat));
                    
                    if (entityTag == 1) numTrianglesMatrix++;
                    else if (entityTag == 2) numTrianglesFiber++;
                }
                else if (elementType == 1) {  // Segment (arête)
                    int n1, n2;
                    elemStream >> n1 >> n2;
                    
                    _edges.insert(Edge(n1, n2, entityTag));
                    
                    if (entityTag == 11) numEdgesFiberMatrix++;
                    else if (entityTag == 12) numEdgesBoundary++;
                }
            }
        }
    }
    else {
        // Format Gmsh 2.2
        numElements = std::stoi(tokens[0]);
        
        for (int i = 0; i < numElements; i++) {
            std::getline(file, line);
            std::istringstream elemStream(line);
            
            int elemId, elemType, numTags;
            elemStream >> elemId >> elemType >> numTags;
            
            std::vector<int> tags;
            for (int t = 0; t < numTags; t++) {
                int tag;
                elemStream >> tag;
                tags.push_back(tag);
            }
            
            int physicalTag = (numTags > 0) ? tags[0] : 0;
            
            if (elemType == 2) {  // Triangle
                int n1, n2, n3;
                elemStream >> n1 >> n2 >> n3;
                
                std::vector<int> nodeIds = {n1, n2, n3};
                Material* mat = _materialMap[physicalTag];
                
                if (mat == nullptr) {
                    std::cerr << "Attention : matériau non défini pour le tag " 
                              << physicalTag << std::endl;
                }
                
                addElement(Element(elemIdCounter++, nodeIds, mat));
                
                if (physicalTag == 1) numTrianglesMatrix++;
                else if (physicalTag == 2) numTrianglesFiber++;
            }
            else if (elemType == 1) {  // Segment (arête)
                int n1, n2;
                elemStream >> n1 >> n2;
                
                _edges.insert(Edge(n1, n2, physicalTag));
                
                if (physicalTag == 11) numEdgesFiberMatrix++;
                else if (physicalTag == 12) numEdgesBoundary++;
            }
        }
    }
    
    std::getline(file, line);  // $EndElements
    
    std::cout << "\nÉléments lus :" << std::endl;
    std::cout << "  - Triangles matrice (tag 1) : " << numTrianglesMatrix << std::endl;
    std::cout << "  - Triangles fibres (tag 2) : " << numTrianglesFiber << std::endl;
    std::cout << "  - Arêtes fibre-matrice (tag 11) : " << numEdgesFiberMatrix << std::endl;
    std::cout << "  - Arêtes bord (tag 12) : " << numEdgesBoundary << std::endl;
    std::cout << "  - Total triangles : " << getNbElements() << std::endl;
}

void Mesh::printStatistics() const {
    std::cout << "\n=== Statistiques du maillage ===" << std::endl;
    std::cout << "Nombre de noeuds : " << _nodes.size() << std::endl;
    std::cout << "Nombre d'éléments : " << _elements.size() << std::endl;
    std::cout << "Nombre d'arêtes : " << _edges.size() << std::endl;
}

const std::set<Edge>& Mesh::getEdges() const {
    return _edges;
}

std::vector<Edge> Mesh::getEdgesByTag(int tag) const {
    std::vector<Edge> result;
    for (const auto& edge : _edges) {
        if (edge.tag == tag) {
            result.push_back(edge);
        }
    }
    return result;
}
