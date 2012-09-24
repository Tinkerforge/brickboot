Brickboot
=========

This repository contains the source code of the Brick Bootloader.

Usage
-----

To compile the C code we recommend you to install the newest CodeSourcery ARM
EABI GCC compiler
(https://sourcery.mentor.com/sgpp/lite/arm/portal/subscription?@template=lite).
You can generate a Makefile from the cmake script with the
generate_makefile shell script (in software/) and build the bootloader
by invoking make in software/build/. The bootloader (.bin) can then be found
in build/

The compiled bootloader can be used with the Atmel SAM-BA in-system programmer
(http://www.atmel.com/dyn/products/tools_card.asp?tool_id=3883). You have to
copy the folders from tcl_lib/ to the tcl_lib/ folder in SAM-BA (with your
new bootloader).
