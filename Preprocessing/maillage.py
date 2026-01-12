#!/usr/bin/env python3
import sys
import cv2

fichier_cercles = "cercles.txt"  # Fichier contenant x y r (un cercle par ligne)
nom_fichier_geo = "maillage_fibres.geo"
taille_maillage = 5  # Taille caractéristique beaucoup plus grossière

# Obtenir la hauteur de l'image pour inverser l'axe y
img_path = "images_test/ech2_x50_0.07um_x50_00.png"
img = cv2.imread(img_path)
if img is None:
    sys.exit("Erreur : impossible de charger l'image pour obtenir les dimensions")
height = img.shape[0]

# Lecture du fichier des cercles
cercles = []
with open(fichier_cercles, "r") as f:
    for ligne in f:
        if ligne.strip() == "":
            continue
        x, y, r = map(float, ligne.strip().split())
        # Inversion de l'axe y (dans les images, y=0 est en haut)
        y = height - y
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

# Génération automatique du maillage avec Gmsh
import subprocess
import os

nom_fichier_msh = nom_fichier_geo.replace('.geo', '.msh')

print(f"\nGénération du maillage avec Gmsh : {nom_fichier_geo} -> {nom_fichier_msh}")
try:
    # Exécuter Gmsh en mode batch pour générer le maillage
    result = subprocess.run(['gmsh', nom_fichier_geo, '-2', '-o', nom_fichier_msh], 
                          capture_output=True, text=True, check=True)
    print(f"Maillage généré avec succès : {nom_fichier_msh}")
except subprocess.CalledProcessError as e:
    print(f"Erreur lors de la génération du maillage : {e}")
    print(f"Stdout: {e.stdout}")
    print(f"Stderr: {e.stderr}")
    sys.exit(1)
except FileNotFoundError:
    print("Erreur : Gmsh n'est pas installé ou n'est pas dans le PATH")
    print("Installez Gmsh (sudo apt install gmsh) ou exécutez manuellement :")
    print(f"gmsh {nom_fichier_geo} -2")
    sys.exit(1)