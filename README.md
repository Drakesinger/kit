![The Kit Framework](http://svkonsult.se/kit-banner-small.png)

# Notice

While I'm working very hard on this project, *Kit* is not by any means ready for an official release. Features may not work as advertised, or even exist at all. Documentation and example code is practically non-existent, although it's being highly prioritized at the moment.

Feel free to contribute by reporting issues, opening pull requests or by financially funding the development on [Patreon](https://www.patreon.com/kitframework)!

<p align="right"><a href="https://www.patreon.com/kitframework"><img src="https://cloud.githubusercontent.com/assets/8225057/5990484/70413560-a9ab-11e4-8942-1a63607c0b00.png"></a></p>


![Lighting model preview](https://raw.githubusercontent.com/haikarainen/kit/master/docs/metalball-crop.png)
<p align="center">Showcasing the lighting model, highly inspired from UE4.</p>

# Dependencies

Kit depends on the following libraries:

* GLFW3 (Window and input management)
* GLM (Math)
* Freetype 2 (Text rendering)
* Chaiscript (Scripting)
* OpenGL (Rendering)

Additionally, the asset importer depends on libassimp (and Qt5 on GNU/Linux distributions)

# Building 

## Linux

You build both the library as well as the tools using plain ol' makefiles.

### The library

The makefile for the library exists in the root folder. Do something like:

`make -j 10 && sudo make install`

Tip: Use -j 10 to parallelize the object compilation, which makes it compile much faster.

### The tools

The tools reside inside `./tools/<toolname>/`. Build it by calling `make` as usual. Installing it will put the binaries in the `dist` folder.

## Windows

Check the `vs2015` folder for a solution. It is known to be buildable, every dependency except `chaiscript` and `assimp` exist in NuGet. Please report any issues you might have using this solution.

# How to contribute

Open issues, create pull requests. But please get to know the codebase a bit first, learn the programming idioms. Even if a lot of code in this project currently is pretty ugly, I still require a very high standard of code-quality.

# Funding

This project takes up all of my spare time at the moment, and while I love doing it, it does not pay my bills or feed my kids. If you want me to be able to give more of my time to this project, you can help by funding the development on  [Patreon](https://www.patreon.com/kitframework).

Please also consider pledging to [ImGui](https://www.patreon.com/imgui), which is a dependency of Kit (mainly for the tools). It has helped quite a lot in terms of debugging as well.

Remember, every little bit helps!

[![Kit on Patreon](https://cloud.githubusercontent.com/assets/8225057/5990484/70413560-a9ab-11e4-8942-1a63607c0b00.png)](https://www.patreon.com/kitframework)
