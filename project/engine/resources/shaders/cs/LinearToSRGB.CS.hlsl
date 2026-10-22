RWTexture2D<unorm float4> gOutput : register(u0);

float LinearToSrgbChannel(float linearValue){
    linearValue = saturate(linearValue);
    if (linearValue <= 0.0031308){
        return linearValue * 12.92;
    }
    return pow(linearValue, 1.0 / 2.4) * 1.055 - 0.055;
}

float3 LinearToSrgb(float3 linearValue){
    return float3(
        LinearToSrgbChannel(linearValue.r),
        LinearToSrgbChannel(linearValue.g),
        LinearToSrgbChannel(linearValue.b)
    );
}

[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID){
    uint width, height;
    gOutput.GetDimensions(width, height);
    if (dispatchThreadID.x >= width || dispatchThreadID.y >= height) {
        return;
    }

    float4 linearColor = gOutput[dispatchThreadID.xy];
    linearColor.rgb = LinearToSrgb(linearColor.rgb);
    gOutput[dispatchThreadID.xy] = linearColor;
}
