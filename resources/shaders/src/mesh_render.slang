struct VSInput {
    float3 inPosition;
    float3 inNormal;
    float3 inColor;
    float2 inTexCoords;
};

struct FrameUBO {
    float4x4 view;
    float4x4 viewInverse;
    float4x4 projection;
    float4x4 projectionInverse;
    float3   cameraPosition;
    float    nearPlane;
    float    farPlane;
    float    frameTimeCounter;
    float    viewWidth;
    float    viewHeight;
};
[[vk::binding(0, 0)]] ConstantBuffer<FrameUBO> frameUBO;

struct ObjectDataGPU {
    float4x4 model;
    float4x4 normalMatrix;
};
[[vk::push_constant]] ObjectDataGPU object;

struct VSOutput {
    float4 position : SV_Position;
    float3 normal;
    float3 color;
    float2 texCoords : TEXCOORD0;
    float4 worldPos  : TEXCOORD1;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;
    output.worldPos  = mul(object.model, float4(input.inPosition, 1.0));
    output.position  = mul(frameUBO.projection, mul(frameUBO.view, output.worldPos));
    output.normal    = normalize(mul(float3x3(object.normalMatrix), input.inNormal));
    output.color     = input.inColor;
    output.texCoords = input.inTexCoords;
    return output;
}

layout (location = 0) out float4 albedoTexture;
layout (location = 1) out float4 normalTexture;

layout (set = 1, binding = 0) Sampler2D texture;

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

    float4 albedo = texture.Sample(vertIn.texCoords);

    if (albedo.a < 1.0) {
        discard;
    }

    albedoTexture = albedo;
    normalTexture = float4(vertIn.normal * 0.5 + 0.5, 1.0);
}
