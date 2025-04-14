# bento

bento (ベント) game engine by me :)

**currently bento does not in fact have a ui so familiarize yourself with a text editor or run as fast as you can**

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

### lightweight

bento is less than 200MB

also the example runs at >70 fps only taking 160-180 MB ram on my computer

### cross-platform

not gonna lie it's not very cross platform right now

it's a propietary engine so i can do what i want with it

it builds for mac and windows and probably linux too with little hassle but it basically only compiles when using mac

## include


all you gotta do is put this in the top of your code

```
#include "bento/bento.h"
```


## building
### MACOS

for the top two you can add -convert to convert the shaders

```
sh run.sh -metal <output> <input1> <input2> ...
```
OR
```
sh run.sh -opengl <output> <input1> <input2> ...
```
OR
```
sh run.sh -windows <output> <input1> <input2> ...
```

warning: these are untested and probably don't work


### LINUX
```
sh runlinux.sh <output> <input1> <input2> ...
```
### WINDOWS
```
run.bat <output> <ONLY ONE INPUT>
```



that's it.