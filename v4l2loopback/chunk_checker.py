from PIL import Image
import numpy as np
import ipdb

chunk_file = open('one_chunk.txt', 'r')
lines = chunk_file.readlines()

frame_size_start = 8
thermal_size_start = frame_size_start+4
jpg_size_start = thermal_size_start+4
status_size_start = jpg_size_start+4

frame_size = 0
for i in range(frame_size_start,frame_size_start+4):
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
for y in range(120):
    for x in range(160):
        # ipdb.set_trace()
        if (x < 80):
            v = int(lines[-1+2*(y * 164 + x) +32])+256*int(lines[-1+2*(y * 164 + x) +33])
        else:
            v = int(lines[-1+2*(y * 164 + x) +32+4])+256*int(lines[-1+2*(y * 164 + x) +33+4])
        image_array[y][x] = v


new_image = Image.fromarray(image_array)
new_image.save('new.png')
print(image_array)