#include "include/uniforms.slang"
#include "include/color.slang"

struct VSInput {
    float3 position  : POSITION;
    float3 normal    : TEXCOORD0;
    float4 tangent   : TEXCOORD1;
    float3 color     : TEXCOORD2;
    float2 texCoords : TEXCOORD3;
}

struct ObjectData {
    float4x4 modelMatrix;
    float4x4 normalMatrix;
};
[[vk::push_constant]] ObjectData object;

// [[vk::binding(0, 1)]] StructuredBuffer<uint> objectIndices;
// [[vk::binding(1, 1)]] StructuredBuffer<ObjectData> objects;

struct VSOutput {
    float4 position  : SV_Position;
    float3 normal    : TEXCOORD0;
    float4 tangent   : TEXCOORD1;
    float3 color     : TEXCOORD2;
    float2 texCoords : TEXCOORD3;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    // uint objIndex = objectIndices[instanceID];
    // ObjectData obj = objects[objIndex];

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

layout (set = 2, binding = 0) Sampler2D albedoTexture;
layout (set = 2, binding = 1) Sampler2D normalTexture;
layout (set = 2, binding = 2) Sampler2D specularTexture;

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
