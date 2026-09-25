#version 450
// Scene-derived WMSH PBR: Cycles diffuse-light bake plus a directional specular
// source, and split-sum specular from a map reflection probe when the LMAP
// atlas carries one, or from the material's $envmap cube. $emissiontexture
// (sRGB, decoded here) adds its color times $emissionscale. The native pixel
// fixture checks normal, metalness, roughness, emission and the environment.
//
// RUNTIME_INDIRECT (RFC 0011 render.indirect-policy.v1's RuntimeIndirect):
// the bound lightmap is the bake's direct layer, and the producer's indirect
// light is added from its atlas (set 8), sampled at the lightmap coordinate.
//
// DELTA_VOLUME (RFC 0011 render.indirect-policy.v1's BakedPlusDelta): the
// bound lightmap is the bake's total, and the producer's signed change of
// indirect light is added from its change volume (sets 8 and 9: a PRBV-layout
// atlas whose indirect layer holds the change, and the grid table), sampled
// at the surface with the probes' visibility. Under INDIRECT_VIEW the change
// is added to the view's indirect layer.
//
// DIRECT_LIGHTS (RFC 0011 G2) adds the frame's unbaked lights, from the
// engine's light set (render/light_set.h), through the legacy dlight falloff
// times the Lambert cosine and the layered BRDF (set 7). The push block's
// lightDirection / lightRadiance directional light is the pixel suite's
// hook; the engine never sets it.
//
// INDIRECT_VIEW (RFC 0011 debug view) replaces the shading with the indirect
// light alone, for comparison with Cycles' DiffInd pass: the bound lightmap
// is then the map's LMAP indirect layer, and the push block's lightDirection
// is ( view, exposure scale, 1 when that layer is bound, 0 ). View 1 writes
// the indirect diffuse light (irradiance / pi, no albedo), view 2 the
// indirect diffuse radiance (times the diffuse albedo and occlusion).
layout( location = 0 ) in vec2 fragUv;
layout( location = 1 ) in vec2 fragLightmapUv;
layout( location = 2 ) in vec3 fragPosition;
layout( location = 3 ) in vec3 fragNormal;
layout( location = 4 ) in vec4 fragTangent;
layout( location = 0 ) out vec4 outColor;

layout( set = 0, binding = 0 ) uniform sampler2D baseTexture;
layout( set = 1, binding = 0 ) uniform sampler2D mraoTexture;
layout( set = 2, binding = 0 ) uniform sampler2D normalTexture;
layout( set = 3, binding = 0 ) uniform sampler2D lightmapTexture;
layout( set = 4, binding = 0 ) uniform sampler2D splitSumTexture;
layout( set = 5, binding = 0 ) uniform sampler2D emissionTexture;
layout( set = 6, binding = 0 ) uniform samplerCube environmentTexture;

layout( push_constant ) uniform Constants
{
	mat4 mvp;
	vec4 eyePosition;   // w: $clearcoat (0: no coat)
	vec4 lightDirection;
	vec4 lightRadiance; // w: $clearcoatroughness
	// x alpha cutoff (<0 disables), y normal-map enable, z $emissionscale
	// (0: no emission), w the $envmap cube's mip count (0: no cube)
	vec4 material;
#ifdef CLIP_PLANES
	vec4 clipPlanes[2];
#endif
}
consts;

#include "world_pbr_probe.glsl"

vec3 SurfaceNormal()
{
	vec3 normal = normalize( fragNormal );
	if ( consts.material.y < 0.5 )
		return normal;
	return MappedNormal( normal, fragTangent, texture( normalTexture, fragUv ).rg );
}

