@echo off
if "*%~1*" == "" (
    echo Usage: run.bat ^<output_name^> ^<c++ file^> [another c++ file ...]
    exit /b 1
)

set output=%1

clang -c bento/glad.c -o glad.o
clang++ %2 glad.o bento/opengl/opengl.cpp bento/opengl/opengltexture.cpp -I./bento/lib -L./bento/lib/GLFW/x64 ^
    bento/lib/imgui/imgui.cpp ^
    bento/lib/imgui/imgui_demo.cpp ^
    bento/lib/imgui/imgui_draw.cpp ^
    bento/lib/imgui/imgui_tables.cpp ^
    bento/lib/imgui/imgui_widgets.cpp ^
    bento/lib/imgui/backends/imgui_impl_opengl3.cpp ^
    bento/lib/imgui/backends/imgui_impl_glfw.cpp ^
    -L./bento/lib/build/Release ^
    -I./bento/lib/AL -L./bento/lib/openal-soft/libs/Win64 -lOpenAL32 ^
    -lglfw3 -lopengl32 -luser32 -lgdi32 -lole32 -loleaut32 ^
    -luuid -lshell32 -lkernel32 -lmsvcrt ^
    -o "%output%" -DUSE_OPENGL -DWINDOWS -Wno-deprecated -D_CRT_SECURE_NO_WARNINGS
if %errorlevel% equ 0 (
    echo Build succeeded.
    %1
) else (
    echo Build failed.
)

del glad.o