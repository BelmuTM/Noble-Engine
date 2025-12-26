float4 screenToClip(float3 screenPosition) {
    return float4(float2(screenPosition.x * 2.0 - 1.0, 1.0 - screenPosition.y * 2.0), screenPosition.z, 1.0);
}

float3 screenToView(float3 screenPosition, float4x4 projectionInverse) {
    float4 viewPosition = mul(projectionInverse, screenToClip(screenPosition));
    return viewPosition.xyz / viewPosition.w;
}

float3 viewToScreen(float3 viewPosition, float4x4 projection) {
    float4 clipPosition = mul(projection, float4(viewPosition, 1.0));
    float3 ndcPosition  = clipPosition.xyz / clipPosition.w;

    return float3(ndcPosition.x * 0.5 + 0.5, 0.5 + ndcPosition.y * 0.5, ndcPosition.z);
}

float3 viewToWorld(float3 viewPosition, float4x4 viewInverse) {
    return mul(viewInverse, float4(viewPosition, 1.0)).xyz;
}

float linearizeDepth(float depth, float nearPlane, float farPlane) {
    return (nearPlane * farPlane) / (depth * (nearPlane - farPlane) + farPlane);
}