// A 2:1 LMAP page is directional (tools/quality/lightmap_directional.py): the
// flat irradiance E0 baked on the smooth normal N on the left, and at the same
// texel on the right the world-space luminance gradient beta of the fitted
// irradiance E(n) = a + g.n, relative to E0. A normal-mapped normal n receives
// E0 * (1 + beta.(n - N)); n = N reproduces the flat bake exactly. Samples are
// clamped to their half so linear filtering never mixes the two.
vec3 BakedIrradiance( vec3 normal )
{
	ivec2 size = textureSize( lightmapTexture, 0 );
	if ( size.x != 2 * size.y )
		return texture( lightmapTexture, fragLightmapUv ).rgb;
	float halfTexel = 0.5 / float( size.x );
	float u = clamp( fragLightmapUv.x * 0.5, halfTexel, 0.5 - halfTexel );
	vec3 irradiance = texture( lightmapTexture, vec2( u, fragLightmapUv.y ) ).rgb;
	vec3 beta = texture( lightmapTexture, vec2( u + 0.5, fragLightmapUv.y ) ).rgb;
	float gain = 1.0 + dot( beta, normal - normalize( fragNormal ) );
	return irradiance * clamp( gain, 0.0, 4.0 );
}

// Specular image light: the material's $envmap cube, else the map probe.
bool ImageRadiance( vec3 direction, float roughness, out vec3 radiance )
{
	if ( consts.material.w >= 1.0 )
	{
		radiance = textureLod( environmentTexture, direction,
		    roughness * ( consts.material.w - 1.0 ) ).rgb;
		return true;
	}
	return ProbeRadiance( direction, roughness, radiance );
}

#ifdef RUNTIME_INDIRECT
layout( set = 8, binding = 0 ) uniform sampler2D producerIndirect;
#endif

#ifdef DELTA_VOLUME
layout( set = 8, binding = 0 ) uniform sampler2D probeAtlas; // the change, PRBV layout
layout( set = 9, binding = 0 ) uniform sampler2D probeGrids; // grid table, RGBA32F
#include "probe_volume.glsl"

// The producer's change of indirect diffuse light along the geometric normal.
vec3 IndirectChange( vec3 normal )
{
	vec3 change;
	return ProbeIrradiance( fragPosition, normal, 1, true, change ) ? change : vec3( 0.0 );
}
#endif

#ifdef DIRECT_LIGHTS
// Four vec4 per light: position.xyz, radius; color.rgb, minLight;
// direction.xyz, outer cone cosine (below -1: no cone); inner cone cosine,
// inverse square (1) or the legacy falloff, source radius.
layout( set = 7, binding = 0 ) uniform DirectLights
{
	vec4 header;      // x: the light count, y: 1 when the shadow field is bound
	vec4 fieldOrigin; // the shadow field's first voxel centre, w its voxel size
	vec4 fieldDims;   // its voxel counts
	vec4 lights[28];
}
directLights;

// RFC 0011 G9: the map's SDFV distances (Source units, R16F, trilinear),
// which shadow the unbaked lights. After the variant's other sets.
#ifdef DELTA_VOLUME
layout( set = 10, binding = 0 ) uniform sampler3D shadowField;
#else
layout( set = 9, binding = 0 ) uniform sampler3D shadowField;
#endif

// The field's distance at `p`; outside it nothing occludes.
float FieldDistance( vec3 p )
{
	vec3 g = ( p - directLights.fieldOrigin.xyz ) / directLights.fieldOrigin.w;
	vec3 dims = directLights.fieldDims.xyz;
	if ( any( lessThan( g, vec3( -0.5 ) ) ) || any( greaterThan( g, dims - 0.5 ) ) )
		return 1e9;
	return texture( shadowField, ( g + 0.5 ) / dims ).r;
}

