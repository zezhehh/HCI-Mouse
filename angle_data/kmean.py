
import os
import numpy as np
from skimage.transform import hough_line, hough_line_peaks
from PIL import Image, ImageEnhance
import math
from cv2 import cv2
from scipy.ndimage.filters import gaussian_filter
import matplotlib.pyplot as plt
from sklearn.cluster import KMeans

def get_imgs():
    dirs = os.listdir()
    angles = []
    for d in dirs:
        if d[:4] == 'red_':
            angles.append(d[4:-4])
    return angles

# def red_img_name(idx_str, prefix='./'):
#     return prefix + 'red_' + idx_str + '.png'

def white_img_name(idx_str, prefix='./'):
    return prefix + 'white_' + idx_str + '.png'

# def img_name(idx_str, color, prefix='./'):
#     return prefix + color + '_' + idx_str + '.png'

def get_threshold(im, ratio=0.8):
    # ratio = 0.5 if mode == 'white' else 0.7
    threshold = 10
    im = np.array(im)
    while sum(sum(im > threshold)) > sum(sum(im >= 0)) * ratio:
        threshold += 1
    return threshold

def calc_angle(im, mode):
    BORDER = 6
    if mode == 'white':
        GAUSSIAN_STRENGTH = 1.5
        MIN_DISTANCE = 0
        MIN_ANGLE = 0
        NUM_PEAKS = 20
        TOLERATE_DIFF = 2
    elif mode == 'red':
        im = ImageEnhance.Contrast(im).enhance(4)
        GAUSSIAN_STRENGTH = 3
        MIN_DISTANCE = 2
        MIN_ANGLE = 1
        NUM_PEAKS = 10
        TOLERATE_DIFF = 3

    im = np.array(im.convert('L'))
    I_DC = gaussian_filter(im, GAUSSIAN_STRENGTH)
    I_DC = I_DC - np.min(I_DC)
    I_copy = np.zeros((19+BORDER,19+BORDER), np.uint8)
    I_copy[int(BORDER/2):19+BORDER-int(BORDER/2),int(BORDER/2):19+BORDER-int(BORDER/2)] = im
    I_DCremove = im - I_DC
    
    if mode == 'white':
        th = np.median(I_DCremove) - 6
    elif mode == 'red':
        th = get_threshold(I_DCremove)

    BW_temp = I_DCremove < th
    BW = np.zeros((19+BORDER, 19+BORDER))
    BW[int(BORDER/2):19+BORDER-int(BORDER/2),int(BORDER/2):19+BORDER-int(BORDER/2)] = BW_temp
    H, T, R = hough_line(BW * 255)
    hspace, angles, dists = hough_line_peaks(H, T, R, min_distance=MIN_DISTANCE, min_angle=MIN_ANGLE, num_peaks=NUM_PEAKS)
    H_weight = hspace.copy()
    top = min(max(5, sum(H_weight==max(H_weight))), len(angles))
    for i in range(top):
        for j in range(len(hspace)):
            if j == i:
                continue
            a1 = math.degrees(angles[i])
            a2 = math.degrees(angles[j])
            if mode == 'red':
                if abs(a1 - a2) < TOLERATE_DIFF:
                    H_weight[i] = H_weight[i] + hspace[j]
            elif mode == 'white':
                if abs(a1 - a2) < TOLERATE_DIFF or abs(abs(a1 - a2) - 90) < TOLERATE_DIFF:
                    H_weight[i] = H_weight[i] + hspace[j]
    tmp = H_weight.tolist()
    index = tmp.index(max(tmp))
    x = math.degrees(angles[index])
    rho = dists[index]
    theta = angles[index]
    ang = - x + 90
    points = calc_points(rho, theta)
    rtn = I_copy
    # rtn = draw_lines(I_copy, (((points[0], points[1]), (points[2], points[3])),))

    ag = angles[index]
    # if mode == 'red':
    #     th = get_threshold(I_DCremove, 0.6)
    #     BW_temp = I_DCremove < th
    # BW[int(BORDER/2):19+BORDER-int(BORDER/2),int(BORDER/2):19+BORDER-int(BORDER/2)] = BW_temp
    H, T, R = hough_line(BW * 255)
    hspace, angles, dists = hough_line_peaks(H, T, R, min_distance=1, min_angle=1, num_peaks=NUM_PEAKS*50)
    rhos = [[], []]
    for i in range(len(angles)):
        if math.degrees(abs(angles[i] - ag)) < TOLERATE_DIFF*2:
            # print('same angle')
            rhos[0].append(dists[i])
        if abs(90 - math.degrees(abs(angles[i] - ag))) < TOLERATE_DIFF*2:
            # print('verticle angle')
            rhos[1].append(dists[i])
    # rhos[0]: rhos of the similar angle as returned one
    # rhos[1]: rhos of the verticle angle as returned one
    return ang, rtn, rhos, x


def pil2cv(im):
    im = np.array(im)
    return cv2.cvtColor(im, cv2.COLOR_RGB2BGR)


def cv2pil(im):
    im = cv2.cvtColor(im, cv2.COLOR_BGR2RGB)
    return Image.fromarray(im)

def calc_points(rho, theta):
    a = np.cos(theta)
    b = np.sin(theta)
    x0 = a * rho
    y0 = b * rho
    x1 = int(x0 + 50*(-b))
    y1 = int(y0 + 50*(a))
    x2 = int(x0 - 50*(-b))
    y2 = int(y0 - 50*(a))
    return x1, y1, x2, y2
