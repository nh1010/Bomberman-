import matplotlib.pyplot as plt
import numpy as np
from PIL import Image
from sklearn.cluster import KMeans

MEM_DIR = "memory/"
RGB_R_FILE = MEM_DIR + "global_color_r.mif"
RGB_G_FILE = MEM_DIR + "global_color_g.mif"
RGB_B_FILE = MEM_DIR + "global_color_b.mif"
MIF_MAP_FILE = MEM_DIR + "map_sprite.txt"
LOCAL_COLORS_FILE = MEM_DIR + "local_colors.mif"
IMAGE_DIR = "images/"
TEST_DIR = "tests/"

def read_mif(mif_dir : str, func):
        data = []
        with open(mif_dir, "r") as f:
                string_data = f.readlines()[6:-1]
                data = [func(i) for i in string_data]
        return data

def read_mif_int(mif_dir : str):
        return read_mif(mif_dir, lambda x : int(x.split(":")[1][1:-2], 16))

#resize the sprite to 16 * 16 using resampling alogorithm. Save the file to bmp
def resample_png_to_bmp(name : str):
        img = Image.open(IMAGE_DIR + name + ".png")
        img = img.resize((16,16))
        img = img.convert("RGB")
        img.save(IMAGE_DIR + name + ".bmp", "BMP")

def read_global_color():
        global_color_r = read_mif_int(RGB_R_FILE)
        global_color_r = np.array(global_color_r).reshape(len(global_color_r),1)
        global_color_g = read_mif_int(RGB_G_FILE)
        global_color_g = np.array(global_color_g).reshape(len(global_color_g),1)
        global_color_b = read_mif_int(RGB_B_FILE)
        global_color_b = np.array(global_color_b).reshape(len(global_color_b),1)
        global_color = np.concatenate((global_color_r, global_color_g, global_color_b), axis=1)
        return global_color.tolist()

def add_to_global_color(global_color, color, tol):
        diffs = np.array([np.linalg.norm(color - a) for a in global_color])
        min_diff = min(diffs)
        if (min_diff < tol):
                return np.argmin(diffs)
        global_color.append(color)
        return (len(global_color) - 1)
        
def int_to_byte(l):
        i = 0
        index = 1
        for num in l:
                i = i + num * index
                index = index * 4
        return "{:02x}".format(i)

def write_mif(mif_dir : str, l):
        with open(mif_dir, "w") as f:
                f.write("DEPTH = {:};\n".format(len(l)))
                f.write("WIDTH = 8;\n")
                f.write("ADDRESS_RADIX = HEX;\n")
                f.write("DATA_RADIX = HEX;\n")
                f.write("CONTENT\n")
                f.write("BEGIN\n")
                addr = 0
                for i in l:
                        f.write("{:02x}".format(addr) + " : " + i + ";\n")
                        addr += 1
                f.write("END;\n")
#will change original mif files, please backup manually
def add_mif_from_png(name : str, pos : int, max_diff : int):
        curr_mif_map = read_mif(MIF_MAP_FILE, lambda x : x.split(":")[1][1:-2])
        if (name in curr_mif_map):
                print(name + "already in the map!\n")
                exit(-1)
        insertion_pos = pos
        if (pos > len(curr_mif_map) or pos < 0):
                insertion_pos = len(curr_mif_map)
        resample_png_to_bmp(name)
        img = plt.imread(IMAGE_DIR + name + ".bmp")
        print(img)
        img = img.reshape(256,3)
        #Automatically generate four colors by doing the clusters
        kmeans = KMeans(n_clusters=4, max_iter=10000).fit(img) 
        color_map =  kmeans.cluster_centers_.reshape(12)
        #round the floating point down and generate 4 * 3 array
        color_map = np.array([int(color) for color in color_map]).reshape(4,3)
        #initialize the global_color_map
        global_color = read_global_color()
        local_color_map = [add_to_global_color(global_color, color, max_diff) for color in color_map]
        local_colors = read_mif_int(LOCAL_COLORS_FILE)
        #generate new local colors

        if (insertion_pos == len(curr_mif_map)):
                for colormap in local_color_map:
                        local_colors.append(colormap)
                curr_mif_map.append(name)
        else:
                for i in range(0,4):
                        local_colors.insert(4 * insertion_pos + i, local_color_map[i])
                curr_mif_map.insert(insertion_pos, name)
        #change name.mif, local_colors.mif, map_sprite.txt global_colors
        sprite = kmeans.labels_
        #write sprite
        write_mif(MEM_DIR + name + ".mif", [int_to_byte(x) for x in [sprite[4 * i: 4 * i + 4] for i in range(0, 64)]])
        #write local color
        write_mif(LOCAL_COLORS_FILE, ["{:02x}".format(x) for x in local_colors])
        #write mif map
        write_mif(MIF_MAP_FILE, curr_mif_map)
        #write global colors
        write_mif(RGB_R_FILE, ["{:02x}".format(x[0]) for x in global_color])
        write_mif(RGB_G_FILE, ["{:02x}".format(x[1]) for x in global_color])
        write_mif(RGB_B_FILE, ["{:02x}".format(x[2]) for x in global_color])

