#version 330 core
uniform sampler2D tex;
const float _100[64] = float[](0.0, 0.5, 0.125, 0.625, 0.03125, 0.53125, 0.15625, 0.65625, 0.75, 0.25, 0.875, 0.375, 0.78125, 0.28125, 0.90625, 0.40625, 0.1875, 0.6875, 0.0625, 0.5625, 0.21875, 0.71875, 0.09375, 0.59375, 0.9375, 0.4375, 0.8125, 0.3125, 0.96875, 0.46875, 0.84375, 0.34375, 0.046875, 0.546875, 0.171875, 0.671875, 0.015625, 0.515625, 0.140625, 0.640625, 0.796875, 0.296875, 0.921875, 0.421875, 0.765625, 0.265625, 0.890625, 0.390625, 0.234375, 0.734375, 0.109375, 0.609375, 0.203125, 0.703125, 0.078125, 0.578125, 0.984375, 0.484375, 0.859375, 0.359375, 0.953125, 0.453125, 0.828125, 0.328125);
in vec2 fragUV;
layout(location = 1) out vec4 fragColor;
layout(location = 2) out vec4 fragNormalOut;
in vec3 fragNormal;
layout(location = 0) out float depth;
in vec4 fragUPos;
layout(location = 3) out vec4 fragPosOut;
in vec3 fragPos;
float dither(vec2 p, float a)
{
    int index = (int(mod(p.y, 8.0)) * 8) + int(mod(p.x, 8.0));
    float threshold = _100[index];
    return float(a > threshold);
}
void main()
{
    vec4 textureColor = texture(tex, fragUV);
    vec2 param = gl_FragCoord.xy;
    float param_1 = textureColor.w;
    if (dither(param, param_1) < 0.5)
    {
        discard;
    }
    fragColor = textureColor;
    fragNormalOut = vec4((fragNormal / vec3(2.0)) + vec3(0.5), 1.0);
    depth = fragUPos.z;
    fragPosOut = vec4(fragPos, 1.0);
}