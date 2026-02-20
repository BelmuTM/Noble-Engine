struct FrameUBO {
    float4x4 view;
    float4x4 viewInverse;
    float4x4 projection;
    float4x4 projectionInverse;
    float3   cameraPosition;
    float    nearPlane;
    float    farPlane;
    float    frameTimeCounter;
    float    frameCounter;
    float    viewWidth;
    float    viewHeight;
    int      debugMode;
};
[[vk::binding(0, 0)]] ConstantBuffer<FrameUBO> uniforms;
