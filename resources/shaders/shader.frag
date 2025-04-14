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

const float matrix[64] = float[](
    0.0/64.0, 32.0/64.0, 8.0/64.0, 40.0/64.0, 2.0/64.0, 34.0/64.0, 10.0/64.0, 42.0/64.0,
    48.0/64.0, 16.0/64.0, 56.0/64.0, 24.0/64.0, 50.0/64.0, 18.0/64.0, 58.0/64.0, 26.0/64.0,
    12.0/64.0, 44.0/64.0, 4.0/64.0, 36.0/64.0, 14.0/64.0, 46.0/64.0, 6.0/64.0, 38.0/64.0,
    60.0/64.0, 28.0/64.0, 52.0/64.0, 20.0/64.0, 62.0/64.0, 30.0/64.0, 54.0/64.0, 22.0/64.0,
    3.0/64.0, 35.0/64.0, 11.0/64.0, 43.0/64.0, 1.0/64.0, 33.0/64.0, 9.0/64.0, 41.0/64.0,
    51.0/64.0, 19.0/64.0, 59.0/64.0, 27.0/64.0, 49.0/64.0, 17.0/64.0, 57.0/64.0, 25.0/64.0,
    15.0/64.0, 47.0/64.0, 7.0/64.0, 39.0/64.0, 13.0/64.0, 45.0/64.0, 5.0/64.0, 37.0/64.0,
    63.0/64.0, 31.0/64.0, 55.0/64.0, 23.0/64.0, 61.0/64.0, 29.0/64.0, 53.0/64.0, 21.0/64.0
);

float dither(vec2 p,float a) {
    int index = int(mod(p.y,8.0))*8+int(mod(p.x,8.0));//this is just like when i was forced to load textures through the shader so i did this
    float threshold = matrix[index];
    return (a > threshold) ? 1.0 : 0.0;
}

void main() {
    // vec3 finalColor = ambientColor;//vec3(dot(fragNormal,vec3(sin(t),cos(t),0)))*0.2;

    vec4 textureColor = texture(tex, fragUV);//-(length(pos-fragPos)/5.0)) + (finalColor * textureColor)

    if(dither(gl_FragCoord.xy,textureColor.a)<0.5)discard;//if you want blending get rid of this line but good luck

    fragColor = textureColor;//*finalColor
    fragNormalOut = vec4(fragNormal / 2.0 + 0.5,1.0);

    depth = fragUPos.z;
    fragPosOut = vec4(fragPos,1.0);
}



//used to be shader.fs (which is a lot nicer) but for some reason glslangvalidator doesn't let you do that
//and it has to be version 450 rip