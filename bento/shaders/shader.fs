#version 330 core
uniform int numLights;
uniform vec3 positions[50];
uniform float constants[50];
uniform float linears[50];
uniform float quadratics[50];
uniform vec3 ambients[50];
uniform vec3 diffuses[50];
uniform vec3 speculars[50];
uniform sampler2D tex;
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
in vec3 fragPos;
in vec3 fragNormal;
in vec2 fragUV;
in vec3 pos;
layout(location = 0) out vec4 fragColor;
in vec3 viewPos;
float tspecular;
vec3 viewDir;
vec3 calculateLighting(Light light)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(fragNormal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, fragNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), tspecular);
    float _distance = length(light.position - fragPos);
    float attenuation = 1.0 / ((light.constant + (light.linear * _distance)) + (light.quadratic * (_distance * _distance)));
    vec3 text = vec3(texture(tex, fragUV).xyz);
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
    tspecular = 100.0;
    float t = 0.4000000059604644775390625;
    vec3 finalColor = vec3(0.0);
    viewDir = normalize(pos - fragPos);
    Light light;
    light.position = vec3(1.0, 1.0, 0.0);
    light.ambient = vec3(1.0);
    light.diffuse = vec3(1.0, 0.0, 0.0);
    light.specular = vec3(1.0, 0.60000002384185791015625, 0.60000002384185791015625);
    light.constant = 0.100000001490116119384765625;
    light.linear = 0.800000011920928955078125;
    light.quadratic = 0.00999999977648258209228515625;
    Light lite;
    for (int i = 0; i < numLights; i++)
    {
        lite.position = positions[i];
        lite.ambient = ambients[i];
        lite.diffuse = diffuses[i];
        lite.specular = speculars[i];
        lite.constant = constants[i];
        lite.linear = linears[i];
        lite.quadratic = quadratics[i];
        Light param = lite;
        finalColor += calculateLighting(param);
    }
    vec3 textureColor = texture(tex, fragUV).xyz;
    fragColor = vec4(finalColor * textureColor, 1.0);
}