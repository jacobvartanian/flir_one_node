from PIL import Image
import numpy as np
import ipdb

chunk_file = open('one_chunk2.txt', 'r')
lines = chunk_file.readlines()

frame_size_start = 9
thermal_size_start = frame_size_start+4
jpg_size_start = thermal_size_start+4
status_size_start = jpg_size_start+4

frame_size = 0
for i in range(frame_size_start,frame_size_start+4):
    # ipdb.set_trace()
    frame_size += int(lines[i]) << 8*(i - frame_size_start)

thermal_size = 0
for i in range(thermal_size_start,thermal_size_start+4):
    thermal_size += int(lines[i]) << 8*(i - thermal_size_start)

jpg_size = 0
for i in range(jpg_size_start,jpg_size_start+4):
    jpg_size += int(lines[i]) << 8*(i - jpg_size_start)

status_size = 0
for i in range(status_size_start,status_size_start+4):
    status_size += int(lines[i]) << 8*(i - status_size_start)

print(f"frame size: {frame_size}")
print(f"thermal size: {thermal_size}")
print(f"jpg size: {jpg_size}")
print(f"status size: {status_size}")

image_array = np.zeros(shape=(120,160), dtype=np.uint16)
min_pixel = 256*256
max_pixel = 0
for y in range(120):
    for x in range(160):
        pixel_index = 2*(y * 164 + x)
        # ipdb.set_trace()
        if (x < 80):
            v = int(lines[pixel_index +33])+256*int(lines[pixel_index +34])
            # v = int(lines[pixel_index +33])
        else:
            v = int(lines[pixel_index +33+4])+256*int(lines[pixel_index +34+4])
            # v = int(lines[pixel_index +33+4])
        if (x == 80):
            print(int(lines[pixel_index +33+3]))
        if (x == 159):
            print(int(lines[pixel_index +33+6+3]))
        if v < min_pixel:
            min_pixel = v
        if v > max_pixel:
            max_pixel = v
        image_array[y][x] = v
    # print(pixel_index)
# ipdb.set_trace()
scaled_image_array = np.zeros(shape=(120,160), dtype=np.uint16)
pixel_range = max_pixel - min_pixel
for y in range(120):
    for x in range(160):
        v = ((image_array[y][x] - min_pixel) / pixel_range) * 256*256
        scaled_image_array[y][x] = v

new_image = Image.fromarray(scaled_image_array)
new_image.save('new3.png')
# print(image_array)
# print(scaled_image_array)

with open('rgb.jpg', 'wb') as img_file:
    for data_index in range(thermal_size + 29, thermal_size + 29 + jpg_size):
        img_file.write((int(lines[data_index])).to_bytes(1, byteorder='big', signed=False))

ipdb.set_trace()