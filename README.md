# ベント

the bento (ベント) game engine made by yours truly

currently private because nothing works on linux or windows (or at all)

## about

ベント is a cross-platform game engine for C++ that handles all of the graphical hurdles that is presented when coding very low level graphics swiftly and with great elegance

The current supported platforms are:
- mac (native)
- windows
- linux

with support for iphone, android, and various vr devices coming soon

ベント uses metal and opengl making it run very fast and making it work on ton of operating systems

because of this, the same code that works in windows can also be reused for mac or linux and vice versa

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

to build, open the terminal and run this

### MACOS

```
sh run.sh -metal -mac
```
OR
```
sh run.sh -opengl -mac
```

### LINUX

```
sh run.sh -opengl -linux
```

**NOTE: metal is not compatible with linux or windows**

### WINDOWS

```
run.bat
```
