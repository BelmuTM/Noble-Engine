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
ConstantBuffer<FrameUBO> frameUBO : register(b0, space0);

struct ObjectUBO {
    float4x4 model;
    float4x4 normalMatrix;
};
ConstantBuffer<ObjectUBO> objectUBO : register(b0, space1);

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
    output.worldPos  = mul(objectUBO.model, float4(input.inPosition, 1.0));
    output.position  = mul(frameUBO.projection, mul(frameUBO.view, output.worldPos));
    output.normal    = normalize(mul(float3x3(objectUBO.normalMatrix), input.inNormal));
    output.color     = input.inColor;
    output.texCoords = input.inTexCoords;
    return output;
}

layout(set = 1, binding = 1) Sampler2D texture;

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
    float3 lightPosition0 = float3(0.0, 5.0, 5.0);
    float3 lightPosition1 = float3(3.0, 2.0, 1.0);

    float3 rainbow;
    rainbow.r = sin(frameUBO.frameTimeCounter * TAU / 6.0 + 0.0) * 0.5 + 0.5;
    rainbow.g = sin(frameUBO.frameTimeCounter * TAU / 6.0 + 2.0) * 0.5 + 0.5;
    rainbow.b = sin(frameUBO.frameTimeCounter * TAU / 6.0 + 4.0) * 0.5 + 0.5;

    float3 color0 = rainbow;
    float  NdotL0 = max(0.0, dot(normalize(vertIn.normal), normalize(lightPosition0)));

    float3 color1 = float3(0.5, 0.9, 1.0);
    float  NdotL1 = max(0.0, dot(normalize(vertIn.normal), normalize(lightPosition1)));

    float dist = distance(frameUBO.cameraPosition, vertIn.worldPos.xyz);
    float fade = saturate((dist - frameUBO.nearPlane) / (1.0 - frameUBO.nearPlane));

    float threshold = bayer32(vertIn.position.xy * 0.5);

    if (threshold > fade) {
        discard;
    }

    float3 albedo   = texture.Sample(vertIn.texCoords).rgb;
    float3 lighting = albedo == float3(0.0) ? color0 * NdotL0 + color1 * NdotL1 : albedo;

    return float4(lighting, 1.0);
}
