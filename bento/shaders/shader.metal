#include <metal_stdlib>
using namespace metal;

struct VertexIn {
    float3 position [[attribute(0)]];
    float3 normal [[attribute(1)]];
    float2 uv [[attribute(2)]];
};

struct VertexOut {
    float4 position [[position]];
    float3 normal;
    float2 uv [[user(texturecoord)]];
};

struct Uniforms {
    float4x4 model;
    float4x4 view;
    float4x4 projection;
};

vertex VertexOut vertex_main(VertexIn in [[stage_in]], constant Uniforms& uniforms [[buffer(1)]]) {
    VertexOut out;
    out.position = uniforms.projection * uniforms.view * uniforms.model * float4(in.position, 1.0);
    out.normal = in.normal;
    out.uv = in.uv;
    return out;
}

fragment float4 fragment_main(VertexOut in [[stage_in]],
                              texture2d<float> texture [[texture(0)]],
                              sampler textureSampler [[sampler(0)]]) {
    float4 texColor = float4(texture.sample(textureSampler, in.uv).xyz,1.0);
    return texColor*(in.normal.y*0.5+0.5);
}