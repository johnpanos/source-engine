#version 450
// Base-texture material shader: samples the bound texture at the interpolated
// coordinate, the way UnlitGeneric/$basetexture materials do.
layout( location = 0 ) in vec2 fragUv;
layout( location = 0 ) out vec4 outColor;
layout( binding = 0 ) uniform sampler2D baseTexture;
void main()
{
	outColor = texture( baseTexture, fragUv );
}
