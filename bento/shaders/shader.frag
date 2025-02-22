#version 450
#define MAX_LIGHTS 50

struct Light {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec3 viewPos;
layout(location = 4) in vec3 pos;

layout(set = 0, binding = 0) uniform sampler2D tex;

layout(set = 0, binding = 7) uniform Uniforms {
    int numLights;
    
    vec3 ambientColor;
    
    vec3 positions[MAX_LIGHTS];
    
    float constants[MAX_LIGHTS];
    float linears[MAX_LIGHTS];
    float quadratics[MAX_LIGHTS];
	
    vec3 ambients[MAX_LIGHTS];
    vec3 diffuses[MAX_LIGHTS];
    vec3 speculars[MAX_LIGHTS];
};

vec3 viewDir;

float tspecular = 100.0;

vec3 calculateLighting(Light light) {
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(fragNormal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, fragNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), tspecular);
    float distance = length(light.position  - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    vec3 text = vec3(texture(tex, fragUV));
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
    vec3 finalColor = ambientColor;//vec3(dot(fragNormal,vec3(sin(t),cos(t),0)))*0.2;

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

    vec3 textureColor = texture(tex, fragUV).rgb;//-(length(pos-fragPos)/5.0)) + (finalColor * textureColor)
    fragColor = vec4(finalColor * textureColor, 1.0);
}



//used to be shader.fs (which is a lot nicer) but for some reason glslangvalidator doesn't let you do that
//and it has to be version 450 rip