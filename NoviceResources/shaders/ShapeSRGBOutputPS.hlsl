#include "Shape.hlsli"

float3 ApplySRGBGamma(float3 linearColor)
{
    return linearColor < 0.0031308 ? 12.92 * linearColor : 1.055 * pow(linearColor, 1.0 / 2.4) - 0.055;
}

float4 main(VSOutput input) : SV_TARGET {
    float4 output = input.color;
    output.xyz = ApplySRGBGamma(output.xyz);
    return output;
}