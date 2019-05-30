struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

float4 PS(PS_INPUT pixel) : SV_TARGET
{
    return pixel.color;
}
