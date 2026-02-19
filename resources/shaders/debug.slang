#include "include/uniforms.slang"

struct VSInput {
    float3 position  : POSITION;
    float3 normal    : NORMAL;
    float4 tangent   : TANGENT;
    float3 color     : COLOR;
    float2 texCoords : TEXCOORD0;
};

struct ObjectData {
    float4x4 modelMatrix;
    float4x4 normalMatrix;
};
[[vk::push_constant]] ObjectData object;

struct VSOutput {
    float4 position : SV_Position;
    float3 color    : COLOR;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;
    output.position = mul(uniforms.projection, mul(uniforms.view, mul(object.modelMatrix, float4(input.position, 1.0))));
    output.color    = input.color;
    return output;
}

layout (location = 0) out float4 debugBuffer;

[shader("fragment")]
void fragMain(VSOutput vertIn) : SV_TARGET {
    debugBuffer = float4(vertIn.color, 1.0);
}
