#version 450

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

void main() {
    float t = 0.4;
    vec3 finalColor = vec3(0.1)+ambientColor;//vec3(dot(fragNorm,vec3(sin(t),cos(t),0)))*0.2;
    fragNorm = normalize(texture(nortex,fUv).xyz * 2.0 - 1.0);//HAS TO BE IN ORDER
    float fragDepth = texture(depthtex,fUv).x;
    fragPos = texture(postex,fUv).xyz;
    vec3 dif = texture(diftex,fUv).xyz;

    viewDir = normalize(pos - fragPos);
    

    for(int i = 0; i < numLights; i++) {
        Light light;
        light.position = positions[i];
        light.ambient = ambients[i];
        light.diffuse = diffuses[i];
        light.specular = speculars[i];
        light.constant = constants[i];
        light.linear = linears[i];
        light.quadratic = quadratics[i];

        finalColor += calculateLighting(light);
    }

    fragColor = vec4(finalColor*texture(diftex,fUv).xyz, 1.0);
    depth = 0.0;
}