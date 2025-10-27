struct VSInput {
    float3 inPosition;
    float3 inNormal;
    float3 inColor;
    float2 inTexCoords;
};

struct UniformBuffer {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};
ConstantBuffer<UniformBuffer> ubo;

struct VSOutput {
    float4 position : SV_Position;
    float3 normal;
    float3 color;
    float2 texCoords;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;
    output.position  = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.inPosition, 1.0))));
    output.normal    = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.inNormal, 1.0)))).xyz;
    output.color     = input.inColor;
    output.texCoords = input.inTexCoords;
    return output;
}

Sampler2D texture;

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {
    return float4(float3(0.2, 0.7, 1.0) * dot(normalize(vertIn.normal), normalize(float3(0.0, 100.0, 100.0))), 1.0);
}
