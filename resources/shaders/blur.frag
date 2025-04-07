#version 450
layout(location = 0) out float depth;
layout(location = 1) out vec4 fragColor;
layout(location = 0) in vec2 fUv;
layout(set = 0, binding = 0) uniform sampler2D diftex;
layout(set = 0, binding = 4) uniform Uniforms {
    int quality;
    int directions;
    vec2 blurAmount;
};//stolen from space endeavor lmao

const float tau = 6.28318530718;

void main() {
    vec2 uv = vec2(fUv.x,1.0-fUv.y);
    vec4 color = texture(diftex, uv);

    if(quality > 0 && directions > 0){
        for(float d=0.0; d<tau; d+=tau/float(directions)){
            for(float i=1.0/float(quality); i<=1.0; i+=1.0/float(quality)){
                vec2 sampPos = uv + vec2(cos(d), sin(d)) * blurAmount * i;
                if(sampPos.x > 0.999){sampPos.x = 0.999;}
                else if(sampPos.x <= 0.0){sampPos.x = 0.0;}
                if(sampPos.y > 0.999){sampPos.y = 0.999;}
                else if(sampPos.y <= 0.0){sampPos.y = 0.0;}
                color += texture(diftex, sampPos);
            }
        }
    }

    color /= float(quality) * float(directions) + 1.0;
    fragColor = color;
    depth = 0.0;
}