#!/usr/bin/env python3
"""
Script de prétraitement pour simulation FEM de matériau composite.
Détecte les fibres depuis une image et génère le fichier de géométrie Gmsh.
"""

import cv2 as cv
import numpy as np
import sys
import argparse
import re

def detect_circles(img_path, pixel_size=None):
    """
    Détecte les cercles dans l'image en utilisant la transformée de Hough.
    Retourne une liste de (x, y, r) en unités physiques.
    """
    img = cv.imread(img_path)
    if img is None:
        raise ValueError(f"Impossible de charger l'image : {img_path}")

    # Prétraitement
    L = max(img.shape)
    w, h = img.shape[1], img.shape[0]
    img_resized = cv.resize(img, (int((w/L)*500), int((h/L)*500)))
    gray = cv.imread(img_path, cv.IMREAD_GRAYSCALE)
    gray_resized = cv.resize(gray, (int((w/L)*500), int((h/L)*500)))
    blur = cv.GaussianBlur(gray_resized, (9, 9), 2)

    # Détection des cercles
    circles = cv.HoughCircles(blur, method=cv.HOUGH_GRADIENT, dp=1, minDist=78,
                              param1=10, param2=7, minRadius=34, maxRadius=40)

    if circles is None:
        return []

    circles = np.uint16(np.around(circles))

    # Normalisation et mise à l'échelle en unités physiques
    scale_factor = pixel_size if pixel_size else L  # Si pas de pixel_size, utiliser L pour normalisation
    detected_circles = []
    for i in circles[0, :]:
        x = i[0] / L * scale_factor
        y = i[1] / L * scale_factor
        r = i[2] / L * scale_factor
        detected_circles.append((x, y, r))

    return detected_circles

def generate_geo(circles, output_file, mesh_size=0.1):
    """
    Génère le fichier .geo Gmsh à partir des cercles.
    """
    if not circles:
        print("Aucun cercle détecté, génération du geo ignorée.")
        return

    # Calcul des limites du domaine
    xmin = min(x - r for x, y, r in circles) - 1.0
    xmax = max(x + r for x, y, r in circles) + 1.0
    ymin = min(y - r for x, y, r in circles) - 1.0
    ymax = max(y + r for x, y, r in circles) + 1.0

    with open(output_file, 'w') as f:
        f.write("// Généré automatiquement depuis le traitement d'image\n")
        f.write(f"lc = {mesh_size};\n\n")

        # Fibres
        fibre_loops = []
        fibre_surfaces = []
        fibre_boundaries = []

        for i, (x, y, r) in enumerate(circles, start=1):
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

            fibre_boundaries.extend([base+6, base+7, base+8, base+9])
            f.write(f"Curve Loop({i}) = {{{base+6}, {base+7}, {base+8}, {base+9}}};\n")
            fibre_loops.append(i)

            surface_id = 1000 + i
            f.write(f"Plane Surface({surface_id}) = {{{i}}};\n")
            fibre_surfaces.append(surface_id)
            f.write("\n")

        # Rectangle extérieur (matrice)
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

        # Matrice = rectangle - fibres
        fibre_list = ", ".join(str(l) for l in fibre_loops)
        f.write(f"Plane Surface(2000) = {{999, {fibre_list}}};\n\n")

        # Tags physiques
        f.write("// Tags physiques\n\n")
        fibre_surf_list = ", ".join(str(s) for s in fibre_surfaces)
        f.write(f"Physical Surface(2) = {{{fibre_surf_list}}};\n")  # Fibres
        f.write("Physical Surface(1) = {2000};\n\n")  # Matrice

        fibre_bound_list = ", ".join(str(b) for b in fibre_boundaries)
        f.write(f"Physical Curve(11) = {{{fibre_bound_list}}};\n")  # Interfaces
        f.write("Physical Curve(12) = {1, 2, 3, 4};\n")  # Bord extérieur

        # Paramètres de maillage
        f.write("\n// Paramètres de maillage\n")
        f.write("Mesh.Algorithm = 5;\n")
        f.write("Mesh 2;\n")

    print(f"Fichier '{output_file}' généré avec succès.")
    print("Convention des tags physiques :")
    print("  - Surface 1 : Matrice")
    print("  - Surface 2 : Fibres")
    print("  - Curve 11 : Interfaces fibre-matrice")
    print("  - Curve 12 : Bord extérieur du domaine")

def extract_pixel_size(filename):
    """
    Extrait la taille de pixel depuis le nom du fichier, ex: 'image_0.001m.png' -> 0.001, ou 'image_0.03um.png' -> 0.00003
    """
    match = re.search(r'_(\d+\.?\d*)(um|m)', filename)
    if match:
        value = float(match.group(1))
        unit = match.group(2)
        if unit == 'um':
            value *= 1e-6  # convertir um en m
        return value
    return None

def main():
    parser = argparse.ArgumentParser(description="Prétraitement pour FEM composite : détecter les fibres et générer la géométrie Gmsh.")
    parser.add_argument('image', help='Chemin vers l\'image d\'entrée')
    parser.add_argument('-o', '--output', default='geo/maillage_fibres.geo', help='Fichier .geo de sortie')
    parser.add_argument('-m', '--mesh-size', type=float, default=0.1, help='Taille caractéristique du maillage')
    parser.add_argument('-p', '--pixel-size', type=float, help='Taille de pixel en mètres (remplace l\'extraction automatique)')
    parser.add_argument('--show', action='store_true', help='Afficher les cercles détectés')

    args = parser.parse_args()

    # Extraire la taille de pixel si non fournie
    pixel_size = args.pixel_size
    if pixel_size is None:
        pixel_size = extract_pixel_size(args.image)
        if pixel_size:
            print(f"Taille de pixel extraite : {pixel_size} m")

    # Détecter les cercles
    circles = detect_circles(args.image, pixel_size)
    print(f"{len(circles)} cercles détectés")

    # Générer le geo
    generate_geo(circles, args.output, args.mesh_size)

    # Affichage optionnel
    if args.show:
        img = cv.imread(args.image)
        if img is not None:
            for x, y, r in circles:
                # Remettre à l'échelle pour l'affichage
                scale = max(img.shape) / (pixel_size if pixel_size else max(img.shape))
                cx = int(x * scale)
                cy = int(y * scale)
                cr = int(r * scale)
                cv.circle(img, (cx, cy), cr, (0, 255, 0), 2)
                cv.circle(img, (cx, cy), 2, (0, 0, 255), 3)
            cv.imshow('Cercles détectés', img)
            cv.waitKey(0)

if __name__ == "__main__":
    main()