// The share of a light of source radius `sourceRadius` at `lightPosition`
// that reaches `p`: a sphere trace through the field from a voxel off the
// surface, softened by the light's angular size (the nearest clearance d at
// distance t along the ray, over the light's angular radius, as a fraction).
float SdfShadow( vec3 p, vec3 normal, vec3 lightPosition, float sourceRadius )
{
	if ( directLights.header.y < 0.5 )
		return 1.0;
	float voxel = directLights.fieldOrigin.w;
	vec3 start = p + normal * voxel;
	vec3 toLight = lightPosition - start;
	float distance = length( toLight );
	vec3 direction = toLight / max( distance, 1e-4 );
	float tMax = distance - max( sourceRadius, voxel );
	float sharpness = distance / max( sourceRadius, 1.0 );
	float visible = 1.0;
	float t = 0.5 * voxel;
	for ( int step = 0; step < 96 && t < tMax; ++step )
	{
		float d = FieldDistance( start + direction * t );
		if ( d < 0.05 * voxel )
			return 0.0;
		visible = min( visible, sharpness * d / t );
		t += max( d, 0.2 * voxel );
	}
	return smoothstep( 0.0, 1.0, clamp( visible, 0.0, 1.0 ) );
}

// render/light_set.h Falloff(): the legacy dlight falloff.
float DynamicFalloff( float distanceSquared, float radius, float minLight )
{
	float radiusSquared = radius * radius;
	if ( !( radiusSquared > 0.0 ) || distanceSquared >= radiusSquared )
		return 0.0;
	float scale = distanceSquared > 0.0 ? radiusSquared * minLight / distanceSquared : 1.0;
	scale *= 1.0 - distanceSquared / radiusSquared;
	return min( scale, 2.0 );
}

// render/light_set.h InverseSquareFalloff(): a physical bulb.
float InverseSquareFalloff( float distanceSquared, float radius, float sourceRadius )
{
	float window = 1.0;
	if ( radius > 0.0 )
	{
		float ratio = distanceSquared / ( radius * radius );
		if ( ratio >= 1.0 )
			return 0.0;
		float edge = 1.0 - ratio * ratio;
		window = edge * edge;
	}
	return 10000.0 / max( distanceSquared, sourceRadius * sourceRadius ) * window;
}

// The direct light the frame's unbaked lights return toward the eye: the
// diffuse albedo times their diffuse light (the bake's unit), plus pi times
// their incident light through the specular lobe, as model_pbr.frag lights.
// Unbaked light i's incident diffuse light at the fragment (its falloff,
// cone and shadow; 0 when it cannot light it), with its direction and the
// Lambert cosine for `normal`.
vec3 DirectLightIncident( int i, vec3 normal, out vec3 light, out float normalDotLight )
{
	vec4 positionRadius = directLights.lights[4 * i];
	vec4 colorMinLight = directLights.lights[4 * i + 1];
	vec4 directionOuter = directLights.lights[4 * i + 2];
	vec4 coneFalloff = directLights.lights[4 * i + 3];
	float innerCos = coneFalloff.x;
	vec3 toLight = positionRadius.xyz - fragPosition;
	float distanceSquared = dot( toLight, toLight );
	float falloff = coneFalloff.y > 0.5
	                    ? InverseSquareFalloff( distanceSquared, positionRadius.w, coneFalloff.z )
	                    : DynamicFalloff( distanceSquared, positionRadius.w, colorMinLight.w );
	light = toLight * inversesqrt( max( distanceSquared, 1e-8 ) );
	normalDotLight = max( dot( normal, light ), 0.0 );
	if ( falloff <= 0.0 )
		return vec3( 0.0 );
	if ( directionOuter.w >= -1.0 )
	{
		float cosine = dot( -light, normalize( directionOuter.xyz ) );
		falloff *= innerCos > directionOuter.w + 1e-4
		               ? smoothstep( directionOuter.w, innerCos, cosine )
		               : step( directionOuter.w, cosine );
	}
	if ( falloff <= 0.0 || normalDotLight <= 0.0 )
		return vec3( 0.0 );
	falloff *= SdfShadow( fragPosition, normalize( fragNormal ), positionRadius.xyz,
	    coneFalloff.z );
	return colorMinLight.rgb * falloff;
}

