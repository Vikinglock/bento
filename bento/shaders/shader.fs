#version 330 core
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 tpos;
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
float specular;
vec3 viewDir;
vec3 calculateLighting(Light light)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(fragNormal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, fragNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), specular);
    float _distance = length(light.position - fragPos);
    float attenuation = 1.0 / ((light.constant + (light.linear * _distance)) + (light.quadratic * (_distance * _distance)));
    vec3 text = vec3(texture(tex, fragUV).xyz);
    vec3 ambient = light.ambient * text;
    vec3 diffuse = (light.diffuse * diff) * text;
    vec3 specular_1 = (light.specular * spec) * text;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular_1 *= attenuation;
    return (ambient + diffuse) + specular_1;
}
void main()
{
    specular = 25.0;
    float t = 0.4000000059604644775390625;
    vec3 finalColor = vec3(0.0);
    viewDir = normalize(pos - fragPos);
    Light light;
    light.position = vec3(1.0, 5.0, 0.0);
    light.ambient = vec3(1.0);
    light.diffuse = vec3(1.0, 0.0, 0.0);
    light.specular = vec3(1.0, 0.60000002384185791015625, 0.60000002384185791015625);
    light.constant = 0.100000001490116119384765625;
    light.linear = 0.800000011920928955078125;
    light.quadratic = 0.00999999977648258209228515625;
    Light param = light;
    finalColor += calculateLighting(param);
    vec3 textureColor = texture(tex, fragUV).xyz;
    fragColor = vec4(max(finalColor * textureColor, vec3(0.100000001490116119384765625)), 1.0);
}