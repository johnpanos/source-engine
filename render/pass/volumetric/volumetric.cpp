//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.volumetric (RFC 0016, the participating-media term);
//			see volumetric.h.
//
//=============================================================================//

#include "render/pass/volumetric/volumetric.h"

#include "render/shaderlib/core_artifacts.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <utility>

namespace render::pass::volumetric
{

namespace
{

using namespace render::device;

constexpr const char *kInjectSource = "render/pass/volumetric/volumetric_inject.comp";
constexpr const char *kCompositeVertex = "render/pass/volumetric/volumetric_composite.vert";
constexpr const char *kCompositeSource = "render/pass/volumetric/volumetric_composite.frag";

// The stages' bindings, all in the draw group.
enum Binding : std::uint32_t
{
	kParams = 0,
	kSliceDepths = 1,
	kVolumes = 2,
	kLights = 3,
	kProjectors = 4,
	kPointSampler = 7,
	kCookies = 8,
	kLinearSampler = 9,
	kInjected = 10,
	kSceneDepth = 12,
};

constexpr std::uint32_t kGroupSize = 8;
constexpr Format kVolumeFormat = Format::kRGBA16Float;

bool Finite( float v )
{
	return std::isfinite( v );
}

bool Finite3( const math::float3 &v )
{
	return Finite( v.x ) && Finite( v.y ) && Finite( v.z );
}

bool NonNegative3( const math::float3 &v )
{
	return Finite3( v ) && v.x >= 0.0f && v.y >= 0.0f && v.z >= 0.0f;
}

bool ValidAnisotropy( float g )
{
	return Finite( g ) && g > -1.0f && g < 1.0f;
}

void Copy3( float out[4], const math::float3 &v, float w )
{
	out[0] = v.x;
	out[1] = v.y;
	out[2] = v.z;
	out[3] = w;
}

template <typename T> std::span<const std::byte> Bytes( const std::vector<T> &values )
{
	return std::as_bytes( std::span<const T>( values ) );
}

// The world-to-view matrix's inverse, in double (a rigid view's inverse is
// its transpose, but a general one is inverted here).
math::float4x4 InverseView( const math::float4x4 &view )
{
	double m[4][8] = {};
	for ( int r = 0; r < 4; ++r )
	{
		const float *row = &view.rows[r].x;
		for ( int c = 0; c < 4; ++c )
			m[r][c] = row[c];
		m[r][4 + r] = 1.0;
	}
	for ( int c = 0; c < 4; ++c )
	{
		int pivot = c;
		for ( int r = c + 1; r < 4; ++r )
		{
			if ( std::fabs( m[r][c] ) > std::fabs( m[pivot][c] ) )
				pivot = r;
		}
		for ( int k = 0; k < 8; ++k )
			std::swap( m[c][k], m[pivot][k] );
		const double inverse = m[c][c] != 0.0 ? 1.0 / m[c][c] : 0.0;
		for ( int k = 0; k < 8; ++k )
			m[c][k] *= inverse;
		for ( int r = 0; r < 4; ++r )
		{
			if ( r == c )
				continue;
			const double f = m[r][c];
			for ( int k = 0; k < 8; ++k )
				m[r][k] -= f * m[c][k];
		}
	}
	math::float4x4 out;
	for ( int r = 0; r < 4; ++r )
	{
		float *row = &out.rows[r].x;
		for ( int c = 0; c < 4; ++c )
			row[c] = static_cast<float>( m[r][4 + c] );
	}
	return out;
}

} // namespace

MediumLight MediumLightFrom( const light_set::RuntimeLight &light )
{
	MediumLight out;
	out.kind = light.shape == light_set::LightShape::Spot ? MediumLightKind::kSpot
	                                                      : MediumLightKind::kPoint;
	out.falloff = light.falloff;
	out.position = { light.position[0], light.position[1], light.position[2] };
	out.direction = { light.direction[0], light.direction[1], light.direction[2] };
	out.color = { light.color[0], light.color[1], light.color[2] };
	out.radius = light.radius;
	out.sourceRadius = light.sourceRadius;
	out.minLight = light.minLight;
	out.innerCos = light.innerCos;
	out.outerCos = light.outerCos;
	out.spotExponent = light.spotExponent;
	return out;
}

bool ValidMedium( const Medium &medium )
{
	if ( !Finite( medium.densityScale ) || medium.densityScale < 0.0f )
		return false;
	const HeightFog &fog = medium.fog;
	if ( !Finite( fog.density ) || fog.density < 0.0f || !Finite( fog.heightDensity ) ||
	     fog.heightDensity < 0.0f || !Finite( fog.heightFalloff ) || !Finite( fog.baseHeight ) ||
	     !Finite( fog.albedo ) || fog.albedo < 0.0f || fog.albedo > 1.0f ||
	     !ValidAnisotropy( fog.anisotropy ) )
		return false;
	for ( const FogVolume &volume : medium.volumes )
	{
		if ( !Finite3( volume.mins ) || !Finite3( volume.maxs ) || !Finite( volume.extinction ) ||
		     volume.extinction < 0.0f || !Finite( volume.albedo ) || volume.albedo < 0.0f ||
		     volume.albedo > 1.0f || !ValidAnisotropy( volume.anisotropy ) ||
		     !NonNegative3( volume.emission ) )
			return false;
	}
	return true;
}

VolumetricParamsGpu PackVolumetricParams( const VolumetricView &view, const VolumetricFrame &frame,
    std::uint32_t lightCount, std::uint32_t volumeCount, std::uint32_t projectorCount )
{
	VolumetricParamsGpu params;
	const FroxelLayout &layout = *view.froxels;
	params.dims[0] = layout.tilesX;
	params.dims[1] = layout.tilesY;
	params.dims[2] = layout.slices;
	params.dims[3] = std::max( frame.sampling.samplesXY, 1u );
	params.counts[0] = lightCount;
	params.counts[1] = volumeCount;
	params.counts[2] = projectorCount;
	params.counts[3] = std::max( frame.sampling.samplesDepth, 1u );
	params.screen[0] = float( layout.widthPixels );
	params.screen[1] = float( layout.heightPixels );
	params.screen[2] = float( layout.tileSizePixels );
	params.screen[3] = frame.medium ? frame.medium->densityScale : 0.0f;
	params.slicing[0] = layout.sliceScale;
	params.slicing[1] = layout.sliceBias;
	params.slicing[2] = layout.nearZ;
	params.slicing[3] = layout.farZ;
	params.rays[0] = layout.rayTopLeft.x;
	params.rays[1] = layout.rayTopLeft.y;
	params.rays[2] = layout.rayBottomRight.x - layout.rayTopLeft.x;
	params.rays[3] = layout.rayBottomRight.y - layout.rayTopLeft.y;
	params.depth[0] = view.projection.rows[2].z;
	params.depth[1] = view.projection.rows[2].w;
	const math::float4x4 toWorld = InverseView( layout.view );
	for ( int r = 0; r < 4; ++r )
		std::memcpy( params.viewToWorld[r], &toWorld.rows[r].x, sizeof( params.viewToWorld[r] ) );
	Copy3( params.eye, view.eye, 1.0f );
	if ( frame.medium )
	{
		const HeightFog &fog = frame.medium->fog;
		params.fog0[0] = fog.density;
		params.fog0[1] = fog.heightDensity;
		params.fog0[2] = fog.heightFalloff;
		params.fog0[3] = fog.baseHeight;
		params.fog1[0] = fog.albedo;
		params.fog1[1] = fog.anisotropy;
	}
	return params;
}

FogVolumeGpu PackFogVolume( const FogVolume &volume )
{
	FogVolumeGpu out;
	Copy3( out.minsExtinction, volume.mins, volume.extinction );
	Copy3( out.maxsAlbedo, volume.maxs, volume.albedo );
	Copy3( out.emissionAnisotropy, volume.emission, volume.anisotropy );
	return out;
}

MediumLightGpu PackMediumLight( const MediumLight &light )
{
	MediumLightGpu out;
	Copy3( out.positionKind, light.position, light.kind == MediumLightKind::kSpot ? 1.0f : 0.0f );
	Copy3( out.colorFalloff, light.color,
	    light.falloff == light_set::LightFalloff::Legacy ? 1.0f : 0.0f );
	Copy3( out.direction, light.direction, 0.0f );
	out.cone[0] = light.innerCos;
	out.cone[1] = light.outerCos;
	out.cone[2] = light.radius;
	out.cone[3] = light.sourceRadius;
	out.misc[0] = light.minLight;
	out.misc[1] = light.spotExponent;
	return out;
}

MediumProjectorGpu PackMediumProjector( const MediumProjector &projector )
{
	return projected_light::PackLightGpu( projector.light, int( projector.cookieLayer ) );
}

foundation::Expected<std::unique_ptr<VolumetricRenderer>, VolumetricStatus>
VolumetricRenderer::Create(
    IRenderDevice2 &device, Format colorFormat, const VolumetricPrograms &programs )
{
	using foundation::MakeUnexpected;
	if ( !device.Facts().capabilities.Has( Capability::kCompute ) )
		return MakeUnexpected( VolumetricStatus::kNoCompute );
	std::unique_ptr<VolumetricRenderer> renderer( new VolumetricRenderer( device ) );

	const ShaderStageSet compute{ ShaderStage::kCompute };
	const ShaderStageSet fragment{ ShaderStage::kFragment };
	const BindingDesc inject[] = { { kParams, BindingKind::kUniformBuffer, 1, compute },
	    { kSliceDepths, BindingKind::kStorageBuffer, 1, compute },
	    { kVolumes, BindingKind::kStorageBuffer, 1, compute },
	    { kLights, BindingKind::kStorageBuffer, 1, compute },
	    { kProjectors, BindingKind::kStorageBuffer, 1, compute },
	    { kCookies, BindingKind::kSampledTexture, 1, compute },
	    { kLinearSampler, BindingKind::kSampler, 1, compute },
	    { kInjected, BindingKind::kStorageTexture, 1, compute } };
	const BindingDesc composite[] = { { kParams, BindingKind::kUniformBuffer, 1, fragment },
	    { kSliceDepths, BindingKind::kStorageBuffer, 1, fragment },
	    { kPointSampler, BindingKind::kSampler, 1, fragment },
	    { kLinearSampler, BindingKind::kSampler, 1, fragment },
	    { kInjected, BindingKind::kSampledTexture, 1, fragment },
	    { kSceneDepth, BindingKind::kSampledTexture, 1, fragment } };
	auto injectLayout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, inject } );
	auto compositeLayout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, composite } );
	if ( injectLayout )
		renderer->m_InjectLayout = injectLayout.Value();
	if ( compositeLayout )
		renderer->m_CompositeLayout = compositeLayout.Value();
	if ( !injectLayout || !compositeLayout )
		return MakeUnexpected( VolumetricStatus::kDevice );

	SamplerDesc samplerDesc;
	samplerDesc.minFilter = samplerDesc.magFilter = samplerDesc.mipFilter = Filter::kNearest;
	samplerDesc.address = AddressMode::kClampToEdge;
	auto point = device.CreateSampler( samplerDesc );
	samplerDesc.minFilter = samplerDesc.magFilter = Filter::kLinear;
	auto linear = device.CreateSampler( samplerDesc );
	if ( point )
		renderer->m_PointSampler = point.Value();
	if ( linear )
		renderer->m_LinearSampler = linear.Value();
	if ( !point || !linear )
		return MakeUnexpected( VolumetricStatus::kDevice );

	// Every cookie white until the caller binds its own: two layers, so the
	// view is an array.
	TextureDesc white;
	white.format = Format::kRGBA8Unorm;
	white.width = white.height = 1;
	white.depthOrLayers = 2;
	white.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	white.debugName = "render.pass.volumetric.white-cookies";
	auto whiteTexture = device.CreateTexture( white );
	if ( !whiteTexture )
		return MakeUnexpected( VolumetricStatus::kDevice );
	renderer->m_WhiteCookies = whiteTexture.Value();

	// The stages in the device's artifact format (RFC 0016 K10); a suite's
	// seeded stage replaces the core one, on a SPIR-V device only.
	shaderlib::ArtifactOverlay artifacts( shaderlib::CoreArtifacts() );
	const ArtifactFormat format = device.Facts().artifactFormat;
	for ( const auto &[source, code] : { std::pair{ kInjectSource, programs.inject },
	          std::pair{ kCompositeSource, programs.composite } } )
	{
		if ( !code.empty() && !artifacts.ReplaceSpirv( source, code, format ) )
			return MakeUnexpected( VolumetricStatus::kDevice );
	}
	const auto computePipeline = [&]( const char *source, BindGroupLayoutId layout,
	                                 const char *name ) -> std::optional<PipelineId>
	{
		shaderlib::PipelineRecipe recipe =
		    shaderlib::CoreRecipe( { source }, PipelineKind::kCompute );
		recipe.layouts = { {}, {}, {}, layout };
		recipe.debugName = name;
		auto resolved = shaderlib::Resolve( recipe, artifacts, format );
		if ( !resolved )
			return std::nullopt;
		auto pipeline = device.CreatePipeline( resolved.Value().Desc() );
		if ( !pipeline )
			return std::nullopt;
		return pipeline.Value();
	};
	const std::optional<PipelineId> injectPipeline =
	    computePipeline( kInjectSource, renderer->m_InjectLayout, "render.pass.volumetric.inject" );
	if ( injectPipeline )
		renderer->m_Inject = *injectPipeline;
	if ( !injectPipeline )
		return MakeUnexpected( VolumetricStatus::kDevice );

	shaderlib::PipelineRecipe recipe =
	    shaderlib::CoreRecipe( { kCompositeVertex, kCompositeSource } );
	recipe.layouts = { {}, {}, {}, renderer->m_CompositeLayout };
	recipe.topology = PrimitiveTopology::kTriangleList;
	recipe.raster.cull = CullMode::kNone;
	recipe.colorFormats = { colorFormat };
	// src + dst * a with the source ( L, T ): dst T + L, T at the target's
	// precision (render.device.v2 D21).
	recipe.blends = { BlendMode::kTransmittance };
	recipe.debugName = "render.pass.volumetric.composite";
	auto resolved = shaderlib::Resolve( recipe, artifacts, format );
	if ( !resolved )
		return MakeUnexpected( VolumetricStatus::kDevice );
	PipelineDesc desc = resolved.Value().Desc();
	// The frame's alpha is not the medium's to change.
	const std::uint8_t masks[] = { kColorWriteRed | kColorWriteGreen | kColorWriteBlue };
	desc.colorWriteMasks = masks;
	auto pipeline = device.CreatePipeline( desc );
	if ( !pipeline )
		return MakeUnexpected( VolumetricStatus::kDevice );
	renderer->m_Composite = pipeline.Value();
	return renderer;
}

