import numpy as np
import cv2

path = R"C:\Users\tazar\Downloads\Artifactory_logo_MARK-HEX_ORANG.png"
img = cv2.imread(path,cv2.IMREAD_COLOR)

height, width = img.shape[:2]

# Crop to a square
crop_height = 650
crop_width = 650
offset_y = 60

y_range = [int(height/2-crop_height/2-offset_y), int(height/2+crop_height/2-offset_y)]
x_range = [int(width/2-crop_width/2), int(width/2+crop_width/2)]

print(y_range)
img = img[y_range[0]:y_range[1], x_range[0]:x_range[1]]

print(f"shape={img.shape}")

centre_y = int(img.shape[0] / 2)
centre_x = int(img.shape[1] / 2)

def dist(pt1, pt2):
    return np.sqrt((pt1[0]-pt2[0])**2 + (pt1[1]-pt2[1])**2)

def gradient(pt1, pt2):
    return (pt1[1]-pt2[1])/(pt1[0]-pt2[0])

colour = (0,0,0)
width = 15
linetype = cv2.LINE_AA

key_pt1 = (centre_x-75, 75)
key_pt2 = (centre_x+75, 75)
cv2.line(img, key_pt1, key_pt2, colour, width, linetype)
key_pt3 = (centre_x-300, 570)
key_pt4 = (centre_x+300, 570)
cv2.line(img, key_pt1, key_pt3, colour, width, linetype)
cv2.line(img, key_pt2, key_pt4, colour, width, linetype)
key_pt5 = (centre_x-150, 570)
key_pt6 = (centre_x+150, 570)
cv2.line(img, key_pt3, key_pt5, colour, width, linetype)
cv2.line(img, key_pt4, key_pt6, colour, width, linetype)
key_pt7 = (centre_x-120, 500)
cv2.line(img, key_pt5, key_pt7, colour, width, linetype)
key_pt8 = (centre_x-165, 280)
key_pt9 = (centre_x-15, 280)
cv2.line(img, key_pt8, key_pt9, colour, width, linetype)
key_pt10 = (centre_x+15, 205)
cv2.line(img, key_pt9, key_pt10, colour, width, linetype)

print(f"Top bar dist = {dist(key_pt1, key_pt2)}")
print(f"Frame left gradient = {gradient(key_pt1, key_pt3)}, dist = {dist(key_pt1, key_pt3)}")
print(f"Frame right gradient = {gradient(key_pt2, key_pt4)}, dist = {dist(key_pt2, key_pt4)}")
print(f"Foot top left dist = {dist(key_pt8, key_pt9)}")
print(f"Foot bottom left dist = {dist(key_pt3, key_pt5)}")
print(f"Foot bottom right dist = {dist(key_pt4, key_pt6)}")
print(f"Tick bottom right gradient = {gradient(key_pt6, key_pt7)}, dist = {dist(key_pt6, key_pt7)}")


cv2.imshow('image',img)
cv2.waitKey(0)
cv2.destroyAllWindows()

# path_out = R"C:\Users\tazar\Downloads\out.png"
# cv2.imwrite(path_out, img)

# cv2.line(img, key_pt1, key_pt2,(0,0,0),15, cv2.LINE_8)
