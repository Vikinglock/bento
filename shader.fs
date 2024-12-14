#version 330 core
out vec4 fragColor;

in vec3 fragPos;
in vec3 fragNormal;
in vec2 fragUV;

uniform sampler2D tex;


void main() {
    fragColor = vec4(texture(tex, fragUV).xyz*(fragNormal.y*0.5+0.5),1.0);
}