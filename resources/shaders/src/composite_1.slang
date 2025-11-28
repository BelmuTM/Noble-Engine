struct VSInput {
    uint id : SV_VertexID;
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

struct VSOutput {
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    float2 uv = float2((input.id & 1) * 2.0, (input.id >> 1) * 2.0);

    VSOutput output;
    output.uv       = uv;
    output.position = float4(uv * 2.0 - 1.0, 0.0, 1.0);

    return output;
}

layout (location = 0) out float4 output;

layout (set = 1, binding = 0) Sampler2D compositeBuffer;

[shader("fragment")]
void fragMain(VSOutput vertIn) : SV_TARGET {
    float3 color = compositeBuffer.Sample(vertIn.uv).rgb;

    output = float4(color, 1.0);
}
