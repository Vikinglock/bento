# bento

the bento (ベント) game engine made by yours truly (me)

**currently bento is very bare bones so don't mind that it's missing lot of features that a regular game engine'd have**

free, forever

(this project uses the MIT license so just include that in your project)

though obviously in the bento/lib/ folder there are a bunch of files and libraries that are not in fact mine (these include glfw, khr, dr, bullet physics engine, glad, stb image, glm, hidapi, openal soft, dear imgui(best), and more)

## about

bento is a powerful game engine for C++ that handles all of the graphical hurdles that is presented when coding very low level graphics swiftly and with great elegance

## roadmap

edit uniforms

deferred rendering / ssao

compile to apps

webgl (expect in a month)(technically also windows)

windows (a while (months) or less if someone lets me borrow their windows)

vulkan (expect like a week or 2 (or 3 years) after windows comes out)

## anything i put here sounds really corporate so you get a large body of large text

### easy to use

even though bento is written in C++ (best language) which is often regarded as the hardest language to learn, bento wraps all the complicated and sometimes convoluted code into neat little functions.

for example, creating a window is as easy as running
```
Bento *bento = new Bento();
bento->init("TITLE",width,height);
```

by the way c++ is easy idk what they're talking about

### open source

bento is open source which means that anyone on the interwebs can view this code, download it, and modify it to their heart's content. (although you're on github so you probably know what this means)

because of this, bento will remain free forever (although any games i or anyone else makes on it will NOT be free (i mean, they can be but it's not a requirement))

also any games made on this engine will NEVER need to pay extra fees because it's on the **MIT LICENSE** (just include it in your project for protection, though i'm probably not gonna sue if i see you use this engine either way, i'd just be proud that someone was able to use this incomprehensible mess)

### lightweight

bento is currently only about 80MB with most of it being other libraries like the bullet physics engine or dear imgui

also the example runs at 60+ fps only taking 52-150 MB ram on my computer (opengl takes the same but at a lower fps)

### cross-platform

bento is (mostly) CROSS-PLATFORM!!!!!

bento was written in opengl **and** metal which means that it's squeeze the most performance it can get from any device it's built on (with webgl and vulkan support coming soon)

also this means that if a game that runs on bento is on a pc platform and is not on another pc platform, then the developer can easily build it for that other platform (it's a bit harder from pc to mobile or console mainly because of the controls and performance)


The current supported platforms are:
- windows
- mac (native)
- linux

with support coming soon for:
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

WINDOWS IS SO HARD TO WORK WITH AAAAAAAAAAAAA

run.bat <output> <ONLY ONE INPUT>

that's it.
