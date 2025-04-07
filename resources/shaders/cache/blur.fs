#version 330 core
uniform int quality;
uniform int directions;
uniform vec2 blurAmount;
uniform sampler2D diftex;
in vec2 fUv;
layout(location = 1) out vec4 fragColor;
layout(location = 0) out float depth;
void main()
{
    vec2 uv = vec2(fUv.x, 1.0 - fUv.y);
    vec4 color = texture(diftex, uv);
    bool _42 = quality > 0;
    bool _49;
    if (_42)
    {
        _49 = directions > 0;
    }
    else
    {
        _49 = _42;
    }
    if (_49)
    {
        for (float d = 0.0; d < 6.283185482025146484375; d += (6.283185482025146484375 / float(directions)))
        {
            float _67 = 1.0 / float(quality);
            for (float i = _67; i <= 1.0; i += (1.0 / float(quality)))
            {
                vec2 sampPos = uv + ((vec2(cos(d), sin(d)) * blurAmount) * i);
                if (sampPos.x > 0.999000012874603271484375)
                {
                    sampPos.x = 0.999000012874603271484375;
                }
                else
                {
                    if (sampPos.x <= 0.0)
                    {
                        sampPos.x = 0.0;
                    }
                }
                if (sampPos.y > 0.999000012874603271484375)
                {
                    sampPos.y = 0.999000012874603271484375;
                }
                else
                {
                    if (sampPos.y <= 0.0)
                    {
                        sampPos.y = 0.0;
                    }
                }
                color += texture(diftex, sampPos);
            }
        }
    }
    color /= vec4((float(quality) * float(directions)) + 1.0);
    fragColor = color;
    depth = 0.0;
}