def reproduce_png():
        names = read_mif(MIF_MAP_FILE, lambda x : x.split(":")[1][1:-2])
        global_colors = read_global_color()
        local_color = read_mif_int(LOCAL_COLORS_FILE)
        local_color_addr = 0
        for name in names:
                sprites = read_mif_int(MEM_DIR + name + ".mif")
                sprites = np.array([[(x // (4 ** i)) % 4 for i in range (0, 4)] for x in sprites])
                sprites = sprites.flatten()
                graph = [global_colors[i] for i in [local_color[local_color_addr + j] for j in sprites]]
                graph = np.array(graph).reshape(16, 16, 3)
                plt.imsave(TEST_DIR + name + ".png", graph / 255)
                local_color_addr += 4

#generate a sprite with same shape but different color_map. change local_color_maps global_color_map, need to backup manually.
def generate_color_remap(name_from : str, name_to : str, pos : int, remaps, tol):
        curr_mif_map = read_mif(MIF_MAP_FILE, lambda x : x.split(":")[1][1:-2])
        if (name_to in curr_mif_map):
                print(name_to + "already in the map!\n")
                exit(-1)
        if not name_from in curr_mif_map:
                print(name_from + "does not exist!\n")
                exit(-1)
        insertion_pos = pos
        if (pos > len(curr_mif_map) or pos < 0):
                insertion_pos = len(curr_mif_map)
        global_color = read_global_color()
        local_color_map = [add_to_global_color(global_color, color, tol) for color in remaps]
        local_colors = read_mif_int(LOCAL_COLORS_FILE)
        if (insertion_pos == len(curr_mif_map)):
                for colormap in local_color_map:
                        local_colors.append(colormap)
                curr_mif_map.append(name_to)
        else:
                for i in range(0,4):
                        local_colors.insert(4 * insertion_pos + i, local_color_map[i])
                curr_mif_map.insert(insertion_pos, name_to)
        #just copy the sprite file
        sprites = read_mif_int(MEM_DIR + name_from + ".mif")
        write_mif(MEM_DIR + name_to + ".mif", ["{:02x}".format(x) for x in sprites])
        #write local color
        write_mif(LOCAL_COLORS_FILE, ["{:02x}".format(x) for x in local_colors])
        #write mif map
        write_mif(MIF_MAP_FILE, curr_mif_map)
        #write global colors
        write_mif(RGB_R_FILE, ["{:02x}".format(x[0]) for x in global_color])
        write_mif(RGB_G_FILE, ["{:02x}".format(x[1]) for x in global_color])
        write_mif(RGB_B_FILE, ["{:02x}".format(x[2]) for x in global_color])

def merge_mifs(l : list, to : str):
        result = []
        for s in l:
                i = read_mif_int(s)
                result = result + i
        write_mif(to, ["{:02x}".format(x) for x in result])



#add_mif_from_png("background", 0, 5)
#add_mif_from_png("stone", 1, 5)
#add_mif_from_png("box", 2, 5)
remap_0 = [[255, 255, 255],
         [36, 127, 207],
         [39, 76, 108],
         [203, 170, 130]]

remap_1 = [[255,255,255],
           [39,76,108],
           [203, 170, 130],
           [54,102,204]]

remap_2 = [[54,102,204],
           [255,255,255],
           [39,76,108],
           [25,37,62]]

remap_3 = [[203, 170, 130],
           [255, 255, 255],
         [39, 76, 108],
         [36, 127, 207]]

remap = [[255, 255, 255],
         [36, 127, 207],
         [39, 76, 108],
         [203, 170, 130],
         [203, 170, 130],
         [54, 102, 204],
         [39, 76, 108],
         [25, 37, 62]]               

        

'''player_sprite_list = ["idle-down", "idle-side", "idle-up", "walk-down1", "walk-down2", "walk-side1", "walk-side2", "walk-side3", "walk-up1", "walk-up2"]
new_p = []
for s in player_sprite_list:
        new_p.append(s + "_blue")
player_sprite_list = player_sprite_list + new_p
merge_mifs([MEM_DIR + x + ".mif" for x in player_sprite_list], "players.mif")'''

#add_mif_from_png("walk-down1", -1, 10)
#add_mif_from_png("walk-down2", -1, 10)
#add_mif_from_png("walk-side1", -1, 10)
#add_mif_from_png("walk-side2", -1, 10)
#add_mif_from_png("walk-side3", -1, 10)
#add_mif_from_png("walk-up1", -1, 10)
#add_mif_from_png("walk-up2", -1, 10)
#generate_color_remap("walk-down1", "walk-down1_blue", -1, np.array([remap[x] for x in [4, 0, 2, 1]]), 5)
#generate_color_remap("walk-down2", "walk-down2_blue", -1, np.array([remap[x] for x in [2, 0, 1, 4]]), 5)
#generate_color_remap("walk-side1", "walk-side1_blue", -1, np.array([remap[x] for x in [0, 2, 5, 4]]), 5)
#generate_color_remap("walk-side2", "walk-side2_blue", -1, np.array([remap[x] for x in [1, 0, 2, 4]]), 5)
#generate_color_remap("walk-side3", "walk-side3_blue", -1, np.array([remap[x] for x in [0, 2, 5, 4]]), 5)
#generate_color_remap("walk-up1", "walk-up1_blue", -1, np.array([remap[x] for x in [5, 0, 6, 7]]), 5)
#generate_color_remap("walk-up2", "walk-up2_blue", -1, np.array([remap[x] for x in [5, 0, 6, 7]]), 5)
reproduce_png()
'''generate_color_remap("idle-down", "idle-down_blue", 13, np.array([remap[x] for x in [0, 1, 2, 3]]), 5)
generate_color_remap("idle-side", "idle-side_blue", 14, np.array([remap[x] for x in [0, 2, 4, 5]]), 5)
generate_color_remap("idle-up", "idle-up_blue", 15, np.array([remap[x] for x in [5, 0, 6, 7]]), 5)
reproduce_png()'''

        






        
        
        
