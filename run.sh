#!/bin/sh

if [ "$#" -lt 2 ]; then
    echo "Usage: $0 (-metal|-opengl) <output_name> <c++ file> <another c++ file> ..."
    exit 1
fi
fw=$1
output=$2
shift 2
#this is getting so long i HAVE to use the \s
#"oh but you weren't using it before?" shhhhhh

#also you have to have spirv-cross and glslangvalidator installed
#or don't have any shaders

if [ "$fw" = "-metal" ]; then
    clang++ "$@" -x objective-c++ -o "$output" bento/metal/metal.mm bento/metal/metaltexture.mm bento/lib/hidapi/mac/hid.c -std=c++17\
                    bento/lib/imgui/imgui.cpp bento/lib/imgui/imgui_demo.cpp bento/lib/imgui/imgui_draw.cpp bento/lib/imgui/imgui_tables.cpp bento/lib/imgui/imgui_widgets.cpp bento/lib/imgui/backends/imgui_impl_metal.mm\
                    -framework Metal -framework QuartzCore -framework Cocoa -framework IOKit -framework MetalKit -framework CoreVideo -framework Foundation -framework CoreFoundation -framework Carbon -framework OpenAL -Wno-null-character -std=c++17 -DUSE_METAL -DMACOS || exit 1 
elif [ "$fw" = "-opengl" ]; then
    clang -c bento/glad.c -o glad.o || exit 1
    clang++ "$@" bento/opengl/opengl.cpp bento/opengl/opengltexture.cpp glad.o\
                 bento/lib/imgui/imgui.cpp bento/lib/imgui/imgui_demo.cpp bento/lib/imgui/imgui_draw.cpp bento/lib/imgui/imgui_tables.cpp bento/lib/imgui/imgui_widgets.cpp bento/lib/imgui/backends/imgui_impl_opengl3.cpp bento/lib/imgui/backends/imgui_impl_glfw.cpp\
                 -std=c++17 -framework OpenGL -framework QuartzCore -framework Cocoa -framework IOKit -framework CoreVideo -framework CoreFoundation -framework Carbon -framework OpenAL -lglfw3 -o "$output" -DUSE_OPENGL -DMACOS || exit 1
    rm glad.o
else
    echo "Usage: $0 (-metal|-opengl) <output_name> <c++ file> <another c++ file> ..."
    exit 1
fi

codesign -s - -v -f --entitlements debug.plist "$output"

./"$output"