def draw_lines(img, lines, color=(0, 0, 255)):
    im = pil2cv(img)
    for p1, p2 in lines:
        # print(p1, p2)
        cv2.line(im, p1, p2, color, 1)
    return cv2pil(im)
def red_img_name(idx_str, prefix='./'):
    return prefix + 'red_' + idx_str + '.png'

fig = plt.figure()
ax = fig.add_subplot(211)
ax2 = fig.add_subplot(212)
plt.ion()
imgs_val = get_imgs()
# imgs_val = ['138']
t=[]
d=[]
c=0
for i in imgs_val:
    r_im = Image.open(red_img_name(i))
    w_im = Image.open(white_img_name(i))
    r_ang, rtn_r, r_rhos, raw_rx = calc_angle(r_im, 'red')
    w_ang, rtn_w, w_rhos, raw_wx = calc_angle(w_im, 'white')
    # ax.imshow(rtn)
    # plt.show()
    # plt.pause(3)
    rhos = []
    if abs(r_ang - w_ang) <= 45:
        ang = w_ang
        rhos.append(w_rhos[0])
        rhos.append(r_rhos[0])
    else:
        ang = w_ang + 90
        rhos.append(w_rhos[1])
        rhos.append(r_rhos[0])
    min_dist = 999
    min_dist_r = [[], []]
    km1 = KMeans(n_clusters=3).fit(np.array(rhos[0]).reshape(-1,1))
    km2 = KMeans(n_clusters=3).fit(np.array(rhos[1]).reshape(-1,1))
    result1 = km1.cluster_centers_.reshape(1,-1).tolist()[0]
    result2 = km2.cluster_centers_.reshape(1,-1).tolist()[0]
    result1.remove(min(result1))
    result1.remove(max(result1))
    result2.remove(min(result2))
    result2.remove(max(result2))
    extract_rho1 = min(result1)
    extract_rho2 = min(result2)
    print('extract:', extract_rho1-extract_rho2)
    # print('km1',km1.cluster_centers_.reshape(1,-1).tolist()[0])
    # print('km2',km2.cluster_centers_.reshape(1,-1).tolist()[0])

    for r1 in rhos[0]:
        for r2 in rhos[1]:
            if r1 == r2:
                continue
            if abs(r1-r2) < min_dist:
                min_dist = abs(r1-r2)
                min_dist_r[0] = [r1]
                min_dist_r[1] = [r2]
            elif abs(r1-r2) == min_dist:
                # print(min_dist_r[0] - r1, min_dist_r[1] - r2)
                # if abs(np.mean(min_dist_r[0]) - r1) < min_dist:
                #     min_dist_r[0].append(r1)
                #     min_dist = abs(np.mean(min_dist_r[0]) - np.mean(min_dist_r[1]))
                if abs(np.mean(min_dist_r[1]) - r2) < min_dist:
                    min_dist_r[1].append(r2)
                    min_dist = abs(np.mean(min_dist_r[0]) - np.mean(min_dist_r[1]))
    if min_dist_r == [[], []]:
        print(i)
        # c+=1
    min_dist_r = [np.mean(min_dist_r[0]), np.mean(min_dist_r[1])]
    raw_rx = math.radians(raw_rx)
    # points = calc_points(min_dist_r[0], raw_rx)
    points = calc_points(extract_rho1, raw_rx)
    
    fim_w = draw_lines(rtn_w, (((points[0], points[1]), (points[2], points[3])),), \
            color=(255, 0, 0))
    # points = calc_points(min_dist_r[1], raw_rx)

    points = calc_points(extract_rho2, raw_rx)

    fim_r = draw_lines(rtn_r, (((points[0], points[1]), (points[2], points[3])),), \
            color=(255, 255, 0))

    positive = min_dist_r[0] - min_dist_r[1] > 0
    if min_dist > 3:
        print('too large min_dist', min_dist)
    # print(min_dist, min_dist_r, raw_rx)
    if ang > 180:
        ang -= 180
    ang -= 90
    if ang < 0:
        ang += 180
    correct = int(i)
    if correct > 180:
        correct -= 180
    diff1= abs(ang-correct)
    # diff2 = abs(90-abs(ang-correct))
    diff3 = abs(180-abs(ang-correct))
    min_diff = min([diff1,diff3])
    # min_diff = diff2
    if round(min_diff) > 3:
        t.append(i)
        d.append(min_diff)
        print(i, ang, min_diff)
#     break
    theta_positive = raw_rx > 0
    
    if positive == theta_positive:
        assert1 = abs(180 - abs(int(i) - ang)) < 10
        # print(abs(180 - abs(int(i) - ang)) < 10)
        if assert1:
            c += 1
        else:
            print(positive, theta_positive, abs(int(i) - ang), min_dist_r)
            ax.imshow(fim_r)
            ax2.imshow(fim_w)
            plt.show()
            plt.pause(3)
            print(i)
    else:
        assert2 = abs(int(i) - ang) < 10
        # print(abs(int(i) - ang) < 10)
        if assert2:
            c += 1
        else:
            print(positive, theta_positive, int(i), ang, min_dist_r)
            ax.imshow(fim_r)
            ax2.imshow(fim_w)
            plt.show()
            plt.pause(3)
            print(i)
print(len(t))
print(100-c)
