#version 450
// Applies the monitor gamma ramp (render.gamma-ramp.v1) while presenting, as a
// hardware gamma ramp would: each channel's stored 8-bit value indexes the
// 256-entry ramp. Entries are the ramp's 16-bit values normalized to 0..1; the
// index is interpolated so filtered (scaled) samples between two values stay
// continuous.
layout( set = 0, binding = 0 ) uniform sampler2D u_backBuffer;
layout( std140, set = 0, binding = 1 ) uniform GammaRamp
{
	vec4 entries[64];
}
u_ramp;

layout( location = 0 ) in vec2 v_uv;
layout( location = 0 ) out vec4 o_color;

float Entry( int index )
{
	return u_ramp.entries[index >> 2][index & 3];
}

float Ramp( float value )
{
	float position = clamp( value, 0.0, 1.0 ) * 255.0;
	int lower = int( floor( position ) );
	int upper = min( lower + 1, 255 );
	return mix( Entry( lower ), Entry( upper ), position - float( lower ) );
}

void main()
{
	vec4 color = texture( u_backBuffer, v_uv );
	o_color = vec4( Ramp( color.r ), Ramp( color.g ), Ramp( color.b ), color.a );
}
