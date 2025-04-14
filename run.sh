#!/bin/sh

if [ "$#" -lt 2 ]; then
    echo "Usage: $0 (-metal|-opengl) <output_name> <c++ file> <another c++ file> ..."
    exit 1
fi
fw=$1
output=$2
shift 2
#this is getting so long i HAVE to use the \ s
#"oh but you weren't using it before?" shhhhhh

#also you have to have spirv-cross and glslangvalidator installed
#or don't have any shaders

#i'll make a startup executable shell file to download stuff for maximum compatibility
#in cause you couldn't tell i care about compatibility

#see when i was a child i saw this cool game about titans who fell, right, and i wanted to play it but oh no would you look at that i have a macbook ): i can't play this game ))): now, i never want this to happen to anyone else and i would never wish this upon my worst enemy so now i'm making sure everybody can play my games, even the people who have decided not to upgrade their consoles since like 2004, so that no one is stuck watching videos of people playing my games while tears run down their eyes because they know they'll need a billion dollar gaming rig to play my games. this is also why i wouldn't do any raytracing only games, also because not even i can run that poopy garbage

if [ "$output" = "-convert" ]; then
    output=$1
    shift


    if [ "$fw" = "-metal" ]; then
        clang++ "$@" -x objective-c++ -o "$output" bento/metal/metal.mm bento/metal/metaltexture.mm bento/lib/hidapi/mac/hid.c -std=c++17\
                        bento/lib/imgui/imgui.cpp bento/lib/imgui/imgui_demo.cpp bento/lib/imgui/imgui_draw.cpp bento/lib/imgui/imgui_tables.cpp bento/lib/imgui/imgui_widgets.cpp bento/lib/imgui/backends/imgui_impl_metal.mm\
                        -framework Metal -framework QuartzCore -framework Cocoa -framework IOKit -framework MetalKit -framework CoreVideo -framework Foundation -framework CoreFoundation -framework Carbon -framework OpenAL -I./bento/lib/bullet-install/include -L./bento/lib/bullet-install/lib -lBulletDynamics -lBulletCollision -lLinearMath -Wno-null-character -std=c++17 -DUSE_METAL -DMACOS -DCONVERT || exit 1 
    elif [ "$fw" = "-opengl" ]; then
        clang -c bento/glad.c -o glad.o || exit 1
        clang++ "$@" bento/opengl/opengl.cpp bento/opengl/opengltexture.cpp glad.o\
                    bento/lib/imgui/imgui.cpp bento/lib/imgui/imgui_demo.cpp bento/lib/imgui/imgui_draw.cpp bento/lib/imgui/imgui_tables.cpp bento/lib/imgui/imgui_widgets.cpp bento/lib/imgui/backends/imgui_impl_opengl3.cpp bento/lib/imgui/backends/imgui_impl_glfw.cpp\
                    -std=c++17 -framework OpenGL -framework QuartzCore -framework Cocoa -framework IOKit -framework CoreVideo -framework CoreFoundation -framework Carbon -framework OpenAL -I./bento/lib/bullet-install/include -L./bento/lib/bullet-install/lib -lBulletDynamics -lBulletCollision -lLinearMath -lglfw3 -o "$output" -DUSE_OPENGL -DMACOS -DCONVERT || exit 1
        rm glad.o
    fi

elif [ "$fw" = "-metal" ]; then
    clang++ "$@" -x objective-c++ -o "$output" bento/metal/metal.mm bento/metal/metaltexture.mm bento/lib/hidapi/mac/hid.c -std=c++17\
                    bento/lib/imgui/imgui.cpp bento/lib/imgui/imgui_demo.cpp bento/lib/imgui/imgui_draw.cpp bento/lib/imgui/imgui_tables.cpp bento/lib/imgui/imgui_widgets.cpp bento/lib/imgui/backends/imgui_impl_metal.mm\
                    -framework Metal -framework QuartzCore -framework Cocoa -framework IOKit -framework MetalKit -framework CoreVideo -framework Foundation -framework CoreFoundation -framework Carbon -framework OpenAL -I./bento/lib/bullet-install/include -L./bento/lib/bullet-install/lib -lBulletDynamics -lBulletCollision -lLinearMath -Wno-null-character -std=c++17 -DUSE_METAL -DMACOS || exit 1 
elif [ "$fw" = "-opengl" ]; then
    clang -c bento/glad.c -o glad.o || exit 1
    clang++ "$@" bento/opengl/opengl.cpp bento/opengl/opengltexture.cpp glad.o\
                 bento/lib/imgui/imgui.cpp bento/lib/imgui/imgui_demo.cpp bento/lib/imgui/imgui_draw.cpp bento/lib/imgui/imgui_tables.cpp bento/lib/imgui/imgui_widgets.cpp bento/lib/imgui/backends/imgui_impl_opengl3.cpp bento/lib/imgui/backends/imgui_impl_glfw.cpp\
                 -std=c++17 -framework OpenGL -framework QuartzCore -framework Cocoa -framework IOKit -framework CoreVideo -framework CoreFoundation -framework Carbon -framework OpenAL -I./bento/lib/bullet-install/include -L./bento/lib/bullet-install/lib -lBulletDynamics -lBulletCollision -lLinearMath -lglfw3 -o "$output" -DUSE_OPENGL -DMACOS || exit 1
    rm glad.o
elif [ "$fw" = "-windows" ]; then
    x86_64-w64-mingw32-g++ "$@" bento/glad.c bento/opengl/opengl.cpp bento/opengl/opengltexture.cpp\
        bento/lib/imgui/imgui.cpp bento/lib/imgui/imgui_draw.cpp bento/lib/imgui/imgui_tables.cpp bento/lib/imgui/imgui_widgets.cpp bento/lib/imgui/imgui_demo.cpp bento/lib/imgui/backends/imgui_impl_glfw.cpp bento/lib/imgui/backends/imgui_impl_opengl3.cpp\
        -o "$output.exe" -DUSE_OPENGL -DWINDOWS -I./bento/lib/bullet-install/include -L./bento/lib/winbins -lglfw3 -lgdi32 -lopengl32 -lopenal32 -lBulletDynamics -lBulletCollision -lLinearMath -static -mwindows || exit 1
    x86_64-w64-mingw32-strip "$output.exe"
    #breaking news: mac devs no longer need a windows pc to compile windows games
    #just know for some reason file loading is broken on windows for some reason
else
    echo "Usage: $0 (-metal|-opengl|-windows) (-convert) <output_name> <c++ file> <another c++ file> ..."
    exit 1
fi

codesign -s - -v -f --entitlements debug.plist "$output"


if [ "$fw" != "-windows" ]; then
    ./"$output"
fi