// View 3 (RFC 0011 G9): the unbaked lights' diffuse light, irradiance / pi.
vec3 DirectLightDiffuse( vec3 normal )
{
	vec3 total = vec3( 0.0 );
	int count = int( directLights.header.x );
	for ( int i = 0; i < 7 && i < count; ++i )
	{
		vec3 light;
		float normalDotLight;
		total += DirectLightIncident( i, normal, light, normalDotLight ) * normalDotLight;
	}
	return total;
}

vec3 DirectLightRadiance( vec3 normal, vec3 view, vec3 diffuseAlbedo, vec3 f0, float roughness,
    vec3 compensation )
{
	vec3 total = vec3( 0.0 );
	int count = int( directLights.header.x );
	for ( int i = 0; i < 7 && i < count; ++i )
	{
		vec3 light;
		float normalDotLight;
		vec3 incident = DirectLightIncident( i, normal, light, normalDotLight );
		if ( normalDotLight <= 0.0 || all( equal( incident, vec3( 0.0 ) ) ) )
			continue;
		vec3 lit = diffuseAlbedo * incident * normalDotLight;
		if ( dot( normal, view ) > 0.0 )
			lit += kPi * incident * PbrSpecular( normal, view, light, f0, roughness ) *
			       compensation * normalDotLight;
		total += lit;
	}
	return total;
}
#endif

