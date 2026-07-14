#include "include/common.slang"
#include "include/uniforms.slang"
#include "include/color.slang"
#include "include/transforms.slang"

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

layout (location = 0) out float4 swapchainOutput;

layout (set = 3, binding = 0) Sampler2D depthBuffer;
layout (set = 3, binding = 1) Sampler2D albedoBuffer;
layout (set = 3, binding = 2) Sampler2D normalBuffer;
layout (set = 3, binding = 3) Sampler2D debugBuffer;

layout (set = 3, binding = 4) Sampler2D compositeBuffer;

float bayer2(float2 a) {
    a = floor(a);
    return fract(dot(a, float2(0.5, a.y * 0.75)));
}

#define bayer4(a)   (bayer2(0.5   * (a))  *  0.25 + bayer2(a))
#define bayer8(a)   (bayer4(0.5   * (a))  *  0.25 + bayer2(a))
#define bayer16(a)  (bayer8(0.5   * (a))  *  0.25 + bayer2(a))
#define bayer32(a)  (bayer16(0.5  * (a))  *  0.25 + bayer2(a))
#define bayer64(a)  (bayer32(0.5  * (a))  *  0.25 + bayer2(a))
#define bayer128(a) (bayer64(0.5  * (a))  *  0.25 + bayer2(a))
#define bayer256(a) (bayer128(0.5 * (a))  *  0.25 + bayer2(a))
#define bayer512(a) (bayer256(0.5 * (a))  *  0.25 + bayer2(a))

void quantizeColor(inout float3 color, float quantizationPeriod) {
    color = floor((color + quantizationPeriod * 0.5) / quantizationPeriod) * quantizationPeriod;
}

void ditherColor(inout float3 color, float2 coords, float quantizationPeriod) {
    color += (bayer4(coords) - 0.5) * quantizationPeriod;
}

void debugOutput(inout float3 color, float2 coords) {
    switch (uniforms.debugMode) {
        case DEBUG_ALBEDO:
            color = albedoBuffer.Sample(coords).rgb; break;

        case DEBUG_NORMAL:
            color = normalBuffer.Sample(coords).rgb; break;

        case DEBUG_DEPTH:
            color = depthBuffer.Sample(coords).r; break;

        case DEBUG_MESH:
            color += debugBuffer.Sample(coords).rgb; break;

        default: break;
    }
}

[shader("fragment")]
void fragMain(VSOutput vertIn) : SV_TARGET {
    float2 resolution = float2(uniforms.viewWidth, uniforms.viewHeight);

    float2 coords = vertIn.uv * resolution;

    float3 color = compositeBuffer.Sample(vertIn.uv).rgb;

    const int   colorPaletteSize   = 8;
    const float quantizationPeriod = 1.0 / colorPaletteSize;

    //ditherColor  (color, coords, quantizationPeriod);
    //quantizeColor(color, quantizationPeriod);

    debugOutput(color, vertIn.uv);

    color = linearToSrgb(color);

    swapchainOutput = float4(color, 1.0);
}
