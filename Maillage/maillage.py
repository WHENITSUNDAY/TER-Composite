#!/usr/bin/env python3
import sys

fichier_cercles = "cercles.txt"  # Fichier contenant x y r (un cercle par ligne)
nom_fichier_geo = "maillage_fibres.geo"
taille_maillage = 0.1  # Taille caractéristique

# Lecture du fichier des cercles
cercles = []
with open(fichier_cercles, "r") as f:
    for ligne in f:
        if ligne.strip() == "":
            continue
        x, y, r = map(float, ligne.strip().split())
        cercles.append((x, y, r))

if not cercles:
    sys.exit("Erreur : aucun cercle trouvé dans le fichier cercles.txt")

# Calcul des limites pour le rectangle englobant
xmin = min(x - r for x, y, r in cercles) - 1.0
xmax = max(x + r for x, y, r in cercles) + 1.0
ymin = min(y - r for x, y, r in cercles) - 1.0
ymax = max(y + r for x, y, r in cercles) + 1.0

# Écriture du fichier .geo
with open(nom_fichier_geo, "w") as f:
    f.write(f"// Fichier généré automatiquement à partir de {fichier_cercles}\n")
    f.write(f"lc = {taille_maillage};\n\n")

    # --- Cercles (fibres)
    fibre_loops = []
    fibre_surfaces = []
    fibre_boundaries = []  # Pour stocker les courbes de bord de chaque fibre
    
    for i, (x, y, r) in enumerate(cercles, start=1):
        base = i * 10
        f.write(f"// Fibre {i}\n")
        f.write(f"Point({base+1}) = {{{x + r}, {y}, 0, lc}};\n")
        f.write(f"Point({base+2}) = {{{x}, {y + r}, 0, lc}};\n")
        f.write(f"Point({base+3}) = {{{x - r}, {y}, 0, lc}};\n")
        f.write(f"Point({base+4}) = {{{x}, {y - r}, 0, lc}};\n")
        f.write(f"Point({base+5}) = {{{x}, {y}, 0, lc}};\n")

        f.write(f"Circle({base+6}) = {{{base+1}, {base+5}, {base+2}}};\n")
        f.write(f"Circle({base+7}) = {{{base+2}, {base+5}, {base+3}}};\n")
        f.write(f"Circle({base+8}) = {{{base+3}, {base+5}, {base+4}}};\n")
        f.write(f"Circle({base+9}) = {{{base+4}, {base+5}, {base+1}}};\n")

        # Stocker les courbes de bord de cette fibre
        fibre_boundaries.extend([base+6, base+7, base+8, base+9])
        
        f.write(f"Curve Loop({i}) = {{{base+6}, {base+7}, {base+8}, {base+9}}};\n")
        fibre_loops.append(i)

        # Chaque fibre est une surface distincte
        surface_id = 1000 + i
        f.write(f"Plane Surface({surface_id}) = {{{i}}};\n")
        fibre_surfaces.append(surface_id)
        f.write("\n")

    # --- Rectangle extérieur (matrice)
    f.write("// Domaine extérieur (matrice)\n")
    f.write(f"Point(1) = {{{xmin}, {ymin}, 0, lc}};\n")
    f.write(f"Point(2) = {{{xmax}, {ymin}, 0, lc}};\n")
    f.write(f"Point(3) = {{{xmax}, {ymax}, 0, lc}};\n")
    f.write(f"Point(4) = {{{xmin}, {ymax}, 0, lc}};\n")

    f.write("Line(1) = {1, 2};\n")
    f.write("Line(2) = {2, 3};\n")
    f.write("Line(3) = {3, 4};\n")
    f.write("Line(4) = {4, 1};\n")
    f.write("Curve Loop(999) = {1, 2, 3, 4};\n\n")

    # Soustraction : la matrice = grand rectangle - fibres
    fibre_list = ", ".join(str(l) for l in fibre_loops)
    f.write(f"Plane Surface(2000) = {{999, {fibre_list}}};\n\n")

    # --- Physical Tags ---
    f.write("// Physical Tags pour identification dans le code\n\n")
    
    # Tag 2 : Surfaces des fibres (mailles fibres)
    fibre_surf_list = ", ".join(str(s) for s in fibre_surfaces)
    f.write(f"Physical Surface(2) = {{{fibre_surf_list}}};\n")
    
    # Tag 1 : Surface de la matrice (mailles matrices)
    f.write("Physical Surface(1) = {2000};\n\n")
    
    # Tag 11 : Arêtes fibre-matrice (interfaces)
    fibre_bound_list = ", ".join(str(b) for b in fibre_boundaries)
    f.write(f"Physical Curve(11) = {{{fibre_bound_list}}};\n")
    
    # Tag 12 : Arêtes du bord extérieur
    f.write("Physical Curve(12) = {1, 2, 3, 4};\n")

print(f"Fichier '{nom_fichier_geo}' généré avec succès.")
print("Convention des tags physiques :")
print("  - Surface tag 1 : Matrice")
print("  - Surface tag 2 : Fibres")
print("  - Curve tag 11 : Interfaces fibre-matrice")
print("  - Curve tag 12 : Bord extérieur du domaine")
print("\nOuvre-le dans Gmsh, puis lance le maillage 2D.")