#version 450

//DO NOT BE FOOLED
//THIS IS LIGHTING

layout(location = 0) out float depth;
layout(location = 1) out vec4 fragColor;
layout(location = 0) in vec2 fUv;

layout(set = 0, binding = 0) uniform sampler2D diftex;
layout(set = 0, binding = 1) uniform sampler2D nortex;
layout(set = 0, binding = 2) uniform sampler2D depthtex;
layout(set = 0, binding = 3) uniform sampler2D postex;

#define MAX_LIGHTS 50
layout(set = 0, binding = 4) uniform Uniforms {
    vec3 pos;
    int numLights;

    vec3 ambientColor;
    
    vec3 positions[MAX_LIGHTS];
    
    float constants[MAX_LIGHTS];
    float linears[MAX_LIGHTS];
    float quadratics[MAX_LIGHTS];
	
    vec3 ambients[MAX_LIGHTS];
    vec3 diffuses[MAX_LIGHTS];
    vec3 speculars[MAX_LIGHTS];
    float tspecular;
};

struct Light {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

vec3 viewDir;
vec3 fragPos;
vec3 fragNorm;


vec3 calculateLighting(Light light) {
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(fragNorm, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, fragNorm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), tspecular);
    float distance = length(light.position  - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    vec3 text = vec3(texture(diftex, fUv));
    vec3 ambient = light.ambient * text;
    vec3 diffuse = light.diffuse * diff * text;
    vec3 specular = light.specular * spec * text;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

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
    int index = int(mod(p.y,8.0))*8+int(mod(p.x,8.0));
    float threshold = matrix[index];
    return (a > threshold) ? 1.0 : 0.0;
}

void main() {
    float t = 0.4;
    vec3 finalColor = vec3(0.1)+ambientColor;//vec3(dot(fragNorm,vec3(sin(t),cos(t),0)))*0.2;
    fragNorm = normalize(texture(nortex,fUv).xyz * 2.0 - 1.0);//the texture()s have to be used in the same order the bindings are set ^^ up there
    float fragDepth = texture(depthtex,fUv).x;
    fragPos = texture(postex,fUv).xyz;
    vec3 dif = texture(diftex,fUv).xyz;

    viewDir = normalize(pos - fragPos);
    

    if(fragDepth > 0.0)
    for(int i = 0; i < numLights; i++) {
        Light light;
        light.position = positions[i];
        light.ambient = ambients[i];
        light.diffuse = diffuses[i];
        light.specular = speculars[i];
        light.constant = constants[i];
        light.linear = linears[i];
        light.quadratic = quadratics[i];

        vec3 color = calculateLighting(light);;

        if(dither(gl_FragCoord.xy,(color.x+color.y+color.z)/3.0)>0.5)finalColor += color;
    }

    fragColor = vec4(finalColor*texture(diftex,fUv).xyz, 1.0);
    depth = 0.0;
}