void main()
{
	vec4 baseSample = texture( baseTexture, fragUv );
	if ( consts.material.x >= 0.0 && baseSample.a < consts.material.x )
		discard;
#ifdef INDIRECT_VIEW
	{
		vec3 indirectLight =
		    consts.lightDirection.z > 0.5 ? BakedIrradiance( normalize( fragNormal ) ) : vec3( 0.0 );
#ifdef DELTA_VOLUME
		indirectLight = max( indirectLight + IndirectChange( normalize( fragNormal ) ), vec3( 0.0 ) );
#endif
		vec3 viewed = indirectLight;
		if ( consts.lightDirection.x > 1.5 )
		{
			vec3 viewMrao = texture( mraoTexture, fragUv ).rgb;
			viewed *= baseSample.rgb * ( 1.0 - clamp( viewMrao.r, 0.0, 1.0 ) ) *
			          clamp( viewMrao.b, 0.0, 1.0 );
		}
		outColor = vec4( viewed * consts.lightDirection.y, baseSample.a );
		return;
	}
#endif
	vec3 base = baseSample.rgb;
	vec3 mrao = texture( mraoTexture, fragUv ).rgb;
	float metalness = clamp( mrao.r, 0.0, 1.0 );
	float roughness = max( mrao.g, 0.02 );
	float occlusion = clamp( mrao.b, 0.0, 1.0 );
	vec3 normal = SurfaceNormal();
	vec3 view = normalize( consts.eyePosition.xyz - fragPosition );
	float normalDotView = max( dot( normal, view ), 0.0 );
	vec3 f0 = mix( vec3( 0.04 ), base, metalness );
	vec2 splitSum = PbrSplitSum( splitSumTexture, normalDotView, roughness );
	// Multiple scattering (pbr_brdf.glsl): the compensated lobe's albedo
	// weights image light and leaves the rest to the diffuse layer.
	vec3 compensation = PbrEnergyCompensation( f0, splitSum );
	vec3 directionalAlbedo = PbrDirectionalAlbedo( f0, splitSum );
	// Cycles DIFFUSE DIRECT+INDIRECT with COLOR disabled already contains the
	// Lambertian 1/pi factor. Multiplying this bake by albedo must not divide
	// it by pi again.
	vec3 bakedDiffuse = BakedIrradiance( normal );
#ifdef RUNTIME_INDIRECT
	bakedDiffuse += texture( producerIndirect, fragLightmapUv ).rgb;
#endif
#ifdef DELTA_VOLUME
	bakedDiffuse = max( bakedDiffuse + IndirectChange( normalize( fragNormal ) ), vec3( 0.0 ) );
#endif
#ifdef DIRECT_LIGHTS
	// View 3 (RFC 0011 G9): all diffuse light, for Cycles' DiffDir + DiffInd:
	// the bake and the producer's change at the smooth normal, plus the
	// unbaked lights' shadowed direct light, no albedo, times the exposure.
	if ( directLights.header.z > 0.5 )
	{
		vec3 smoothNormal = normalize( fragNormal );
		vec3 light = BakedIrradiance( smoothNormal );
#ifdef RUNTIME_INDIRECT
		light += texture( producerIndirect, fragLightmapUv ).rgb;
#endif
#ifdef DELTA_VOLUME
		light = max( light + IndirectChange( smoothNormal ), vec3( 0.0 ) );
#endif
		outColor = vec4( ( light + DirectLightDiffuse( smoothNormal ) ) * directLights.header.w,
		    baseSample.a );
		return;
	}
#endif
	vec3 diffuse = base * ( 1.0 - metalness ) *
	    ( vec3( 1.0 ) - directionalAlbedo ) * bakedDiffuse * occlusion;
	vec3 specular = vec3( 0.0 );
	vec3 light = normalize( -consts.lightDirection.xyz );
	float normalDotLight = max( dot( normal, light ), 0.0 );
	if ( normalDotView > 0.0 && normalDotLight > 0.0 )
		specular = consts.lightRadiance.rgb * PbrSpecular( normal, view, light, f0, roughness ) *
		           compensation * normalDotLight;
	float coat = consts.eyePosition.w;
	vec3 coatNormal = normalize( fragNormal );
	if ( coat > 0.0 )
	{
		// The coat (pbr_brdf.glsl, after Filament's standard model) on the
		// geometric normal; the base beneath is attenuated by 1 - Fc.
		vec3 halfVector = normalize( view + light );
		vec2 coatLobe = PbrClearCoat( coat, max( dot( coatNormal, halfVector ), 0.0 ),
		    max( dot( light, halfVector ), 0.0 ), consts.lightRadiance.w );
		specular = specular * ( 1.0 - coatLobe.y ) + consts.lightRadiance.rgb * coatLobe.x *
		                                                  max( dot( coatNormal, light ), 0.0 );
	}
	vec3 probe;
	vec3 reflected = reflect( -view, normal );
	// Specular horizon occlusion: a normal-mapped normal that faces away from
	// the viewer reflects it below the geometric surface, where no light
	// arrives (Cycles bends such normals back; without this, every groove
	// facing away from the camera mirrored the probe at Fresnel 1).
	float horizon = clamp( 1.0 + 1.3 * dot( reflected, normalize( fragNormal ) ), 0.0, 1.0 );
	float probeWeight = occlusion * horizon * horizon;
	vec3 image = diffuse;
	if ( ImageRadiance( reflected, roughness, probe ) )
		image += probe * directionalAlbedo * probeWeight;
	if ( coat > 0.0 )
	{
		// The baked and image light beneath the coat, attenuated by its
		// view-angle Fresnel, then the coat's own image light (its geometric
		// normal needs no horizon term).
		float coatFresnel = PbrFresnelSchlick( 0.04, max( dot( coatNormal, view ), 0.0 ) ) * coat;
		image *= 1.0 - coatFresnel;
		if ( ImageRadiance( reflect( -view, coatNormal ), max( consts.lightRadiance.w, 0.02 ),
		         probe ) )
			image += probe * coatFresnel * occlusion;
	}
	vec3 emission = vec3( 0.0 );
	if ( consts.material.z > 0.0 )
	{
		vec3 encoded = texture( emissionTexture, fragUv ).rgb;
		emission = mix( encoded / 12.92, pow( ( encoded + 0.055 ) / 1.055, vec3( 2.4 ) ),
		               step( 0.04045, encoded ) ) *
		           consts.material.z;
	}
#ifdef DIRECT_LIGHTS
	specular += DirectLightRadiance(
	    normal, view, base * ( 1.0 - metalness ) * ( vec3( 1.0 ) - directionalAlbedo ), f0, roughness,
	    compensation );
#endif
	outColor = vec4( image + specular + emission, baseSample.a );
}
