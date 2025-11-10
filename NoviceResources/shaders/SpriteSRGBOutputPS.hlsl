#include "Sprite.hlsli"

Texture2D<float4> tex : register(t0); // 0番スロットに設定されたテクスチャ
SamplerState smp : register(s0);      // 0番スロットに設定されたサンプラー

float3 ApplySRGBGamma(float3 linearColor)
{
    return linearColor < 0.0031308 ? 12.92 * linearColor : 1.055 * pow(linearColor, 1.0 / 2.4) - 0.055;
}

float4 main(VSOutput input) : SV_TARGET {
    float4 output = tex.Sample(smp, input.uv) * color;
    output.rgb = ApplySRGBGamma(output.rgb);
    return output;
}