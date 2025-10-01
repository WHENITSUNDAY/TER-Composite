import cv2 as cv
import numpy as np
from matplotlib import pyplot as plt


img = cv.imread('images_test/1.png')
gray = cv.imread('images_test/1.png', cv.IMREAD_GRAYSCALE)

mean = np.mean(gray)
#binary = cv.adaptiveThreshold(gray, 255, cv.ADAPTIVE_THRESH_GAUSSIAN_C, cv.THRESH_BINARY, 11, 2)
_, binary = cv.threshold(gray, mean - 20, 255, cv.THRESH_BINARY)

cv.imshow('Image binaire', binary)
cv.waitKey(0)

blur = cv.GaussianBlur(binary, (9, 9), 2)

circles = cv.HoughCircles(blur, method=cv.HOUGH_GRADIENT, dp=1, minDist=20, param1=100, param2=18, minRadius=25, maxRadius=50)

if circles is not None:
    circles = np.uint16(np.around(circles))
    for i in circles[0, :]:
        cv.circle(img, (i[0], i[1]), i[2], (0, 255, 0), 2)
        cv.circle(img, (i[0], i[1]), 2, (0, 0, 255), 3)

cv.imshow('Detected Circles', img)
cv.waitKey(0)