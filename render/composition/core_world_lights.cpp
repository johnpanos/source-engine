//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's world (RFC 0016 K5): a view's lights, area lights and projector cookies.
//
//=============================================================================//

#include "core_world_internal.h"

namespace render::composition
{

namespace
{

// The game's files through the host's RenderCoreFileSource.
class HostFiles final : public mdl::IModelFiles
{
public:
	explicit HostFiles( const RenderCoreFileSource &source ) : m_Source( source ) {}
	bool Exists( const std::string &path ) const override
	{
		return m_Source.size && m_Source.size( path.c_str() ) != 0;
	}
	bool Read( const std::string &path, std::string &out ) const override
	{
		if ( !m_Source.size || !m_Source.read )
			return false;
		const unsigned long long bytes = m_Source.size( path.c_str() );
		if ( bytes == 0 || bytes > ( 256ull << 20 ) )
			return false;
		out.resize( std::size_t( bytes ) );
		return m_Source.read( path.c_str(), out.data(), bytes );
	}

private:
	RenderCoreFileSource m_Source;
};

} // namespace

void CoreWorld::RefreshCookies()
{
	std::vector<std::string> names;
	for ( const light_set::RuntimeProjectedLight &projected : m_Lights.projected )
	{
		const std::string cookie = projected.light.cookie;
		if ( !cookie.empty() && std::find( names.begin(), names.end(), cookie ) == names.end() )
			names.push_back( cookie );
	}
	if ( names == m_CookieNames && ( m_CookieImages || !m_CookieRefusal.empty() ) )
		return;
	m_CookieNames = names;
	m_CookieImages.reset();
	m_CookieRefusal.clear();
	if ( m_Lights.projected.empty() )
	{
		m_ProjectorsLit = 0;
		return;
	}
	if ( !m_Files.size || !m_Files.read )
		m_CookieRefusal = "no file source for projector cookies";
	else
	{
		auto images =
		    DecodeCookies( HostFiles( m_Files ), names, texturecontainer::vtf::Decompress );
		if ( images )
			m_CookieImages = std::make_shared<const CookieImages>( std::move( images ).Value() );
		else
			m_CookieRefusal = images.Error();
	}
	m_ProjectorsLit = m_CookieImages ? unsigned( m_Lights.projected.size() ) : 0u;
	if ( !m_CookieImages )
	{
		m_ProjectorsRefused.fetch_add( m_Lights.projected.size(), std::memory_order_relaxed );
		std::fprintf( stderr, "render core: %zu projected light(s) refused: %s\n",
		    m_Lights.projected.size(), m_CookieRefusal.c_str() );
	}
}

CoreWorld::ViewLightInputs CoreWorld::TakeViewLightInputs(
    const float worldToView[16], const float viewToClip[16], const float viewport[6] ) const
{
	ViewLightInputs in;
	if ( worldToView )
		std::copy( worldToView, worldToView + 16, in.worldToView );
	if ( viewToClip )
		std::copy( viewToClip, viewToClip + 16, in.viewToClip );
	if ( viewport )
		std::copy( viewport, viewport + 6, in.viewport );
	// A relit BSP retains switchable world lights but strips its baked ones.
	// Merge by individual lamp so that one live light does not hide the map's
	// always-on direct lights from the model point.
	in.stageWorld = m_StageSet;
	if ( in.stageWorld )
		in.lights = pass::lights::MergeMapLights( m_Lights.lights, m_MapLights );
	// The area lights: the map's (baked light fixtures) and the frame's
	// emitting surfaces; the sun: the map's.
	in.areas = ViewAreaLights( in.stageWorld );
	in.mapAreas = in.stageWorld ? std::min( m_MapLights.areas.size(), in.areas.size() ) : 0;
	if ( in.stageWorld )
		in.sun = m_MapLights.sun;
	// The frame's projected lights, each with its cookie's layer (the white
	// layer after the named ones for a projector without a cookie).
	if ( in.stageWorld && m_CookieImages )
	{
		in.cookies = m_CookieImages;
		const std::vector<std::string> &names = m_CookieImages->names;
		for ( const light_set::RuntimeProjectedLight &projected : m_Lights.projected )
		{
			const std::string cookie = projected.light.cookie;
			const auto found = std::find( names.begin(), names.end(), cookie );
			in.projectors.push_back( projected.light );
			in.cookieLayers.push_back( cookie.empty() || found == names.end()
			                               ? int( names.size() )
			                               : int( found - names.begin() ) );
		}
	}
	in.sunMask = m_StageSunMask;
	in.maskLights = m_StageMaskLights;
	in.shadowQuality = m_ShadowQuality.load( std::memory_order_relaxed );
	if ( in.shadowQuality > 0 && m_ShadowMovers.load( std::memory_order_relaxed ) )
	{
		in.movers = m_Lights.occluders;
		in.triangles = m_Lights.triangles;
	}
	return in;
}

std::shared_ptr<const pass::world::StageViewLights> CoreWorld::StageViewLightsFor(
    const ViewLightInputs &in, std::shared_ptr<const ShadowWork> *shadows,
    device::CommandEncoder &encoder )
{
	*shadows = nullptr;
	const float *worldToView = in.worldToView;
	const float *viewToClip = in.viewToClip;
	const float *viewport = in.viewport;
	if ( viewport[2] < 1.0f || viewport[3] < 1.0f )
		return nullptr;
	auto matrix = []( const float m[16] )
	{
		math::float4x4 out;
		for ( int r = 0; r < 4; ++r )
			out.rows[r] = { m[r * 4 + 0], m[r * 4 + 1], m[r * 4 + 2], m[r * 4 + 3] };
		return out;
	};
	pass::lights::ClusterViewDesc desc;
	desc.view = matrix( worldToView );
	desc.projection = matrix( viewToClip );
	desc.widthPixels = std::uint32_t( viewport[2] );
	desc.heightPixels = std::uint32_t( viewport[3] );
	// The depth range from the projection (depth 0 at the near plane, 1 at
	// the far one, clip w = -view z): near = m23 / m22, far = m23 / (m22 + 1).
	const float a = viewToClip[2 * 4 + 2];
	const float b = viewToClip[2 * 4 + 3];
	desc.nearZ = a != 0.0f ? b / a : 0.0f;
	desc.farZ = a + 1.0f != 0.0f ? b / ( a + 1.0f ) : 0.0f;
	if ( !std::isfinite( desc.farZ ) || desc.farZ <= desc.nearZ )
		desc.farZ = 65536.0f;
	const pass::lights::ClusterLimits limits = pass::lights::DesktopClusterLimits();
	auto grid = pass::lights::CreateClusterGrid( desc, limits );
	if ( !grid )
		return nullptr;
	const std::vector<light_set::RuntimeLight> &frameLights = in.lights;
	auto dispatch =
	    pass::lights::PrepareSurfaceClusterDispatch( grid.Value(), frameLights, in.areas );
	auto assigned = m_ClusterKernel->RecordView( encoder, dispatch );
	if ( !assigned )
		return nullptr;
	auto out = std::make_shared<pass::world::StageViewLights>();
	out->view.grid[0] = grid.Value().tilesX;
	out->view.grid[1] = grid.Value().tilesY;
	out->view.grid[2] = grid.Value().slices;
	out->view.grid[3] = limits.tileSizePixels;
	out->view.slices[0] = grid.Value().sliceScale;
	out->view.slices[1] = grid.Value().sliceBias;
	out->view.slices[2] = grid.Value().nearZ;
	out->view.slices[3] = m_ShadowPcss.load( std::memory_order_relaxed ) ? 0.0f : 1.0f;
	out->view.counts[2] = viewport[0];
	out->view.counts[3] = viewport[1];
	const math::float4 &z = desc.view.rows[2];
	out->view.viewDistance[0] = -z.x;
	out->view.viewDistance[1] = -z.y;
	out->view.viewDistance[2] = -z.z;
	out->view.viewDistance[3] = -z.w;
	out->gpuFroxels = assigned.Value().froxels;
	out->gpuIndices = assigned.Value().indices;
	// Shadow planning uses conservative whole-view visibility, never a GPU-list
	// readback. Fine per-froxel membership belongs exclusively to the GPU.
	std::vector<char> reaches( frameLights.size(), 0 );
	for ( std::size_t i = 0; i < dispatch.lightCount; ++i )
	{
		const auto &light = dispatch.lights[i];
		const math::float3 p{
		    light.positionRadius[0], light.positionRadius[1], light.positionRadius[2] };
		const float r =
		    light.positionRadius[3] + 4e-6f * ( math::Length( p ) + light.positionRadius[3] );
		const auto &g = grid.Value();
		if ( g.columnPlanes.front().Distance( p ) >= -r &&
		     g.columnPlanes.back().Distance( p ) <= r && g.rowPlanes.front().Distance( p ) >= -r &&
		     g.rowPlanes.back().Distance( p ) <= r && -p.z + r >= g.nearZ && -p.z - r <= g.farZ )
			reaches[dispatch.lightSetIndex[i]] = 1;
	}
	std::vector<light_set::RuntimeLight> shadowed;
	std::vector<int> shadowedOf( frameLights.size(), -1 );
	for ( std::size_t i = 0; i < frameLights.size(); ++i )
	{
		const light_set::RuntimeLight &light = frameLights[i];
		if ( reaches[i] && ( light.shape == light_set::LightShape::Point ||
		                       light.shape == light_set::LightShape::Spot ) )
		{
			shadowedOf[i] = int( shadowed.size() );
			shadowed.push_back( light );
		}
	}
	// The area lights: the map's (baked light fixtures) and the frame's
	// emitting surfaces; the sun: the map's.
	const std::vector<area_light::AreaLight> &areas = in.areas;
	const std::optional<pass::lights::MapSun> &sun = in.sun;
	pass::shadows::ShadowPlan plan;
	if ( ( !shadowed.empty() || !areas.empty() || sun || !in.projectors.empty() ) &&
	     in.shadowQuality > 0 )
	{
		pass::shadows::ShadowPlanInput input;
		input.lights = shadowed;
		input.areas = areas;
		input.projectors = in.projectors;
		if ( sun )
			input.toSun = sun->toSun;
		input.camera.view = desc.view;
		// The cascades cover the view's frustum: its vertical field of view
		// and aspect from the projection's scales.
		const float yScale = desc.projection.rows[1].y;
		const float xScale = desc.projection.rows[0].x;
		if ( yScale > 0.0f && xScale > 0.0f )
		{
			input.camera.verticalFovRadians = 2.0f * std::atan( 1.0f / yScale );
			input.camera.aspect = yScale / xScale;
		}
		input.camera.nearZ = desc.nearZ;
		input.atlasSize = ShadowAtlasFor( in.shadowQuality );
		input.guardTexels = 4;
		if ( pass::shadows::PlanShadows( input, plan ) )
			plan = pass::shadows::ShadowPlan(); // refused: unshadowed
		m_ShadowLightsUnshadowed.fetch_add( plan.unshadowed, std::memory_order_relaxed );
	}
	// A baked light's diffuse light is in the lightmap: the core adds its
	// specular lobe alone (each light counts once per surface).
	auto work = std::make_shared<ShadowWork>();
	work->view = desc.view;
	// The projection the world pass rasterizes with: D3D9 pixel centers, a
	// half pixel right and down (render.pass.world), so the screen passes
	// reconstruct each pixel where it was drawn.
	work->projection = desc.projection;
	if ( const auto fromView = math::Inverse( desc.view ) )
	{
		work->eye[0] = fromView->rows[0].w;
		work->eye[1] = fromView->rows[1].w;
		work->eye[2] = fromView->rows[2].w;
	}
	for ( int c = 0; c < 4; ++c )
	{
		const float w = ( &desc.projection.rows[3].x )[c];
		( &work->projection.rows[0].x )[c] += w / viewport[2];
		( &work->projection.rows[1].x )[c] -= w / viewport[3];
	}
	// The view's moving casters as spheres, grown by a margin for the soft
	// filter's reach beyond the exact penumbra (its least width and the
	// tile's texel size at the receiver).
	for ( std::size_t m = 0; m < in.movers.size() && m < material::kSurfaceMaxViewMovers; ++m )
	{
		const light_set::RuntimeOccluder &mover = in.movers[m];
		float reach = 0.0f;
		for ( int a = 0; a < 3; ++a )
			for ( int k = 0; k < 3; ++k )
				reach += mover.box.axes[a][k] * mover.box.axes[a][k];
		for ( int k = 0; k < 3; ++k )
			out->view.movers[m][k] = mover.box.center[k];
		out->view.movers[m][3] = std::sqrt( reach ) * 1.25f + 8.0f;
	}
	for ( std::size_t i = 0; i < frameLights.size(); ++i )
	{
		const light_set::RuntimeLight &light = frameLights[i];
		const int k = shadowedOf[i];
		const int tile =
		    k >= 0 && std::size_t( k ) < plan.lightTiles.size() ? plan.lightTiles[k] : -1;
		const RuntimeShadowLayout layout =
		    tile >= 0 ? plan.lightLayouts[k] : RuntimeShadowLayout::kSingle;
		// A static light with a baked shadow mask (its record at the light's
		// origin): its static shadow is the mask's; its tile still serves
		// moving casters in its reach.
		int maskId = -1;
		bool moverInReach = false;
		if ( in.maskLights && light.kind == light_set::LightKind::World )
			for ( const mapcontainer::LightShadowMaskRecord &record : *in.maskLights )
				if ( std::fabs( record.origin[0] - light.position[0] ) <=
				         mapcontainer::kLightShadowMasksMatchUnits &&
				     std::fabs( record.origin[1] - light.position[1] ) <=
				         mapcontainer::kLightShadowMasksMatchUnits &&
				     std::fabs( record.origin[2] - light.position[2] ) <=
				         mapcontainer::kLightShadowMasksMatchUnits )
				{
					maskId = int( record.id );
					break;
				}
		// The moving casters in reach, as bits naming the view's mover
		// spheres (the shader reads the tile only where a pixel's path to the
		// light passes near one of them); a mover past the view's spheres
		// makes every point read it.
		std::uint32_t moverBits = 0;
		if ( maskId > 0 && tile >= 0 )
			for ( std::size_t m = 0; m < in.movers.size(); ++m )
			{
				const light_set::RuntimeOccluder &mover = in.movers[m];
				float reach = 0.0f, distance = 0.0f;
				for ( int a = 0; a < 3; ++a )
				{
					for ( int k = 0; k < 3; ++k )
						reach += mover.box.axes[a][k] * mover.box.axes[a][k];
					const float d = mover.box.center[a] - light.position[a];
					distance += d * d;
				}
				const float limit = std::sqrt( reach ) + light.radius;
				if ( light.radius > 0.0f && distance >= limit * limit )
					continue;
				moverInReach = true;
				moverBits |= m < material::kSurfaceMaxViewMovers
				                 ? 1u << m
				                 : material::kSurfaceMoversEverywhere;
			}
		( maskId <= 0      ? m_UnmaskedLights
		    : moverInReach ? m_MaskedMoverLights
		                   : m_MaskedLights )
		    .fetch_add( 1, std::memory_order_relaxed );
		if ( maskId <= 0 && light.kind == light_set::LightKind::World )
			m_UnmaskedWorldLights.fetch_add( 1, std::memory_order_relaxed );
		out->lights.push_back( material::PackSurfaceLight(
		    light, tile, layout, light.baked, maskId, moverInReach, moverBits ) );
	}
	// The projected lights, with their cookie layers and tiles.
	for ( std::size_t i = 0; i < in.projectors.size(); ++i )
		out->projectors.push_back( projected_light::PackLightGpu( in.projectors[i],
		    in.cookieLayers[i], i < plan.projectorTiles.size() ? plan.projectorTiles[i] : -1 ) );
	out->view.counts[0] = float( out->projectors.size() );
	if ( m_Cookies && m_CookiesUploaded == in.cookies )
	{
		out->cookies = m_Cookies->Texture();
		out->cookiesDesc = m_Cookies->Desc();
	}
	// The area lights and the sun, with their tiles.
	PackViewAreaLights( areas, in.mapAreas, plan.areaTiles, *out );
	if ( sun )
	{
		out->sunDirection[0] = sun->toSun.x;
		out->sunDirection[1] = sun->toSun.y;
		out->sunDirection[2] = sun->toSun.z;
		out->sunDirection[3] =
		    float( std::tan( 0.5 * double( sun->spreadDegrees ) * 3.14159265358979 / 180.0 ) );
		out->sunColor[0] = sun->color.x;
		out->sunColor[1] = sun->color.y;
		out->sunColor[2] = sun->color.z;
		out->sunColor[3] = 1.0f;
		out->sunShadow[0] = float( plan.sunFirst );
		out->sunShadow[1] = float( plan.sunCount );
		out->sunShadow[2] = in.sunMask ? 1.0f : 0.0f;
	}
	// The view group's light records: at least one (an empty view's lists
	// name none of them).
	if ( out->lights.empty() )
		out->lights.emplace_back();
	if ( !plan.views.empty() )
	{
		out->shadowTiles = std::move( plan.tiles );
		work->views = std::move( plan.views );
		work->sunFirst = plan.sunFirst;
		work->sunCount = plan.sunCount;
		work->movers = in.movers;
		work->triangles = in.triangles;
		work->atlasSize = plan.atlasSize;
		work->guardTexels = plan.guardTexels;
	}
	*shadows = std::move( work );
	return out;
}

std::vector<area_light::AreaLight> CoreWorld::ViewAreaLights( bool withMapAreas ) const
{
	std::vector<area_light::AreaLight> areas;
	// Runtime (LTC) area lights are opt-in (r_core_area_lights).
	if ( !m_AreaLightsOn.load( std::memory_order_relaxed ) )
		return areas;
	if ( withMapAreas )
		areas = m_MapLights.areas;
	for ( const light_set::RuntimeAreaLight &area : m_Lights.areas )
		areas.push_back( area.light );
	if ( areas.size() > std::size_t( material::kSurfaceMaxAreaLights ) )
		areas.resize( std::size_t( material::kSurfaceMaxAreaLights ) );
	return areas;
}

void CoreWorld::PackViewAreaLights( const std::vector<area_light::AreaLight> &areas,
    std::size_t mapAreas, const std::vector<int> &areaTiles, pass::world::StageViewLights &out )
{
	// A map's light fixture is in the bake (the surface program adds its
	// specular alone); an emitting surface is not: the engine leaves the
	// surfaces the core draws out of its lightmap (area_lights.h), and a
	// stage's lightmap never held it.
	for ( std::size_t i = 0; i < areas.size(); ++i )
		out.areas.push_back( material::PackAreaLight(
		    areas[i], i < mapAreas, i < areaTiles.size() ? areaTiles[i] : -1 ) );
}

} // namespace render::composition
