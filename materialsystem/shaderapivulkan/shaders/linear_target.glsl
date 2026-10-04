// Legacy frontend storage adapter. Its caller defines LegacyColorMain and
// outColor; alpha, discard and all material arithmetic remain the caller's.
// Specialization 31 converts gamma-valued UI/post pixels into the linear
// floating point scene. Linear scene writers disable their old output encode.
#include "../../../render/shaders/common/color_transfer.glsl"
layout( constant_id = 31 ) const bool kDecodeLegacyOutput = false;
void main()
{
	LegacyColorMain();
	if ( kDecodeLegacyOutput )
		outColor.rgb = OutputLinearFromSrgb( outColor.rgb );
}
