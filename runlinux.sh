#!/bin/sh

#idk how else to do it
if command -v apt >/dev/null; then
    sudo apt update && sudo apt install -y libopenal-dev
elif command -v dnf >/dev/null; then
    sudo dnf install -y openal-soft-devel
elif command -v pacman >/dev/null; then
    sudo pacman -S openal
elif command -v zypper >/dev/null; then
    sudo zypper install openal-soft-devel
fi

output=$1
shift

clang -c bento/glad.c -o glad.o || exit 1

clang++ "$@" bento/opengl/opengl.cpp bento/opengl/opengltexture.cpp glad.o\
                bento/lib/imgui/imgui.cpp bento/lib/imgui/imgui_demo.cpp bento/lib/imgui/imgui_draw.cpp bento/lib/imgui/imgui_tables.cpp bento/lib/imgui/imgui_widgets.cpp bento/lib/imgui/backends/imgui_impl_opengl3.cpp bento/lib/imgui/backends/imgui_impl_glfw.cpp\
                -std=c++17 -lglfw -ldl -lGL -lX11 -lXrandr -lpthread -lopenal -o "$output" -DUSE_OPENGL -DLINUX || exit 1
rm glad.o


./"$OUTPUT"

#i'll fix this when i get back to testing on linux