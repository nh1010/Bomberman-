import matplotlib.pyplot as plt
import numpy as np
from PIL import Image
import random
import mifpngtool
map_element_names = mifpngtool.read_mif(mifpngtool.MIF_MAP_FILE, lambda x : x.split(":")
                                       [1][1:-2])
map_element_list = []
for name in map_element_names:
        map_element_list.append(plt.imread("tests/" + name + ".png"))

def manhattan_distance(x,y):
        return abs(x[0] - y[0]) + abs(x[1] - y[1])

def row_gen(map, row):
        pic_row = [map_element_list[x] for x in map[row]]
        return np.concatenate([np.array(x) for x in pic_row], axis=1)
map = np.zeros((30,40), dtype=int)
pos_p1 = (10,10)
pos_p2 = (20,30)
for i in range(0, 30):
        for j in range(0, 40):
                if (i % 2 == 1 and j % 2 == 1):
                        map[i][j] = 1
                else:
                        manhattan_d = min(manhattan_distance((i,j), pos_p1), manhattan_distance((i,j), pos_p2))
                        thres = 0.8 * manhattan_d / 30
                        if (manhattan_d < 4):
                                thres = 0
                        if (random.random() < thres):
                                map[i][j] = 2

pic = np.concatenate([row_gen(map, i) for i in range(0,30)], axis=0)
#visualize the map
plt.imsave("tests/map.png", pic)
map = map.reshape(600,2)
map = [(x[0] + x[1] * 16) for x in map]
mifpngtool.write_mif(mifpngtool.MEM_DIR + "map.mif", ["{:02x}".format(x) for x in map])



