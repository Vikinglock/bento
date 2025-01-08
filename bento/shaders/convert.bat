::I HATE WINDOWS I HATE WINDOWS I HATE WINDOWS I HATE WINDOWS

@echo off

:usage
echo Usage:
echo   %~nx0 ^<vertexShader^> ^<fragmentShader^> --metal ^<vertexShaderOut^> ^<fragmentShaderOut^>
echo   %~nx0 ^<vertexShader^> ^<fragmentShader^> --opengl330 ^<vertexShaderOut^> ^<fragmentShaderOut^>
echo   %~nx0 ^<vertexShader^> ^<fragmentShader^> --opengl330c ^<vertexShaderOut^> ^<fragmentShaderOut^>
echo   %~nx0 ^<vertexShader^> ^<fragmentShader^> --vulkan ^<outputFile^>
echo   %~nx0 ^<vertexShader^> ^<fragmentShader^> --hlsl ^<vertexShaderOut^> ^<fragmentShaderOut^>
echo   %~nx0 ^<vertexShader^> ^<fragmentShader^> --spirv ^<vertexShaderOut^> ^<fragmentShaderOut^>
exit /b 1

if "%~4"=="" goto usage
set vertShader=%1
set fragShader=%2
set mode=%3
shift
shift
shift

if "%mode%"=="--metal" (
    if "%~2"=="" goto usage
    glslangValidator -V --quiet %vertShader% -o "vert.spv" || exit /b 1
    glslangValidator -V --quiet %fragShader% -o "frag.spv" || exit /b 1
    spirv-cross "vert.spv" --msl --output %1 || exit /b 1
    spirv-cross "frag.spv" --msl --output %2 || exit /b 1
    bento\shaders\bindfix.exe %vertShader% %1 || (
        clang++ bento\shaders\bindingsfix.cpp -o bento\shaders\bindfix.exe && ^
        bento\shaders\bindfix.exe %vertShader% %1
    )
    del "vert.spv" "frag.spv"
) else if "%mode%"=="--opengl330" (
    if "%~2"=="" goto usage
    glslangValidator -V --quiet %vertShader% -o "vert.spv" || exit /b 1
    glslangValidator -V --quiet %fragShader% -o "frag.spv" || exit /b 1
    spirv-cross "vert.spv" --version 330 --output %1 || exit /b 1
    spirv-cross "frag.spv" --version 330 --output %2 || exit /b 1
    del "vert.spv" "frag.spv"
) else if "%mode%"=="--opengl330c" (
    if "%~2"=="" goto usage
    glslangValidator -V --quiet %vertShader% -o "vert.spv" || exit /b 1
    glslangValidator -V --quiet %fragShader% -o "frag.spv" || exit /b 1
    spirv-cross "vert.spv" --version 330 --output %1 || exit /b 1
    spirv-cross "frag.spv" --version 330 --output %2 || exit /b 1
    bento\shaders\330c.exe %1 %2 || (
        clang++ bento\shaders\330to330core.cpp -std=c++11 -o bento\shaders\330c.exe && ^
        bento\shaders\330c.exe %1 %2
    )
    del "vert.spv" "frag.spv"
) else if "%mode%"=="--vulkan" (
    if "%~1"=="" goto usage
    glslangValidator -V --quiet %vertShader% -o "vert.spv" || exit /b 1
    glslangValidator -V --quiet %fragShader% -o "frag.spv" || exit /b 1
    copy /b "vert.spv"+"frag.spv" %1 > nul
    del "vert.spv" "frag.spv"
) else if "%mode%"=="--hlsl" (
    if "%~2"=="" goto usage
    glslangValidator -V --quiet %vertShader% -o "vert.spv" || exit /b 1
    glslangValidator -V --quiet %fragShader% -o "frag.spv" || exit /b 1
    spirv-cross "vert.spv" --hlsl --output %1 || exit /b 1
    spirv-cross "frag.spv" --hlsl --output %2 || exit /b 1
    del "vert.spv" "frag.spv"
) else if "%mode%"=="--spirv" (
    if "%~2"=="" goto usage
    glslangValidator -V --quiet %vertShader% -o "vert.spv" || exit /b 1
    glslangValidator -V --quiet %fragShader% -o "frag.spv" || exit /b 1
    copy "vert.spv" %1 > nul
    copy "frag.spv" %2 > nul
    del "vert.spv" "frag.spv"
) else (
    goto usage
)