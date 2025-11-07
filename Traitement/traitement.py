import cv2 as cv
import numpy as np
import sys

"""
Ce code permet de détecter des cercles dans une image de matériau composite avec la transformée de Hough en utilisant la librarie OpenCv.
Ces cercles correspondent aux fibres du matériau, et leurs paramètres (coordonées du centre/rayon) sont exportées dans un fichier qui sera 
ensuite utilisé pour la création du maillage avec Gmsh. Ce dernier sera utilisé pour la simulation par éléments finis.
"""

if __name__ == "__main__":

    if len(sys.argv) > 3 or len(sys.argv) < 2:
        print("Utilisation type : python traitement.py <image_path> [show](optionnel)")
        sys.exit(1)

    img_path = sys.argv[1]
    show = True if len(sys.argv) > 2 else False
    
    img = cv.imread(img_path)
    #Prétaitement de l'image (recadrage, conversion en niveaux de gris et floutage)
    L = max(img.shape)
    w, h = img.shape[1], img.shape[0]
    img = cv.resize(img, (int((w/L)*500), int((h/L)*500)))
    gray = cv.imread(img_path, cv.IMREAD_GRAYSCALE)
    gray = cv.resize(gray, (int((w/L)*500), int((h/L)*500)))
    blur = cv.GaussianBlur(gray, (9, 9), 2)
    #blur = cv.addWeighted(blur, 1.5, cv.GaussianBlur(blur, (0, 0), 10), -0.5, 0)
    #cv.imshow('Blurred Image', blur)
    #Détection des cercles avec la transformée de Hough
    circles = cv.HoughCircles(blur, method=cv.HOUGH_GRADIENT, dp=1, minDist=78, param1=10, param2=7, minRadius=34, maxRadius=40)

    print(f"Nombre de cercles détectés : {0 if circles is None else len(circles[0])}")
    print(f"Ecriture des paramètres (coordonnées du centre et rayon) des cercles dans le fichier circles.dat")

    if circles is not None:
        circles = np.uint16(np.around(circles))
        with open('circles.dat', 'w') as f: 
            for i in circles[0, :]:
                f.write(f"{i[0]/L} {i[1]/L} {i[2]/L}\n")
                cv.circle(img, (i[0], i[1]), i[2], (0, 255, 0), 2)
                cv.circle(img, (i[0], i[1]), 2, (0, 0, 255), 3)

    if show:
        cv.imshow('Detected Circles', img)
        cv.waitKey(0)