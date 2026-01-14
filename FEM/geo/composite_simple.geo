// Composite simple: Carré avec fibre circulaire au centre
// Matrice + fibre isotrope

// Dimensions
L = 1.0;        // Côté du carré
R = 0.15;       // Rayon de la fibre
lc_matrix = 0.05;   // Taille de maille matrice
lc_fiber = 0.02;    // Taille de maille fibre (plus fin)

// Points du carré
Point(1) = {0, 0, 0, lc_matrix};
Point(2) = {L, 0, 0, lc_matrix};
Point(3) = {L, L, 0, lc_matrix};
Point(4) = {0, L, 0, lc_matrix};

// Centre de la fibre
cx = L/2;
cy = L/2;

// Points de la fibre (cercle au centre)
Point(5) = {cx, cy, 0, lc_fiber};          // Centre
Point(6) = {cx+R, cy, 0, lc_fiber};        // Droite
Point(7) = {cx, cy+R, 0, lc_fiber};        // Haut
Point(8) = {cx-R, cy, 0, lc_fiber};        // Gauche
Point(9) = {cx, cy-R, 0, lc_fiber};        // Bas

// Lignes du carré
Line(1) = {1, 2};  // Bas
Line(2) = {2, 3};  // Droite
Line(3) = {3, 4};  // Haut
Line(4) = {4, 1};  // Gauche

// Cercle de la fibre (4 arcs)
Circle(5) = {6, 5, 7};  // Droite -> Haut
Circle(6) = {7, 5, 8};  // Haut -> Gauche
Circle(7) = {8, 5, 9};  // Gauche -> Bas
Circle(8) = {9, 5, 6};  // Bas -> Droite

// Surface du carré (matrice)
Line Loop(1) = {1, 2, 3, 4};      // Contour carré
Line Loop(2) = {5, 6, 7, 8};      // Contour fibre
Plane Surface(1) = {1, 2};        // Matrice = carré - fibre

// Surface de la fibre
Plane Surface(2) = {2};           // Fibre

// Tags physiques pour les bords
Physical Line("left", 10) = {4};     // Bord gauche
Physical Line("right", 11) = {2};    // Bord droit
Physical Line("bottom", 12) = {1};   // Bord bas
Physical Line("top", 13) = {3};      // Bord haut

// Tags physiques pour les matériaux
Physical Surface("matrix", 1) = {1};  // Matrice (matériau 1)
Physical Surface("fiber", 2) = {2};   // Fibre (matériau 2)

// Générer le maillage
Mesh 2;
