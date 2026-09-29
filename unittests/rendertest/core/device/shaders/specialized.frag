// render.device.v2 D20 fixture: specialization constant 7 picks the color
// (0, the default, red; 1 green).
#version 450

layout( constant_id = 7 ) const int kChoice = 0;

layout( location = 0 ) out vec4 outColor;

void main()
{
	outColor = kChoice == 1 ? vec4( 0.0, 1.0, 0.0, 1.0 ) : vec4( 1.0, 0.0, 0.0, 1.0 );
}