VolumetricRenderer::~VolumetricRenderer()
{
	std::lock_guard<std::mutex> lock( m_PendingLock );
	for ( const ResourceId &resource : m_Pending )
		(void)m_Device.Release( resource, m_LastToken );
	const ResourceId owned[] = { m_Inject, m_Composite, m_InjectLayout, m_CompositeLayout,
	    m_PointSampler, m_LinearSampler, m_WhiteCookies };
	for ( const ResourceId &resource : owned )
	{
		if ( resource.value != 0 )
			(void)m_Device.Release( resource, m_LastToken );
	}
}

foundation::Expected<VolumetricStats, VolumetricStatus> VolumetricRenderer::Record(
    CommandEncoder &encoder, const VolumetricView &view, const VolumetricFrame &frame,
    const VolumetricTargets &targets )
{
	using foundation::MakeUnexpected;
	const FroxelLayout *layout = view.froxels;
	if ( !layout || layout->tilesX == 0 || layout->tilesY == 0 || layout->slices == 0 ||
	     layout->tileSizePixels == 0 ||
	     layout->sliceDepths.size() != std::size_t( layout->slices ) + 1 ||
	     layout->widthPixels != targets.width || layout->heightPixels != targets.height ||
	     !targets.color.IsValid() || !targets.depth.IsValid() )
		return MakeUnexpected( VolumetricStatus::kInvalidGrid );
	static const Medium kNoMedium;
	const Medium &medium = frame.medium ? *frame.medium : kNoMedium;
	if ( !ValidMedium( medium ) )
		return MakeUnexpected( VolumetricStatus::kInvalidMedium );
	if ( medium.volumes.size() > kMaxFogVolumes || frame.lights.size() > kMaxMediumLights ||
	     frame.projectors.size() > kMaxMediumProjectors )
		return MakeUnexpected( VolumetricStatus::kInvalidFrame );

	// The records, each list at least one long so every binding has a size.
	std::vector<FogVolumeGpu> volumes;
	for ( const FogVolume &volume : medium.volumes )
		volumes.push_back( PackFogVolume( volume ) );
	std::vector<MediumLightGpu> lights;
	for ( const MediumLight &light : frame.lights )
		lights.push_back( PackMediumLight( light ) );
	std::vector<MediumProjectorGpu> projectors;
	for ( const MediumProjector &projector : frame.projectors )
		projectors.push_back( PackMediumProjector( projector ) );
	VolumetricFrame packed = frame;
	packed.medium = &medium;
	const VolumetricParamsGpu params =
	    PackVolumetricParams( view, packed, std::uint32_t( lights.size() ),
	        std::uint32_t( volumes.size() ), std::uint32_t( projectors.size() ) );
	volumes.resize( std::max<std::size_t>( volumes.size(), 1 ) );
	lights.resize( std::max<std::size_t>( lights.size(), 1 ) );
	projectors.resize( std::max<std::size_t>( projectors.size(), 1 ) );
	const std::vector<float> depths( layout->sliceDepths.begin(), layout->sliceDepths.end() );

	std::vector<ResourceId> made;
	const auto fail = [&]()
	{
		for ( const ResourceId &resource : made )
			(void)m_Device.Release( resource, m_LastToken );
		return MakeUnexpected( VolumetricStatus::kDevice );
	};
	const auto buffer = [&]( std::span<const std::byte> bytes, ResourceUsage use ) -> BufferId
	{
		BufferDesc desc;
		desc.size = bytes.size();
		desc.usages = { ResourceUsage::kCopyDestination, use };
		desc.debugName = "render.pass.volumetric.input";
		auto created = m_Device.CreateBuffer( desc );
		if ( !created )
			return BufferId();
		made.push_back( created.Value() );
		encoder.TransitionBuffer(
		    created.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.WriteBuffer( created.Value(), 0, bytes );
		encoder.TransitionBuffer( created.Value(), ResourceUsage::kCopyDestination, use );
		return created.Value();
	};
	const BufferId paramsBuffer =
	    buffer( std::as_bytes( std::span( &params, 1 ) ), ResourceUsage::kUniform );
	const BufferId depthsBuffer = buffer( Bytes( depths ), ResourceUsage::kStorageRead );
	const BufferId volumesBuffer = buffer( Bytes( volumes ), ResourceUsage::kStorageRead );
	const BufferId lightsBuffer = buffer( Bytes( lights ), ResourceUsage::kStorageRead );
	const BufferId projectorsBuffer = buffer( Bytes( projectors ), ResourceUsage::kStorageRead );
	if ( !paramsBuffer.IsValid() || !depthsBuffer.IsValid() || !volumesBuffer.IsValid() ||
	     !lightsBuffer.IsValid() || !projectorsBuffer.IsValid() )
		return fail();

	if ( !m_WhiteStaged )
	{
		const std::byte white[4] = {
		    std::byte( 255 ), std::byte( 255 ), std::byte( 255 ), std::byte( 255 ) };
		BufferDesc desc;
		desc.size = sizeof( white );
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource };
		auto staging = m_Device.CreateBuffer( desc );
		if ( !staging )
			return fail();
		made.push_back( staging.Value() );
		encoder.TransitionBuffer(
		    staging.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.WriteBuffer( staging.Value(), 0, white );
		encoder.TransitionBuffer(
		    staging.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
		encoder.TransitionTexture( m_WhiteCookies, ResourceUsage::kUndefined,
		    ResourceUsage::kCopyDestination, { 0, 1, 0, 2 } );
		for ( std::uint32_t layer = 0; layer < 2; ++layer )
			encoder.CopyBufferToTexture( staging.Value(), m_WhiteCookies, { 0, 0, layer, 1, 1 } );
		encoder.TransitionTexture( m_WhiteCookies, ResourceUsage::kCopyDestination,
		    ResourceUsage::kSampled, { 0, 1, 0, 2 } );
		m_WhiteStaged = true;
	}

	TextureDesc volumeDesc;
	volumeDesc.dimension = TextureDimension::k3D;
	volumeDesc.format = kVolumeFormat;
	volumeDesc.width = layout->tilesX;
	volumeDesc.height = layout->tilesY;
	volumeDesc.depthOrLayers = layout->slices;
	volumeDesc.usages = { ResourceUsage::kStorageWrite, ResourceUsage::kSampled };
	volumeDesc.debugName = "render.pass.volumetric.injected";
	auto injected = m_Device.CreateTexture( volumeDesc );
	if ( !injected )
		return fail();
	made.push_back( injected.Value() );

	const TextureId cookies = targets.cookies.IsValid() ? targets.cookies : m_WhiteCookies;
	const BindGroupEntry injectEntries[] = {
	    { kParams, paramsBuffer, 0, sizeof( VolumetricParamsGpu ), {}, {} },
	    { kSliceDepths, depthsBuffer, 0, 0, {}, {} }, { kVolumes, volumesBuffer, 0, 0, {}, {} },
	    { kLights, lightsBuffer, 0, 0, {}, {} }, { kProjectors, projectorsBuffer, 0, 0, {}, {} },
	    { kCookies, {}, 0, 0, cookies, {} }, { kLinearSampler, {}, 0, 0, {}, m_LinearSampler },
	    { kInjected, {}, 0, 0, injected.Value(), {} } };
	const BindGroupEntry compositeEntries[] = {
	    { kParams, paramsBuffer, 0, sizeof( VolumetricParamsGpu ), {}, {} },
	    { kSliceDepths, depthsBuffer, 0, 0, {}, {} },
	    { kPointSampler, {}, 0, 0, {}, m_PointSampler },
	    { kLinearSampler, {}, 0, 0, {}, m_LinearSampler },
	    { kInjected, {}, 0, 0, injected.Value(), {} },
	    { kSceneDepth, {}, 0, 0, targets.depth, {} } };
	auto injectGroup = m_Device.CreateBindGroup( { m_InjectLayout, injectEntries } );
	if ( injectGroup )
		made.push_back( injectGroup.Value() );
	auto compositeGroup = m_Device.CreateBindGroup( { m_CompositeLayout, compositeEntries } );
	if ( compositeGroup )
		made.push_back( compositeGroup.Value() );
	if ( !injectGroup || !compositeGroup )
		return fail();

	// Inject.
	encoder.TransitionTexture(
	    injected.Value(), ResourceUsage::kUndefined, ResourceUsage::kStorageWrite );
	encoder.SetPipeline( m_Inject );
	encoder.SetBindGroup( BindGroupRole::kDraw, injectGroup.Value() );
	encoder.Dispatch( ( layout->tilesX + kGroupSize - 1 ) / kGroupSize,
	    ( layout->tilesY + kGroupSize - 1 ) / kGroupSize, layout->slices );
	// Composite.
	encoder.TransitionTexture(
	    injected.Value(), ResourceUsage::kStorageWrite, ResourceUsage::kSampled );
	if ( targets.depthUsage != ResourceUsage::kSampled )
		encoder.TransitionTexture( targets.depth, targets.depthUsage, ResourceUsage::kSampled );
	ColorAttachment color;
	color.texture = targets.color;
	color.load = LoadOp::kLoad;
	color.store = StoreOp::kStore;
	const ColorAttachment colors[] = { color };
	RenderingDesc rendering;
	rendering.colors = colors;
	rendering.width = targets.width;
	rendering.height = targets.height;
	encoder.BeginRendering( rendering );
	encoder.SetViewport(
	    { 0.0f, 0.0f, float( targets.width ), float( targets.height ), 0.0f, 1.0f } );
	encoder.SetPipeline( m_Composite );
	encoder.SetBindGroup( BindGroupRole::kDraw, compositeGroup.Value() );
	encoder.Draw( 3 );
	encoder.EndRendering();
	if ( targets.depthUsage != ResourceUsage::kSampled )
		encoder.TransitionTexture( targets.depth, ResourceUsage::kSampled, targets.depthUsage );

	{
		std::lock_guard<std::mutex> lock( m_PendingLock );
		m_Pending.insert( m_Pending.end(), made.begin(), made.end() );
	}
	VolumetricStats stats;
	stats.froxels = layout->tilesX * layout->tilesY * layout->slices;
	stats.lights = std::uint32_t( frame.lights.size() );
	stats.projectors = std::uint32_t( frame.projectors.size() );
	stats.volumes = std::uint32_t( medium.volumes.size() );
	return stats;
}

void VolumetricRenderer::Collect( CompletionToken token )
{
	std::lock_guard<std::mutex> lock( m_PendingLock );
	for ( const ResourceId &resource : m_Pending )
		(void)m_Device.Release( resource, token );
	m_Pending.clear();
	m_LastToken = token;
}

} // namespace render::pass::volumetric
