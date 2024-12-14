#!/bin/sh

if [ "$#" -lt 2 ]; then
    echo "Usage: $0 <output_name> <c++ file> [another c++ file ...]"
    exit 1
fi

OUTPUT=$1
shift

clang -c glad.c -o glad.o || exit 1

clang++ "$@" opengl.cpp glad.o -lglfw -ldl -lGL -lX11 -lXrandr -lpthread -o "$OUTPUT" -DUSE_OPENGL || exit 1

./"$OUTPUT"