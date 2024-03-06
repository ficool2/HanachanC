# HanachanC
Port of [Hanachan](https://github.com/stblr/Hanachan) to C11.

See the original repo for more information.

Note: this was a personal project for fun. Keep on eye-out for [Kinoko](https://github.com/vabold/Kinoko) (C++ implementation) which is WIP but will be more complete.

![Preview](https://github.com/ficool2/HanachanC/blob/main/preview.png)

## Extended Features
- Graphical view with SDL2 and OpenGL 3.1
- Freeroam + freecam mode
- Multiple desync fixes
- Braking and deacceleration
- Standstill mini turbos (SSMT)

## Usage
Download the [release](https://github.com/ficool2/HanachanC/releases/). Run the execuable for usage.

#### Controls

##### General
* Z : Freeroam
* X : Freecam
* C : Pause
* Right arrow : Step one frame when paused

##### Freeroam
* W : Accelerate/dive down
* S : Brake/dive up
* A : Turn left
* D : Turn right
* E : Mushroom
* Space : Hop/drift
* Up/Down/Left/Right arrow : Trick/wheelie

##### Freecam
* W/A/S/D : Move camera
* Shift : Move x3 faster
* Mouse : Rotate camera
* Space : Move vertically

## Building
Open the .sln file in Visual Studio 2022 and build.

## License
Copyright 2003-2021 Dolphin Emulator Project

Copyright 2020-2021 Pablo Stebler

##### Libraries
* [GLEW](https://glew.sourceforge.net/)
* [SDL2](https://www.libsdl.org/)
* [tinydir](https://github.com/cxong/tinydir)
* Modified [glText](https://github.com/vallentin/glText)