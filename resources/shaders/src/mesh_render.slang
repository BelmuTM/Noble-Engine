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

layout(set = 1, binding = 0) Sampler2D texture;

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

#define TAU 2.0 * 3.14159

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {
    const float3 lightPosition0 = float3(-1.5, 1.0, 5.0);
    const float3 lightPosition1 = float3(4.5, -1.0, 1.0);

    float3 lightDirection0 = normalize(lightPosition0 - vertIn.worldPos.xyz);
    float3 lightDirection1 = normalize(lightPosition1 - vertIn.worldPos.xyz);

    float NdotL0 = max(0.0, dot(normalize(vertIn.normal), lightDirection0));
    float NdotL1 = max(0.0, dot(normalize(vertIn.normal), lightDirection1));

    float3 rainbow;
    rainbow.r = sin(frameUBO.frameTimeCounter * TAU / 6.0 + 0.0) * 0.5 + 0.5;
    rainbow.g = sin(frameUBO.frameTimeCounter * TAU / 6.0 + 2.0) * 0.5 + 0.5;
    rainbow.b = sin(frameUBO.frameTimeCounter * TAU / 6.0 + 4.0) * 0.5 + 0.5;

    const float3 lightColor0 = float3(0.969, 0.549, 0.125);
    float3 lightColor1 = rainbow;

    const float fadeStartDist = 0.25;

    float dist = distance(frameUBO.cameraPosition, vertIn.worldPos.xyz);
    float fade = saturate((dist - frameUBO.nearPlane) / (fadeStartDist - frameUBO.nearPlane));

    const float ditherScale = 0.5;
    float threshold = bayer32(vertIn.position.xy * ditherScale);

    if (threshold > fade) {
        discard;
    }

    float4 albedo = texture.Sample(vertIn.texCoords);

    if (albedo.a < 1.0) {
        discard;
    }

    float dist0 = length(lightPosition0 - vertIn.worldPos.xyz);
    float dist1 = length(lightPosition1 - vertIn.worldPos.xyz);

    float attenuation0 = 1.0 / (dist0 * dist0 * 0.15);
    float attenuation1 = 1.0 / (dist1 * dist1 * 0.35);

    float3 lighting = albedo.rgb * (lightColor0 * NdotL0 * attenuation0 + lightColor1 * NdotL1 * attenuation1);

    return float4(lighting, 1.0);
}
