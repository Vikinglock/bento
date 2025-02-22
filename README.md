# bento

the bento (ベント) game engine made by yours truly (me)

**currently bento is very bare bones so don't mind that it's missing lot of features that a regular game engine'd have**

this project uses the MIT license so it's gonna be free so just include that in your project

though obviously in the bento/lib/ folder there are a bunch of files and libraries that are not in fact mine (these include glfw, khr, dr, bullet physics engine, glad, stb image, glm, hidapi, openal soft, dear imgui(best), and more)

## about

bento is a powerful game engine for C++ that handles all of the graphical hurdles that is presented when coding very low level graphics swiftly and with great elegance

## roadmap

edit uniforms

deferred rendering / ssao

compile to apps

webgl (expect ~~in a month~~ NEVER HAHAHAHAHHAHAHAHA)(i'll do it later with emscripten)

vulkan (expect like 4 months or 2 (or 3) years)

## anything i put here sounds really corporate so you get a large body of large text

### easy to use

even though bento is written in C++ (the best language) which is often regarded as the hardest language to learn, bento wraps all the complicated and sometimes convoluted code into neat little functions.

for example, creating a window is as easy as running
```
Bento *bento = new Bento();
bento->init("TITLE",width,height);
```

by the way c++ is easy idk what they're talking about

### open source

bento is open source, meaning anyone on the interwebs can view this code, download it, and modify it to their heart's content. (although you're on github so you probably know what this means)

### lightweight

bento is less than 100MB

also the example runs at 60+ fps only taking 52-150 MB ram on my computer (opengl takes the same but at a lower fps)

### cross-platform

bento is (mostly) CROSS-PLATFORM!!!!!

bento was written in opengl **and** metal

The current supported platforms are:
- windows
- mac (native)
- linux

with support coming soon (in a while) for:
- iphone
- android
- consoles (nintendo, sony, microsoft)
- pcvr
- standalone vr (quest, pico)

i'll try adding support for older gen consoles too (:)


speaking of consoles and pc

built in controller support exists for linux and mac (on metal only the specific controller brand that i have works)

## include


all you gotta do is put this in the top of your code

```
#include "bento/bento.h"
```


## building

**NOTE: metal is not compatible with linux or windows (obviously)**

to build, open the terminal and run this

### MACOS

```
sh run.sh -metal -convert <output> <input1> <input2> ...
```
OR
```
sh run.sh -opengl -convertcore <output> <input1> <input2> ...
```

### LINUX


```
sh runlinux.sh <output> <input1> <input2> ...
```

### WINDOWS

```
run.bat <output> <ONLY ONE INPUT>
```

that's it.
