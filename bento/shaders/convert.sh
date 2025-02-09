#!/bin/bash

usage() {
    echo "Usage:"
    echo "  $0 <vertexShader> <fragmentShader> --metal <vertexShader> <fragmentShader>"
    echo "  $0 <vertexShader> <fragmentShader> --opengl330 <vertexShader> <fragmentShader>"
    echo "  $0 <vertexShader> <fragmentShader> --opengl330c <vertexShader> <fragmentShader>"
    echo "  $0 <vertexShader> <fragmentShader> --vulkan <shader>"
    echo "  $0 <vertexShader> <fragmentShader> --hlsl <vertexShader> <fragmentShader>"
    echo "  $0 <vertexShader> <fragmentShader> --spirv <vertexShader> <fragmentShader>"
    exit 1
}

if [ "$#" -lt 4 ]; then
    usage
fi

case "$3" in
    --metal)
        if [ "$#" -ne 5 ]; then usage; fi
        glslangValidator -V --quiet $1 -o "vert.spv" || exit 1
        glslangValidator -V --quiet $2 -o "frag.spv" || exit 1
        spirv-cross "vert.spv" --msl --output $4 || exit 1
        spirv-cross "frag.spv" --msl --output $5 || exit 1
        ./bento/shaders/bindfix $1 $4 || (clang++ bento/shaders/bindingsfix.cpp -o bento/shaders/bindfix ; ./bento/shaders/bindfix $1 $4)
        rm "vert.spv" "frag.spv"
        ;;
    --opengl330)
        if [ "$#" -ne 5 ]; then usage; fi
        glslangValidator -V --quiet $1 -o "vert.spv" || exit 1
        glslangValidator -V --quiet $2 -o "frag.spv" || exit 1
        spirv-cross "vert.spv" --version 330 --output $4 || exit 1
        spirv-cross "frag.spv" --version 330 --output $5 || exit 1
        rm "vert.spv" "frag.spv"
        ;;
    --opengl330c)
        if [ "$#" -ne 5 ]; then usage; fi
        glslangValidator -V --quiet $1 -o "vert.spv" || exit 1
        glslangValidator -V --quiet $2 -o "frag.spv" || exit 1
        spirv-cross "vert.spv" --version 330 --output $4 || exit 1
        spirv-cross "frag.spv" --version 330 --output $5 || exit 1
        ./bento/shaders/330c $4 $5 || (clang++ bento/shaders/330to330core.cpp -std=c++11 -o bento/shaders/330c ; ./bento/shaders/330c $4 $5)
        ;;
    --vulkan)
        if [ "$#" -ne 4 ]; then usage; fi
        glslangValidator -V --quiet $1 -o "vert.spv" || exit 1
        glslangValidator -V --quiet $2 -o "frag.spv" || exit 1
        cat "vert.spv" "frag.spv" > $4
        rm "vert.spv" "frag.spv"
        ;;
    --hlsl)
        if [ "$#" -ne 5 ]; then usage; fi
        glslangValidator -V --quiet $1 -o "vert.spv" || exit 1
        glslangValidator -V --quiet $2 -o "frag.spv" || exit 1
        spirv-cross "vert.spv" --hlsl --output $4 || exit 1
        spirv-cross "frag.spv" --hlsl --output $5 || exit 1
        rm "vert.spv" "frag.spv"
        ;;
    --spirv)
        if [ "$#" -ne 5 ]; then usage; fi
        glslangValidator -V --quiet $1 -o "vert.spv" || exit 1
        glslangValidator -V --quiet $2 -o "frag.spv" || exit 1
        cp "vert.spv" $4
        cp "frag.spv" $5
        rm "vert.spv" "frag.spv"
        ;;
    *)
        usage
        ;;
esac