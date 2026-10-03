// render.energy-field.v1: SolidEnergy's intrinsic flow radiance, before
// camera opacity/fade, exposure and output encoding. The retained surface and
// render_lab use this one GLSL definition; public/render/energy_field.h is its
// CPU integration oracle. No additional receiving-light model belongs here.
#ifndef ENERGY_FIELD_GLSL
#define ENERGY_FIELD_GLSL

float EnergyFieldSmooth( float a, float b, float x )
{
	float t = clamp( ( x - a ) / ( b - a ), 0.0, 1.0 );
	return t * t * ( 3.0 - 2.0 * t );
}

vec4 EnergyFieldReveal( vec4 base, float noise, float edge, float powerUp )
{
#ifdef SEEDED_FIELD_REVEAL_IGNORED
	powerUp = 1.0;
#endif
	if ( powerUp > 0.0 && powerUp < 1.0 )
	{
		float reveal = ( noise + ( 1.0 - edge ) ) * 0.5;
		float stage = clamp( powerUp * 3.0, 0.0, 1.0 );
		float pulse = EnergyFieldSmooth( 0.02, 0.0, abs( reveal - powerUp ) );
		float mask = EnergyFieldSmooth( 0.02, 0.0, reveal - powerUp );
		base.ag += pulse * powerUp * ( 1.0 - powerUp );
		base.ag *= stage * mask;
		base.ag += edge * stage;
	}
	else
		base.ag += edge;
	return base;
}

vec3 EnergyFieldRadiance( vec4 base, vec3 flowColor, vec3 vortexColor,
    float vortexIntensity, bool vortices, float bounds, float intensity )
{
	vec3 field = base.a * flowColor;
	if ( vortices )
	{
		vec3 vortex = base.g * vortexColor;
		field = mix( field, vortex, vortexIntensity );
	}
#ifdef SEEDED_FIELD_INTENSITY_IGNORED
	intensity = 1.0;
#endif
	return field * bounds * intensity;
}

#endif // ENERGY_FIELD_GLSL
