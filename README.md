# bento

bento (ベント) game engine by me :)

**currently bento is very bare bones so don't mind that it's missing lot of features that a regular game engine'd have**

this project uses the MIT license so it's gonna be free so just include that in your project

though in the bento/lib/ folder there are a bunch of files and libraries that are not in fact mine (these include glfw, khr, dr, bullet physics engine, glad, stb image, glm, hidapi, openal soft, dear imgui (the best), and more)

## roadmap

compile to apps

webgl (expect ~~in a month~~ NEVER HAHAHAHAHHAHAHAHA)(i'll do it later with emscripten)

vulkan (expect like 4 months or 2 (or 3) years)

## anything i put here sounds really corporate so here's a moderately sized body of large text

### easy to use

people are lying to you

C++ is easy and on top of that bento abstracts all the convoluted parts of other graphics libraries

for example, creating a window is as easy as running
```
Bento *bento = new Bento();
bento->init("TITLE",width,height);
```

### open source

bento is open source, meaning anyone on the interwebs can view this code, download it, and modify it to their heart's content. (although you're on github so you probably know what this means)

### lightweight

bento is less than 130MB

also the example runs at >70 fps only taking 160-180 MB ram on my computer

### cross-platform

bento is (mostly) CROSS-PLATFORM!!!!!

bento was written in opengl **and** metal

The current supported platforms are:
- windows
- mac
- linux

with support coming soon (in a while) for:
- iphone
- android
- consoles (nintendo, sony, microsoft)
- pcvr
- standalone vr (quest, pico)

i'll try adding support for older gen consoles too (:

## include


all you gotta do is put this in the top of your code

```
#include "bento/bento.h"
```


## building
### MACOS
```
sh run.sh -metal <output> <input1> <input2> ...
```
OR
```
sh run.sh -opengl <output> <input1> <input2> ...
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