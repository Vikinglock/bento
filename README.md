# bento

the bento (ベント) game engine made by yours truly (me)

free, forever

this project uses the MIT license so just include that in your project

though obviously in the bento/lib/ folder there are a bunch of files and libraries that are not in fact mine and the whole file structure has been modified

## about

bento is a powerful game engine for C++ that handles all of the graphical hurdles that is presented when coding very low level graphics swiftly and with great elegance

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

because of this, bento will remain free forever (although any games i or anyone else makes on it will NOT be free)

also any games made on this engine will NEVER need to pay extra fees because it's on the **MIT LICENSE** (just include it in your project for protection)

### lightweight

~~bento is currently only about 20MB with most of it being other libraries like the bullet physics engine and opengl~~

~~also the example runs at 60+ fps only taking 45 MB ram on my computer~~

holy christ remind me to optimize it at some point

### cross-platform

bento is CROSS-PLATFORM!!!!!

bento was written in opengl **and** metal which means that it's squeeze the most performance it can get from any device it's built on (with webgl and vulkan support coming soon)

also this means that if a game that runs on bento is on a pc platform and is not on another pc platform, then the developer can easily build it for that other platform (it's a bit harder from pc to mobile or console mainly because of the controls and performance)


The current supported platforms are:
- mac (native)
- windows
- linux

with support coming soon for:
- iphone
- android
- consoles (nintendo, sony, microsoft)
- pcvr
- standalone vr (quest, pico)

i'll try adding support for older gen consoles too (:


speaking of consoles and pc

built in controller support exists for windows linux and mac (on mac only the specific controller brand that i have works)

## include


all you gotta do is put this in the top of your code

```
#ifdef USE_METAL
#include "metal.h"
#elif USE_OPENGL
#include "opengl.h"
#endif
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
sh runlinux.sh -opengl <output> <input1> <input2> ...
```

### WINDOWS

```
run.bat <output> <input1> <input2> ...
```

that's it.
