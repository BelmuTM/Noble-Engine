#include "include/uniforms.slang"

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
};
[[vk::push_constant]] ObjectData object;

struct VSOutput {
    float4 position  : SV_Position;
    float3 normal    : NORMAL;
    float4 tangent   : TANGENT;
    float3 color     : COLOR;
    float2 texCoords : TEXCOORD0;
    float4 worldPos  : TEXCOORD1;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    float3 worldNormal  = normalize(mul(float3x3(object.normalMatrix), input.normal));
    float3 worldTangent = normalize(mul(float3x3(object.normalMatrix), input.tangent.xyz));

    VSOutput output;
    output.worldPos  = mul(object.modelMatrix, float4(input.position, 1.0));
    output.position  = mul(uniforms.projection, mul(uniforms.view, output.worldPos));
    output.normal    = worldNormal;
    output.tangent   = float4(worldTangent, input.tangent.w);
    output.color     = input.color;
    output.texCoords = input.texCoords;

    return output;
}

layout (location = 0) out float4 albedoBuffer;
layout (location = 1) out float4 normalBuffer;
layout (location = 2) out float4 debugBuffer;

layout (set = 1, binding = 0) Sampler2D albedoTexture;
layout (set = 1, binding = 1) Sampler2D normalTexture;
layout (set = 1, binding = 2) Sampler2D specularTexture;

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

[shader("fragment")]
void fragMain(VSOutput vertIn) : SV_TARGET {
    const float fadeStartDist = 0.25;

    float4 albedo = albedoTexture.Sample(vertIn.texCoords);

    if (albedo.a < 1.0) {
        discard;
    }

    float3 normal = normalTexture.Sample(vertIn.texCoords).rgb;

    if (!normal.equals(float3(0.0))) {

        float3 N = normalize(vertIn.normal);

        float3 T = normalize(vertIn.tangent.xyz);
        float3 B = vertIn.tangent.w * cross(N, T);

        float3x3 TBN = float3x3(T, B, N);

        normal = normalize(mul(normal * 2.0 - 1.0, TBN));

    } else {
        normal = normalize(vertIn.normal);
    }

    albedoBuffer = albedo;
    normalBuffer = float4(normal * 0.5 + 0.5, 1.0);
    //debugBuffer  = float4(max(0.0, vertIn.tangent.w), 0.0, 0.0, 1.0);
}
