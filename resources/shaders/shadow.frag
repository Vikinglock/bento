#version 450

layout(location = 0) out float depth;
layout(location = 1) out vec4 fragColor;

layout(location = 0) in vec2 fUv;

layout(set = 0, binding = 0) uniform sampler2D posTex;
layout(set = 0, binding = 1) uniform sampler2D depthMap;

layout(set = 0, binding = 7) uniform Uniforms {
    mat4 lightProjection;
};

float ShadowCalculation(vec4 pos){
    vec3 projCoords = pos.xyz / pos.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(depthMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    float bias = 0.002;
    float shadow = 0.0;
    float texelSize = 1.0 / textureSize(depthMap, 0).x;
    for (int x = -1; x <= 1; ++x){
        for (int y = -1; y <= 1; ++y){
            float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    return shadow;
}

void main() {
    vec2 uv = vec2(fUv.x,1.0-fUv.y);
    vec3 fragPosWorld = texture(posTex, uv).xyz;
    vec4 fragPosLightSpace = lightProjection * vec4(fragPosWorld, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    projCoords.y = 1-projCoords.y;
    float closestDepth = texture(depthMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth ? 0.0 : 1.0;

    //float shadow = ShadowCalculation(fragPos);
    fragColor = vec4(shadow,shadow,shadow,1.0);
    depth = 0.0;
}