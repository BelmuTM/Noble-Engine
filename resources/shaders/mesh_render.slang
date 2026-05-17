#include "include/uniforms.slang"
#include "include/color.slang"

struct VSInput {
    float3 position  : POSITION;
    float3 normal    : NORMAL;
    float4 tangent   : TANGENT;
    float3 color     : COLOR;
    float2 texCoords : TEXCOORD0;
}

struct ObjectData {
    float4x4 modelMatrix;
    float4x4 normalMatrix;
    float3   debugColor;
};

[[vk::binding(0, 1)]] StructuredBuffer<ObjectData> objects;
[[vk::binding(0, 2)]] StructuredBuffer<uint> objectIndices;

struct VSOutput {
    float4 position  : SV_Position;
    float3 normal    : NORMAL;
    float4 tangent   : TANGENT;
    float3 color     : COLOR;
    float2 texCoords : TEXCOORD0;
};

[shader("vertex")]
VSOutput vertMain(VSInput input, uint instanceID : SV_InstanceID, uint baseInstance : SV_StartInstanceLocation) {
    uint objectIndex  = objectIndices[baseInstance + instanceID];
    ObjectData object = objects[objectIndex];

    float4 worldPosition = mul(object.modelMatrix, float4(input.position, 1.0));
    float3 worldNormal   = normalize(mul(float3x3(object.normalMatrix), input.normal));

    VSOutput output;
    output.position  = mul(uniforms.projection, mul(uniforms.view, worldPosition));
    output.normal    = worldNormal;
    output.tangent   = input.tangent;
    output.color     = input.color;
    output.texCoords = input.texCoords;

    return output;
}

layout (location = 0) out float4 albedoBuffer;
layout (location = 1) out float4 normalBuffer;
// layout (location = 2) out float4 debugViewBuffer;

layout (set = 3, binding = 0) Sampler2D albedoTexture;
layout (set = 3, binding = 1) Sampler2D normalTexture;
layout (set = 3, binding = 2) Sampler2D specularTexture;

[shader("fragment")]
void fragMain(VSOutput vertIn) : SV_TARGET {
    float4 albedo = albedoTexture.Sample(vertIn.texCoords);

    albedo.rgb *= vertIn.color;

    if (albedo.a < 1.0) {
        discard;
    }

    albedo.rgb = srgbToLinear(albedo.rgb);

    float3 worldNormal   = vertIn.normal;
    float3 tangentNormal = normalTexture.Sample(vertIn.texCoords).rgb * 2.0 - 1.0;

    if (length(vertIn.tangent.xyz) > 1e-4) {
        float3 N = vertIn.normal;
        float3 T = vertIn.tangent.xyz;
        float3 B = cross(N, T) * vertIn.tangent.w;

        float3x3 TBN = float3x3(T, B, N);

        worldNormal = normalize(mul(tangentNormal, TBN));
    }

    albedoBuffer = albedo;
    normalBuffer = float4(worldNormal * 0.5 + 0.5, 1.0);

    // debugViewBuffer = float4(worldNormal * 0.5 + 0.5, 1.0);
}
