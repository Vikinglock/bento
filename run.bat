@echo off

if "%~1" == "" (
    echo Usage: run.bat <output_name> <c++ file> [another c++ file ...]
    exit /b 1
)

set OUTPUT=%1
shift /1

clang++ %* glad.c opengl.cpp -I./lib -L./lib/GLFW/x64 -lglfw3 -lopengl32 -luser32 -lgdi32 -lole32 -loleaut32 -luuid -lshell32 -lkernel32 -lmsvcrt -o %OUTPUT% -DUSE_OPENGL -Wno-deprecated

if %errorlevel% equ 0 (
    echo Build succeeded.
    %OUTPUT%
) else (
    echo Build failed.
)