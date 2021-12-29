# TarotClub - The Card Game

This is the main repository for TarotClub.

# How to build on Linux

You will need the SDL2 library version 2.0.18 or higher. If it is not the case, build SDL2 from the source code.

## Build SDL2

The installation is system wide and will be located in /usr/local

* Download SDL source code (libsdl.org)
* cd SDL2-2.0.xx (xx is the last version available)
* ./configure
* make
* sudo make install

## Build TarotClub

* cd tarotclub-app
* mkdir -p build
* cd build
* cmake ..
* make

# How to generate the distribution archive

From the build path:

* cpack



