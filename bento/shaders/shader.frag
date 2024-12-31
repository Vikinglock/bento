#version 450

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec3 viewPos;
layout(location = 4) in vec3 viewDir;

layout(set = 0, binding = 0) uniform sampler2D tex;

void main() {
    fragColor =  vec4(texture(tex, fragUV).xyz*(dot(fragNormal,vec3(0,1,0))*0.5+0.5),1.0);
}

//used to be shader.fs (which is a lot nicer) but for some reason glslangvalidator doesn't let you do that
//and it has to be version 450 rip