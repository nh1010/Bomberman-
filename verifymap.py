import matplotlib.pyplot as plt
import numpy as np
from PIL import Image
data = np.loadtxt("output.txt", dtype=int, delimiter=',')[3:(3 + 480 * 640)].reshape((480,640,3))
plt.imsave("tests/hw_map_test.png", data / 255)
print(data.shape)