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
if [ "$fw" = "-metal" ]; then
    if [ "$output" = "-convert" ]; then
        echo "(converting shaders to msl)"
        sh bento/shaders/convert.sh bento/shaders/shader.vert bento/shaders/shader.frag --metal bento/shaders/shader.vsmetal bento/shaders/shader.fsmetal || exit 1
        #note .fsmetal is not the standard i just couldn't be bothered to search what it should be
        output=$1
        shift
    fi

    clang++ "$@" -x objective-c++ -o "$output" bento/metal/metal.mm bento/metal/metaltexture.mm bento/lib/hidapi/mac/hid.c -std=c++17\
                    bento/lib/imgui/imgui.cpp bento/lib/imgui/imgui_demo.cpp bento/lib/imgui/imgui_draw.cpp bento/lib/imgui/imgui_tables.cpp bento/lib/imgui/imgui_widgets.cpp bento/lib/imgui/backends/imgui_impl_metal.mm\
                    -framework Metal -framework QuartzCore -framework Cocoa -framework IOKit -framework MetalKit -framework CoreVideo -framework Foundation -framework CoreFoundation -framework Carbon -framework OpenAL -Wno-null-character -std=c++17 -DUSE_METAL -DMACOS || exit 1 
elif [ "$fw" = "-opengl" ]; then

    if [ "$output" = "-convert" ]; then
        echo "(converting shaders to 'glsl 330')"
        sh bento/shaders/convert.sh bento/shaders/shader.vert bento/shaders/shader.frag --opengl330 bento/shaders/shader.vs bento/shaders/shader.fs || exit 1
        #i don't think this'll even work
        output=$1
        shift
    elif [ "$output" = "-convertcore" ]; then
        echo "(converting shaders to glsl 330 core)"
        sh bento/shaders/convert.sh bento/shaders/shader.vert bento/shaders/shader.frag --opengl330c bento/shaders/shader.vs bento/shaders/shader.fs || exit 1
        output=$1
        shift
    fi

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