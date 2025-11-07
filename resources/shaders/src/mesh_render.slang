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
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;
    output.position  = mul(frameUBO.projection, mul(frameUBO.view, mul(objectUBO.model, float4(input.inPosition, 1.0))));
    output.normal    = mul(input.inNormal, float3x3(objectUBO.normalMatrix));
    output.color     = input.inColor;
    output.texCoords = input.inTexCoords;
    return output;
}

layout(set = 1, binding = 1) Sampler2D texture;

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {
    float3 lightPosition = float3(0.0, 7.0, 0.0);

    float3 color = float3(0.2, sin(frameUBO.frameTimeCounter), cos(frameUBO.frameTimeCounter));
    float  NdotL = max(0.01, dot(normalize(vertIn.normal), normalize(lightPosition)));

    return float4(color * NdotL, 1.0);
}
