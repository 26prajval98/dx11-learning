struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

struct VS_INPUT
{
    float4 pos : POSITION;
    float4 color : COLOR;
};

PS_INPUT VS(VS_INPUT ip)
{
    PS_INPUT op;
    op.pos = ip.pos;
    op.color = ip.color;

    return op;
}

float4 PS(PS_INPUT pixel) : SV_TARGET
{
    return pixel.color;
}
