import matplotlib.pyplot as plt
import numpy as np
from PIL import Image
def byte_to_int_list (a):
        return [(a // (4 ** i)) % 4 for i in range (0, 4)]
file_names = ["idle-down", "idle-side", "idle-up"]
index = 0
mem_dir = "memory/"
#color_file = mem_dir + "colors.mif"
test_dir = "tests/"
for name in file_names:
        with open(mem_dir + name + ".mif", "r") as f:
                lines = f.readlines()
        sprite_content = lines[6:-1]
        color = np.zeros((4,3))
        '''with open(color_file, "r") as f:
                raw = f.readlines()[6:-1]
                global_colors_raw = raw[len(file_names) * 4:]
                global_colors = [int(global_colors_raw[i].split(":")[1][0:-2], 16) for i in range(0,len(global_colors_raw))]
                global_colors = np.array(global_colors).reshape(len(global_colors_raw) // 3, 3)
                local_colors_raw = raw[index: index + 4]
                local_colors = [int(local_colors_raw[i].split(":")[1][0:-2], 16) for i in range(0,4)]
                color = [global_colors[i] for i in local_colors]'''
        pixels_64 = [int(sprite_content[i].split(":")[1][0:-2], 16) for i in range(0,64)]
        pixels_256 = np.array([ byte_to_int_list(a) for a in pixels_64 ]).reshape(256)
        #graph = np.array([color[i] for i in pixels_256]).reshape((16,16,3), order='A')
        #print(color)
        print(pixels_256)