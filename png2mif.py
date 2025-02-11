import matplotlib.pyplot as plt
import numpy as np
from PIL import Image
from sklearn.cluster import KMeans
def int_to_byte(l):
        i = 0
        index = 1
        for num in l:
                i = i + num * index
                index = index * 4
        return "{:02x}".format(i)
def diff_global(color, glob, thres):
        if (glob == []):
               return -1
        
        diffs = np.array([np.linalg.norm(color - a) for a in glob])
        min_diff = min(diffs)
        if (min_diff < thres):
                return np.argmin(diffs)
        return -1

file_names = ["idle-down", "idle-side", "idle-up"]
mem_dir = "memory/"
global_color_map = []
generated_color_map = []
for name in file_names:
        img = Image.open("images/" + name + ".png")
        img.thumbnail((16,16))
        img = img.convert("RGB")
        img.save("images/" + name + ".bmp", "BMP")
        img2 = plt.imread("images/" + name + ".bmp")
        img2 = img2.reshape(256,3)
        kmeans = KMeans(n_clusters=4, max_iter=1000).fit(img2) #Automatically generate four colors by doing the clusters
        color_map =  kmeans.cluster_centers_.reshape(12)
        color_map = np.array([int(color) for color in color_map]).reshape(4,3)
        #print(color_map)
        #generate a global_color_map
        for color in color_map:
                pos = diff_global(color, global_color_map, 10)
                if (pos < 0):
                        global_color_map.append(color)
                        generated_color_map.append(len(global_color_map) - 1)
                else:
                        generated_color_map.append(pos)

        sprite = kmeans.labels_
        with open(mem_dir + name + ".mif", "w") as f:
                f.write("DEPTH = 64;\n" )
                f.write("WIDTH = 8;\n")
                f.write("ADDRESS_RADIX = HEX;\n")
                f.write("DATA_RADIX = HEX;\n")
                f.write("CONTENT\n")
                f.write("BEGIN\n")
                addr = 0
                for i in range(0,64):
                        info = int_to_byte(sprite[4 * i : 4 * i + 4])
                        f.write("{:02x}".format(addr) + " : " + info + ";\n")
                        addr += 1
                f.write("END;\n")
#write the color map file
print("Formatting {:} colors to mlf file!\n".format(len(global_color_map)))
global_color_map = np.array(global_color_map)

for i in range(0, len(global_color_map)):
        if (sum(global_color_map[i]) == 765):
                print("White color is the {:}th color\n".format(i))

global_color_R = [x[0] for x in global_color_map]
global_color_G = [x[1] for x in global_color_map]
global_color_B = [x[2] for x in global_color_map]
with open(mem_dir + "local_colors.mif", "w") as f:
        total_len = len(generated_color_map)
        f.write("DEPTH = {:};\n".format(total_len))
        f.write("WIDTH = 8;\n")
        f.write("ADDRESS_RADIX = HEX;\n")
        f.write("DATA_RADIX = HEX;\n")
        f.write("CONTENT\n")
        f.write("BEGIN\n")
        addr = 0
        for i in range(0,len(generated_color_map)):
                info = "{:02x}".format(generated_color_map[i])
                f.write("{:02x}".format(addr) + " : " + info + ";\n")
                addr += 1
        
        f.write("END;\n")
with open(mem_dir + "global_color_r.mif", "w") as f:
        total_len = len(global_color_R)
        f.write("DEPTH = {:};\n".format(total_len))
        f.write("WIDTH = 8;\n")
        f.write("ADDRESS_RADIX = HEX;\n")
        f.write("DATA_RADIX = HEX;\n")
        f.write("CONTENT\n")
        f.write("BEGIN\n")
        addr = 0
        for i in range(0,total_len):
                info = "{:02x}".format(global_color_R[i])
                f.write("{:02x}".format(addr) + " : " + info + ";\n")
                addr += 1
        
        f.write("END;\n")
with open(mem_dir + "global_color_g.mif", "w") as f:
        total_len = len(global_color_G)
        f.write("DEPTH = {:};\n".format(total_len))
        f.write("WIDTH = 8;\n")
        f.write("ADDRESS_RADIX = HEX;\n")
        f.write("DATA_RADIX = HEX;\n")
        f.write("CONTENT\n")
        f.write("BEGIN\n")
        addr = 0
        for i in range(0,total_len):
                info = "{:02x}".format(global_color_G[i])
                f.write("{:02x}".format(addr) + " : " + info + ";\n")
                addr += 1
        
        f.write("END;\n")
with open(mem_dir + "global_color_b.mif", "w") as f:
        total_len = len(global_color_B)
        f.write("DEPTH = {:};\n".format(total_len))
        f.write("WIDTH = 8;\n")
        f.write("ADDRESS_RADIX = HEX;\n")
        f.write("DATA_RADIX = HEX;\n")
        f.write("CONTENT\n")
        f.write("BEGIN\n")
        addr = 0
        for i in range(0,total_len):
                info = "{:02x}".format(global_color_B[i])
                f.write("{:02x}".format(addr) + " : " + info + ";\n")
                addr += 1
        
        f.write("END;\n")


        