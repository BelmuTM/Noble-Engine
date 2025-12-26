#include "include/uniforms.slang"

struct VSInput {
    uint id : SV_VertexID;
};

struct VSOutput {
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    float2 uv = float2((input.id & 1) * 2.0, (input.id >> 1) * 2.0);

    VSOutput output;
    output.uv       = uv;
    output.position = float4(uv * 2.0 - 1.0, 0.0, 1.0);

    return output;
}

layout (location = 0) out float4 compositeBuffer;

layout (set = 1, binding = 0) Sampler2D depthBuffer;
layout (set = 1, binding = 1) Sampler2D albedoBuffer;
layout (set = 1, binding = 2) Sampler2D normalBuffer;
layout (set = 1, binding = 3) Sampler2D debugBuffer;

#define TAU 2.0 * 3.14159

#include "include/transforms.slang"

[shader("fragment")]
void fragMain(VSOutput vertIn) : SV_TARGET {
    uint2 resolution = uint2(uint(uniforms.viewWidth), uint(uniforms.viewHeight));
    uint2 coords     = uint2(round(vertIn.uv * resolution));

    float3 albedo = albedoBuffer.Sample(vertIn.uv).rgb;
    float3 normal = normalBuffer.Sample(vertIn.uv).rgb * 2.0 - 1.0;

    float depth = depthBuffer.Sample(vertIn.uv).r;

    float3 screenPosition = float3(vertIn.uv, depth);
    float3 viewPosition   = screenToView(screenPosition, uniforms.projectionInverse);
    float3 worldPosition  = viewToWorld(viewPosition, uniforms.viewInverse);

    float3 lightPosition0 = float3(0.0, 1.0, 5.0);
    float3 lightPosition1 = float3(4.5, -1.0, 1.0);

    lightPosition0.x += lerp(0.0, 5.5, sin(uniforms.frameTimeCounter));

    float3 lightDirection0 = normalize(lightPosition0 - worldPosition.xyz);
    float3 lightDirection1 = normalize(lightPosition1 - worldPosition.xyz);

    float NdotL0 = max(0.0, dot(normalize(normal), lightDirection0));
    float NdotL1 = max(0.0, dot(normalize(normal), lightDirection1));

    float dist0 = length(lightPosition0 - worldPosition.xyz);
    float dist1 = length(lightPosition1 - worldPosition.xyz);

    float attenuation0 = 1.0 / (dist0 * dist0 * 0.15);
    float attenuation1 = 1.0 / (dist1 * dist1 * 0.35);

    float3 rainbow;
    rainbow.r = sin(uniforms.frameTimeCounter * TAU / 6.0 + 0.0) * 0.5 + 0.5;
    rainbow.g = sin(uniforms.frameTimeCounter * TAU / 6.0 + 2.0) * 0.5 + 0.5;
    rainbow.b = sin(uniforms.frameTimeCounter * TAU / 6.0 + 4.0) * 0.5 + 0.5;

    const float3 lightColor0 = float3(0.969, 0.549, 0.125);
    float3 lightColor1 = rainbow;

    const float3 ambient = float3(0.01);

    float3 lighting = albedo * (ambient + lightColor0 * NdotL0 * attenuation0 + lightColor1 * NdotL1 * attenuation1);

    compositeBuffer = float4(lighting, 1.0);

    /*
    if (coords.x < resolution.x * 0.4 && coords.y < resolution.y * 0.4) {
        float4 debug = debugBuffer.Sample(vertIn.uv / 0.4);
        compositeBuffer = debug;
    }
    */
}
