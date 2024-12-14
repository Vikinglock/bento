#!/bin/sh

if [ "$#" -lt 2 ]; then
    echo "Usage: $0 (-metal|-opengl) <output_name> <c++ file> [another c++ file ...]"
    exit 1
fi

MODE=$1
OUTPUT=$2
shift 2

if [ "$MODE" = "-metal" ]; then
    clang++ "$@" -x objective-c++ metal.mm metaltexture.mm -Wno-null-character -std=c++17 -framework Metal -framework QuartzCore -framework Cocoa -framework IOKit -framework MetalKit -framework CoreVideo -framework Foundation -framework CoreFoundation -framework Carbon -o "$OUTPUT" -DUSE_METAL || exit 1 
elif [ "$MODE" = "-opengl" ]; then
    clang -c glad.c -o glad.o || exit 1
    clang++ "$@" opengl.cpp opengltexture.cpp glad.o -std=c++17 -framework OpenGL -framework QuartzCore -framework Cocoa -framework IOKit -framework CoreVideo -framework CoreFoundation -framework Carbon -lglfw3 -o "$OUTPUT" -DUSE_OPENGL || exit 1
else
    echo "Usage: $0 (-metal|-opengl) <output_name> <file1.cpp> [file2.cpp ...]"
    exit 1
fi

./"$OUTPUT"