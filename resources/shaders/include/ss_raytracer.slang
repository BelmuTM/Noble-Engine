/*
    [References]:
        McGuire, M., & Mara, M. (2014). Efficient GPU Screen-Space Ray Tracing. https://jcgt.org/published/0003/04/04/paper.pdf
*/

float minOf(float2 x) { return min(x.x, x.y);                     }
float minOf(float3 x) { return min(x.x, min(x.y, x.z));           }
float minOf(float4 x) { return min(min(x.x, x.y), min(x.z, x.w)); }

float maxOf(float2 x) { return max(x.x, x.y);                     }
float maxOf(float3 x) { return max(x.x, max(x.y, x.z));           }
float maxOf(float4 x) { return max(max(x.x, x.y), max(x.z, x.w)); }

float thickenDepth(float depth, float zThickness, float4x4 projection) {
    depth = 1.0 - 2.0 * depth;
    depth = (depth + projection[2].z * zThickness) / (1.0 + zThickness);
    return 0.5 - 0.5 * depth;
}

bool raytrace(
    Sampler2D inputDepthTexture,
    float4x4 projection,
    float4x4 projectionInverse,
    float3 viewPosition,
    float3 rayDirection,
    float stride,
    float jitter,
    float2 resolution,
    out float3 rayPosition
) {
    // Scale the jitter to the stride in pixel size
    jitter *= stride;

    // DDA setup (McGuire & Mara, 2014)
    rayPosition   = viewToScreen(viewPosition, projection);
    rayDirection  = viewPosition + abs(viewPosition.z) * rayDirection;
    rayDirection  = viewToScreen(rayDirection, projection) - rayPosition;
    rayDirection *= minOf((step(0.0, rayDirection) - rayPosition) / rayDirection);

    rayPosition.xy  *= resolution;
    rayDirection.xy *= resolution;

    // Normalise the DDA ray step to walk a fixed amount of pixels per step
    rayDirection /= maxOf(abs(rayDirection.xy));

    // Upper screen-space bounds are XY = resolution - 1.0 and Z = 1.0
    float3 startPosition = rayPosition;
    float3 endPosition   = step(0.0, rayDirection) * float3(resolution - 1.0, 1.0);

    // Compute the minimal amount of steps to reach an upper bound on any axis
    float3 stepsToEndPosition    = (endPosition - startPosition) / rayDirection;
    // Extend the upper bound to guarantee full coverage of the far plane
           stepsToEndPosition.z += stride;

    // Clamp to the resolution to avoid marching forever
    float tMax = min(minOf(stepsToEndPosition), maxOf(resolution));
    float t    = jitter;

    /*
        Thicken each depth sample by a factor to prevent false positives during
        intersection checks, this makes each depth sample equivalent to a frustum-shaped voxel
        (McGuire & Mara, 2014)
    */
    float zThickness = max(log2(stride), 1.0) * stride * projectionInverse[1].y / resolution.y;

    bool intersected = false;

    // March until we reach the edge or intersect something
    while (t < tMax && !intersected) {
        rayPosition = startPosition + rayDirection * t;

        float stepT = (t == jitter ? jitter : stride);
        float maxZ  = rayPosition.z;
        float minZ  = rayPosition.z - stepT * abs(rayDirection.z);

        float depth      = inputDepthTexture.Sample(rayPosition.xy / resolution).r;
        float thickDepth = thickenDepth(depth, zThickness, projection);

        /*
            Intersection check, take account of the depth sample's thickness, and avoid
            player hand fragments and sky fragments
        */
        if (maxZ >= depth && minZ <= thickDepth) {
            intersected = true;
            break;
        }

        t += stride;
    }

    rayPosition.xy /= resolution;

    return intersected;
}
