# Composite Video Arduino Example

This is code that was used in my writting of my [first](https://blogs.electro707.com/electronics/2024/06/24/Arduino-Composite-Video-Part2.html) and [second blog post](https://blogs.electro707.com/electronics/2024/06/18/Arduino-Composite-Video.html) about creating an NTSC composite video signal from an Arduino Nano

# Physical Requirements

Per the first blog post, the requirements to run this is an Arduino Uno connected to an R2R resistor ladder of ~300ohms from Pin 0 to 7

# Build and Run
To build this application, just run `make`. You will need `avr-gcc` installed on your system.

To program to the Arduino, run `make program`, which will attempt to program to the device connected to `/dev/ttyUSB0`. You may need to change that port in the makefile.

The following make commands are setup:
- <nothing>: runs through `compile` and `size`
- `compile`: compiles the code
- `quick`: runs `compile`, `size`, then `program`
- `size`: uses `avr-size` to print out memory usage
- `program` or `prog`: uploads the compiled code over to the Arduino
- `clean`: cleans up workspace
- `asm`: dumps the main program into assembly for inspection

# Image building
To build the image define `image.h`, use the program in `imageGen/genH.py`. You may have to modify the `file_path` variable for the desired image to use. The image should be 180x122 pixels (at least what the script currently expects)

# License

This code, as bad as it is, is licensed under [GPLv3](LICENSE.md)
