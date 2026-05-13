/*
    [References]:
        Wikipedia. (2023). Luma (video). https://en.wikipedia.org/wiki/Luma_(video)
        Wikipedia. (2023). sRGB. https://en.wikipedia.org/wiki/SRGB
*/

//////////////////////////////////////////////////////////
/*------------- COLOR CONVERSION MATRICES --------------*/
//////////////////////////////////////////////////////////

static const float3x3 SRGB_2_XYZ_MAT = float3x3(
    0.4124564, 0.3575761, 0.1804375,
    0.2126729, 0.7151522, 0.0721750,
    0.0193339, 0.1191920, 0.9503041
);

static const float3x3 XYZ_2_SRGB_MAT = float3x3(
     3.2409699419,-1.5373831776,-0.4986107603,
    -0.9692436363, 1.8759675015, 0.0415550574,
     0.0556300797,-0.2039769589, 1.0569715142
);

//////////////////////////////////////////////////////////
/*----------------- COLOR CONVERSIONS ------------------*/
//////////////////////////////////////////////////////////

float luminance(float3 color) {
    return dot(color, SRGB_2_XYZ_MAT[1]);
}

float3 linearToSrgb(float3 linear) {
    float3 higher = (pow(abs(linear), float3(0.41666666)) * 1.055) - 0.055;
    float3 lower  = linear * 12.92;
    return lerp(higher, lower, step(linear, float3(0.0031308)));
}

float3 srgbToLinear(float3 srgb) {
    float3 higher = pow((srgb + 0.055) * 0.94786729, float3(2.4));
    float3 lower  = srgb * 0.07739938;
    return lerp(higher, lower, step(srgb, float3(0.04045)));
}
