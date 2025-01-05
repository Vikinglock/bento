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
layout(set = 0, binding = 4) uniform Unis {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 tpos;
};

vec3 viewDir;

float specular = 25.0;

vec3 calculateLighting(Light light) {
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(fragNormal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, fragNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), specular);
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
    vec3 finalColor = vec3(0.0);//vec3(dot(fragNormal,vec3(sin(t),cos(t),0)))*0.2;
    viewDir = normalize(pos - fragPos);

    Light light;
    
    light.position = vec3(1,5,0);
    light.ambient = vec3(1,1,1);
    light.diffuse = vec3(1,0,0);
    light.specular = vec3(1,0.6,0.6);
    light.constant = 0.1;
    light.linear = 0.8;
    light.quadratic = 0.01;

    finalColor += calculateLighting(light);
    
    vec3 textureColor = texture(tex, fragUV).rgb;//-(length(pos-fragPos)/5.0)) + (finalColor * textureColor)
    fragColor = vec4(max(finalColor * textureColor,0.1), 1.0);
}



//used to be shader.fs (which is a lot nicer) but for some reason glslangvalidator doesn't let you do that
//and it has to be version 450 rip