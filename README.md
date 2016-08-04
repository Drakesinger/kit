# The Kit Framework

## Notice

This project is not by any means ready for an official release. Features may not work as advertised, or even exist at all. Documentation is practially non-existent. Feel free to contribute!

Sharing this mainly for educational reasons. Enjoy!

## Dependencies

Kit depends on the following libraries:

* GLFW3 (Window and input management)
* GLM (Math)
* Freetype 2 (Text rendering)
* Chaiscript (Scripting)
* OpenGL (Rendering)

Additionally, the asset importer depends on libassimp (and Qt5 on GNU/Linux distributions)

## Building 

### Linux

You build both the library as well as the tools using plain ol' makefiles.

#### The library

The makefile for the library exists in the root folder. Do something like:

`make -j 10 && sudo make install`

Tip: Use -j 10 to parallelize the object compilation, which makes it compile much faster.

#### The tools

The tools reside inside `./tools/<toolname>/`. Build it by calling `make` as usual. Installing it will put the binaries in the `dist` folder.

### Windows

Check the `vs2015` folder for a solution. It is known to be buildable, every dependency except `chaiscript` and `assimp` exist in NuGet. Please report any issues you might have using this solution.

## How to contribute

Open issues, create pull requests. But please get to know the codebase a bit first, learn the programming idioms. Even if a lot of code in this project currently is pretty ugly, I still require a very high standard of code-quality.
