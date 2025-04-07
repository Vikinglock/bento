#version 450

layout(location = 0) out float depth;
layout(location = 1) out vec4 fragColor;
layout(location = 2) out vec4 fragNormalOut;
layout(location = 3) out vec4 fragPosOut;


layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec4 fragUPos;

layout(set = 0, binding = 0) uniform sampler2D tex;

//layout(set = 0, binding = 7) uniform Uniforms {};


void main() {
    // vec3 finalColor = ambientColor;//vec3(dot(fragNormal,vec3(sin(t),cos(t),0)))*0.2;

    vec3 textureColor = texture(tex, fragUV).rgb;//-(length(pos-fragPos)/5.0)) + (finalColor * textureColor)
    fragColor = vec4(textureColor, 1.0);//*finalColor
    fragNormalOut = vec4(fragNormal / 2.0 + 0.5,1.0);

    depth = fragUPos.z;
    fragPosOut = vec4(fragPos,1.0);
}



//used to be shader.fs (which is a lot nicer) but for some reason glslangvalidator doesn't let you do that
//and it has to be version 450 rip