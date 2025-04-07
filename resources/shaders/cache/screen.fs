#version 330 core
uniform vec3 pos;
uniform int numLights;
uniform vec3 ambientColor;
uniform vec3 positions[50];
uniform float constants[50];
uniform float linears[50];
uniform float quadratics[50];
uniform vec3 ambients[50];
uniform vec3 diffuses[50];
uniform vec3 speculars[50];
uniform float tspecular;
uniform sampler2D diftex;
uniform sampler2D nortex;
uniform sampler2D depthtex;
uniform sampler2D postex;
struct Light
{
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
in vec2 fUv;
layout(location = 1) out vec4 fragColor;
layout(location = 0) out float depth;
vec3 fragPos;
vec3 fragNorm;
vec3 viewDir;
vec3 calculateLighting(Light light)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(fragNorm, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, fragNorm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), tspecular);
    float _distance = length(light.position - fragPos);
    float attenuation = 1.0 / ((light.constant + (light.linear * _distance)) + (light.quadratic * (_distance * _distance)));
    vec3 text = vec3(texture(diftex, fUv).xyz);
    vec3 ambient = light.ambient * text;
    vec3 diffuse = (light.diffuse * diff) * text;
    vec3 specular = (light.specular * spec) * text;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse) + specular;
}
void main()
{
    float t = 0.4000000059604644775390625;
    vec3 finalColor = vec3(0.100000001490116119384765625) + ambientColor;
    fragNorm = normalize((texture(nortex, fUv).xyz * 2.0) - vec3(1.0));
    float fragDepth = texture(depthtex, fUv).x;
    fragPos = texture(postex, fUv).xyz;
    vec3 dif = texture(diftex, fUv).xyz;
    viewDir = normalize(pos - fragPos);
    Light light;
    for (int i = 0; i < numLights; i++)
    {
        light.position = positions[i];
        light.ambient = ambients[i];
        light.diffuse = diffuses[i];
        light.specular = speculars[i];
        light.constant = constants[i];
        light.linear = linears[i];
        light.quadratic = quadratics[i];
        Light param = light;
        finalColor += calculateLighting(param);
    }
    fragColor = vec4(finalColor * texture(diftex, fUv).xyz, 1.0);
    depth = 0.0;
}