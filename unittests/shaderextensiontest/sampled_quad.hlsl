//========= Copyright Valve Corporation, All rights reserved. ============//
// Native presentation fixture: clip-space geometry and an unchanged texture sample.
float4 clipOffset : register( c0 );
sampler2D image : register( s0 );

struct Vertex
{
	float4 position : POSITION;
	float2 uv : TEXCOORD0;
};

Vertex VertexMain( Vertex input )
{
	Vertex output = input;
	output.position += clipOffset;
	return output;
}

float4 PixelMain( float2 uv : TEXCOORD0 ) : COLOR
{
	return tex2D( image, uv );
}
