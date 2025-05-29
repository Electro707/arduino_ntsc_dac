#!/bin/python3

from PIL import Image


file_path = 'image1.png'
# file_path = 'eid.png'
out_name = '../image.h'

i = Image.open(file_path).convert(mode='L')

# i = i.resize(size=(360, 243), resample=Image.NEAREST)


to_print = f"#include <stdint.h>\n#include <avr/pgmspace.h>\nconst uint8_t PROGMEM image[{122*180}] = {{"
for y in range(122):
    for x in range(180):
        # print(i.getpixel((x, y)))
        v = i.getpixel((x, y))

        v = int(((255-87) * (v / 255)) + 87)
        v = max(v, 87)
        if v < 87:
            raise UserWarning()
        to_print += f"{v},"

to_print = to_print[:-1]
to_print += "};"

with open(out_name, 'w') as f:
    f.write(to_print)
