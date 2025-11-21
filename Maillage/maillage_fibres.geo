// Fichier généré automatiquement à partir de cercles.txt
lc = 0.1;

// Fibre 1
Point(11) = {1.0, 0.0, 0, lc};
Point(12) = {0.0, 1.0, 0, lc};
Point(13) = {-1.0, 0.0, 0, lc};
Point(14) = {0.0, -1.0, 0, lc};
Point(15) = {0.0, 0.0, 0, lc};
Circle(16) = {11, 15, 12};
Circle(17) = {12, 15, 13};
Circle(18) = {13, 15, 14};
Circle(19) = {14, 15, 11};
Curve Loop(1) = {16, 17, 18, 19};
Plane Surface(1) = {1};
Physical Surface("Fiber_1") = {1};

// Fibre 2
Point(21) = {2.5, 2.0, 0, lc};
Point(22) = {2.0, 2.5, 0, lc};
Point(23) = {1.5, 2.0, 0, lc};
Point(24) = {2.0, 1.5, 0, lc};
Point(25) = {2.0, 2.0, 0, lc};
Circle(26) = {21, 25, 22};
Circle(27) = {22, 25, 23};
Circle(28) = {23, 25, 24};
Circle(29) = {24, 25, 21};
Curve Loop(2) = {26, 27, 28, 29};
Plane Surface(2) = {2};
Physical Surface("Fiber_2") = {2};

// Fibre 3
Point(31) = {-0.7, 1.0, 0, lc};
Point(32) = {-1.0, 1.3, 0, lc};
Point(33) = {-1.3, 1.0, 0, lc};
Point(34) = {-1.0, 0.7, 0, lc};
Point(35) = {-1.0, 1.0, 0, lc};
Circle(36) = {31, 35, 32};
Circle(37) = {32, 35, 33};
Circle(38) = {33, 35, 34};
Circle(39) = {34, 35, 31};
Curve Loop(3) = {36, 37, 38, 39};
Plane Surface(3) = {3};
Physical Surface("Fiber_3") = {3};

// Domaine extérieur (matrice)
Point(1) = {-2.3, -2.0, 0, lc};
Point(2) = {3.5, -2.0, 0, lc};
Point(3) = {3.5, 3.5, 0, lc};
Point(4) = {-2.3, 3.5, 0, lc};
Line(1) = {1, 2};
Line(2) = {2, 3};
Line(3) = {3, 4};
Line(4) = {4, 1};
Curve Loop(999) = {1, 2, 3, 4};
Plane Surface(1000) = {999, 1, 2, 3};
Physical Surface("Matrix") = {1000};
