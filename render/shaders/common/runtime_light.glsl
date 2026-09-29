// render.light-set.v1's runtime point and spot lights (RFC 0011,
// public/render/light_set.h): the one GLSL copy of their falloff and cone
// for the render core. Each function mirrors the C++ definition it names;
// render_lab's clustered-lights suite judges them against it.
#ifndef RUNTIME_LIGHT_GLSL
#define RUNTIME_LIGHT_GLSL

// light_set::Falloff: a Source dlight.
float RuntimeLightFalloffLegacy( float distanceSquared, float radius, float minLight )
{
	const float radiusSquared = radius * radius;
	if ( !( radiusSquared > 0.0 ) || distanceSquared >= radiusSquared )
		return 0.0;
	float scale = distanceSquared > 0.0 ? radiusSquared * minLight / distanceSquared : 1.0;
	scale *= 1.0 - distanceSquared / radiusSquared;
	return min( scale, 2.0 );
}

// light_set::InverseSquareFalloff: a physical bulb of the given source radius.
float RuntimeLightFalloffInverseSquare( float distanceSquared, float radius, float sourceRadius )
{
	float window = 1.0;
	if ( radius > 0.0 )
	{
		const float ratio = distanceSquared / ( radius * radius );
		if ( ratio >= 1.0 )
			return 0.0;
		const float edge = 1.0 - ratio * ratio;
		window = edge * edge;
	}
	const float d2 = max( distanceSquared, sourceRadius * sourceRadius );
	return 100.0 * 100.0 / d2 * window;
}

// light_set::SpotFactor.
float RuntimeLightSpot( float cosine, float innerCos, float outerCos )
{
	if ( !( innerCos > outerCos + 1e-4 ) )
		return cosine >= outerCos ? 1.0 : 0.0;
	const float t = clamp( ( cosine - outerCos ) / ( innerCos - outerCos ), 0.0, 1.0 );
	return t * t * ( 3.0 - 2.0 * t );
}

#endif // RUNTIME_LIGHT_GLSL
