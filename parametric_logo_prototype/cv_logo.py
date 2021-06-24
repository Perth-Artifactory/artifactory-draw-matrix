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
    dx = pt1[1]-pt2[1]
    dy = pt1[0]-pt2[0]
    d = np.sqrt((dy)**2 + (dx)**2)
    return f'{d}, dx={dx}, dy={dy}'

def gradient(pt1, pt2):
    grad = (pt1[1]-pt2[1])/(pt1[0]-pt2[0])
    angle = np.arctan2(pt1[1]-pt2[1], pt1[0]-pt2[0])
    return f'{grad}, {angle/np.pi*180} deg'

colour = (0,0,0)
width = 15
linetype = cv2.LINE_AA

# Top
key_pt1 = (centre_x-75, 75)
key_pt2 = (centre_x+75, 75)
cv2.line(img, key_pt1, key_pt2, colour, width, linetype)

# Base
key_pt3 = (centre_x-300, 570)
key_pt4 = (centre_x+300, 570)
# Outer lines

# Inner foot
key_pt5 = (centre_x-150, 570)
key_pt6 = (centre_x+150, 570)

# Bottom left tick
# key_pt7 = (centre_x-120, 500)
theta = 114.44395478041653 * np.pi / 180
key_pt7 = (int(key_pt5[0] - 75*np.cos(theta)), int(key_pt5[1] - 75*np.sin(theta)))

# Upper foot
#key_pt8 = (centre_x-170, 280)
key_pt8 = (int(key_pt1[0] + 225*np.cos(theta)), int(key_pt1[1] + 225*np.sin(theta)))
key_pt9 = (key_pt8[0]+150, key_pt8[1])

# Upper tick
# key_pt10 = (centre_x+15, 205)
key_pt10 = (int(key_pt9[0] - 75*np.cos(theta)), int(key_pt9[1] - 75*np.sin(theta)))

# Middle right inner
key_pt11 = (int(key_pt6[0] + 200*np.cos(theta)), int(key_pt6[1] - 200*np.sin(theta)))

# Middle left lower
key_pt12 = (int(key_pt3[0] - 200*np.cos(theta)), int(key_pt3[1] - 200*np.sin(theta)))

# #cv2.line(img, key_pt1, key_pt3, colour, width, linetype)
# cv2.line(img, key_pt2, key_pt4, colour, width, linetype)
# cv2.line(img, key_pt3, key_pt5, colour, width, linetype)
# cv2.line(img, key_pt4, key_pt6, colour, width, linetype)
# cv2.line(img, key_pt5, key_pt7, colour, width, linetype)
# cv2.line(img, key_pt8, key_pt9, colour, width, linetype)
# cv2.line(img, key_pt9, key_pt10, colour, width, linetype)
# cv2.line(img, key_pt6, key_pt11, colour, width, linetype)
# cv2.line(img, key_pt11, key_pt12, colour, width, linetype)
# cv2.line(img, key_pt1, key_pt8, colour, width, linetype)
# cv2.line(img, key_pt3, key_pt12, colour, width, linetype)

order = [
    key_pt10, key_pt9, key_pt8,
    key_pt1, key_pt2,
    key_pt4, key_pt6, key_pt11,
    key_pt12, key_pt3, key_pt5, key_pt7,
]

for start, end in zip(order[:-1], order[1:]):
    # cv2.line(img, start, end, colour, width, linetype)
    cv2.arrowedLine(img, start, end, colour, width, linetype)

print(f"Top bar dist = {dist(key_pt1, key_pt2)}")
print(f"Frame left gradient = {gradient(key_pt1, key_pt3)}, dist = {dist(key_pt1, key_pt3)}")
print(f"Frame right gradient = {gradient(key_pt2, key_pt4)}, dist = {dist(key_pt2, key_pt4)}")
print(f"Foot top left dist = {dist(key_pt8, key_pt9)}")
print(f"Foot bottom left dist = {dist(key_pt3, key_pt5)}")
print(f"Foot bottom right dist = {dist(key_pt4, key_pt6)}")
print(f"Tick bottom left gradient = {gradient(key_pt5, key_pt7)}, dist = {dist(key_pt5, key_pt7)}")


cv2.imshow('image',img)
cv2.waitKey(0)
cv2.destroyAllWindows()

# path_out = R"C:\Users\tazar\Downloads\out.png"
# cv2.imwrite(path_out, img)

# cv2.line(img, key_pt1, key_pt2,(0,0,0),15, cv2.LINE_8)
