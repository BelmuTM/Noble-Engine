#include "include/common.slang"
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
    float3   debugColor;
};

[[vk::binding(0, 1)]] StructuredBuffer<ObjectData> objects;
[[vk::binding(0, 2)]] StructuredBuffer<uint> objectIndices;

struct VSOutput {
    float4 position : SV_Position;
    float3 color    : COLOR;
};

[shader("vertex")]
VSOutput vertMain(VSInput input, uint instanceID : SV_InstanceID, uint baseInstance : SV_StartInstanceLocation) {
    uint objectIndex  = objectIndices[baseInstance + instanceID];
    ObjectData object = objects[objectIndex];

    float4 worldPosition = mul(object.modelMatrix, float4(input.position, 1.0));

    VSOutput output;
    output.position = mul(uniforms.projection, mul(uniforms.view, worldPosition));
    output.color    = object.debugColor;

    return output;
}

layout (location = 0) out float4 debugBuffer;

[shader("fragment")]
void fragMain(VSOutput vertIn) : SV_TARGET {
    if (uniforms.debugMode != DEBUG_MESH) discard;

    debugBuffer = float4(vertIn.color, 1.0);
}
