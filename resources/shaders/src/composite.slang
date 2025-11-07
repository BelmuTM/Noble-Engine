struct VSInput {
    uint id : SV_VertexID;
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

struct VSOutput {
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    float2 uv = float2((input.id << 1) & 2, input.id & 2);
    VSOutput output;
    output.uv       = uv;
    output.position = float4(uv * 2.0 - 1.0, 0.0, 1.0);
    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {
    float2 ndc = vertIn.uv * 2.0 - 1.0;

    float4 view  = mul(frameUBO.projectionInverse, float4(ndc, 1.0, 1.0));
           view /= view.w;

    float3 rayOrigin    = frameUBO.cameraPosition;
    float3 rayDirection = normalize(mul((float3x3) frameUBO.viewInverse, normalize(view.xyz)));

    float t = -rayOrigin.z / rayDirection.z;
    if (t <= 0.0) discard;

    float3 hit = rayOrigin + t * rayDirection;

    float  scale   = 1.0;
    float  checker = fmod(floor(hit.x * scale) + floor(hit.y * scale), 2.0);

    float3 planeColor = checker < 1.0 ? float3(0.1, 0.1, 0.1) : float3(1.0, 1.0, 1.0);

    return float4(planeColor, 1.0);
}
