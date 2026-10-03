//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.world-panel.v1 (RFC 0016 and RFC 0010's in-world panel
//			capability): a UI panel placed in the world (a vgui_screen:
//			Portal 2's chamber sign) as an emitting surface. The one
//			definition of:
//			- the panel's image: a draw list, textured and colored quads in
//			  the panel's layout units (origin top-left, y down). Its
//			  emissive quads (the lit board, its text and icons) composite
//			  in paint order over opaque black, in gamma space (the legacy 2D
//			  path's blending): an opaque quad replaces, an alpha quad blends
//			  with its alpha, an additive quad adds its color times alpha.
//			  Its coating quads (grime on the screen's face) emit nothing.
//			  They composite among themselves, in paint order, as
//			  premultiplied linear color over nothing: coverage A.a and color
//			  A.rgb (a coating of alpha a and linear color c gives a and a c).
//			  The coating is a translucent diffuser on the face: its clear
//			  part (1 - A.a) passes the emission under it, and its particles
//			  scatter, tinted by A.rgb, the light that reaches them from the lit
//			  board around them (the scatter field below: the emission's mean
//			  over a box of about 36 units), so grime over a dark print catches the light around it as
//			  real dust on a lit screen does, whatever it is painted over or
//			  under. A.rgb is also the face's diffuse albedo, lit by the scene
//			  like any surface. render.pass.panels
//			  rasterizes it on the GPU; TileRadiance below integrates it on
//			  the CPU;
//			- the image's resolution (PixelsPerUnit, ChooseResolution): at
//			  least one texel per screen pixel where the panel is densest on
//			  screen, so what the toolkit paints for that resolution (text
//			  rasterized at it) is never magnified;
//			- what the panel emits and casts: EmissionAt times emissionScale
//			  (an emissive image value v decoded from sRGB, with no coating,
//			  emits emissionScale * v), as the surface's emission and as its RFC
//			  0011 area lights (render.area-light.v1). The lights are a grid of
//			  the panel's tiles (Tiles), each with the mean emission of its
//			  part of the image (TileRadiance), so grime on a lit board dims
//			  the light it casts from where the grime is, as a real dirty
//			  screen does, and never adds light of its own. Surface and lights
//			  come from the same frame's list: they show and cast one state.
//
//			Header-only and plain (no render namespace, no device types), so
//			the client, the engine, the UI surface and the core share it.
//
//=============================================================================//

#ifndef RENDER_WORLD_PANEL_H
#define RENDER_WORLD_PANEL_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace world_panel
{

enum Blend : std::uint8_t
{
	kBlendOpaque = 0, // the texture's alpha is not read; the quad replaces
	kBlendAlpha = 1,  // straight alpha over what is there
	kBlendAdditive = 2
};

// A quad's texture when it has none: white.
constexpr std::uint32_t kWhite = 0xFFFFFFFFu;

enum Layer : std::uint8_t
{
	kLayerEmissive = 0, // part of the lit image
	kLayerCoating = 1   // on the face: blocks the emission under it, reflects light, emits none
};

struct Quad
{
	float x0, y0, x1, y1;  // panel units: top-left and bottom-right corners
	float s0, t0, s1, t1;  // texture coordinates at (x0, y0) and (x1, y1)
	std::uint8_t color[4]; // gamma RGBA, straight alpha, modulating the texture
	std::uint32_t texture; // into the list's textures, or kWhite
	std::uint8_t blend;    // Blend (an emissive quad's; a coating blends by its alpha)
	std::uint8_t layer;    // Layer
	// For a texture whose texels only the GPU holds (a font's glyph page):
	// the mean alpha of the quad's texels, which TileRadiance uses in place
	// of sampling it; negative when unknown.
	float coverage;
};

// A rectangular board can use the opaque physical surface even when its
// legacy host sorted it with translucent UI. Require a full-face opaque
// foundation; overlays cannot punch holes in the recorded blend operations.
// Clear displays use the same family's alpha surface and recorded coverage.
inline bool CoversFace( const Quad *quads, std::uint32_t count, float wide, float tall )
{
	if ( !quads || !( wide > 0.0f ) || !( tall > 0.0f ) )
		return false;
	for ( std::uint32_t i = 0; i < count; ++i )
	{
		const Quad &q = quads[i];
		if ( q.layer == kLayerEmissive && q.x0 <= 0.0f && q.y0 <= 0.0f && q.x1 >= wide &&
		     q.y1 >= tall &&
		     ( q.blend == kBlendOpaque || ( q.blend == kBlendAlpha && q.color[3] == 255 &&
		                                      ( q.texture == kWhite || q.coverage == 1.0f ) ) ) )
			return true;
	}
	return false;
}

// A draw list as the owner holds it: `quads` in paint order, `textures`
// the keys its quads name (the material system's texture handles in the
// product).
struct DrawListView
{
	float unitsWide;
	float unitsTall;
	const Quad *quads;
	std::uint32_t quadCount;
	const int *textures;
	std::uint32_t textureCount;
};

// The world position of the panel's top-left corner, and the world vectors
// across its whole width and down its whole height. Its front faces along
// cross( down, right ).
struct Placement
{
	float origin[3];
	float right[3];
	float down[3];
};

// The image's size and its texel density (texels per panel unit, the same
// along both axes).
struct Resolution
{
	std::uint32_t width;
	std::uint32_t height;
	float texelsPerUnit;
};

inline bool operator==( const Resolution &a, const Resolution &b )
{
	return a.width == b.width && a.height == b.height && a.texelsPerUnit == b.texelsPerUnit;
}

// The resolution policy's constants.
constexpr float kDensityStep = 1.18920712f; // 2^(1/4): the ladder's step
constexpr float kDensityHold =
    1.41421356f; // a density is kept while at most this much denser than needed
constexpr float kMinTexelsPerUnit = 0.25f;    // what a far panel keeps
constexpr std::uint32_t kMaxDimension = 4096; // texels, either axis

// The screen pixels one panel unit covers where the panel is densest on
// screen (the larger of its two axes), over the part of the panel inside
// the view (a grid of the panel's points inside the frustum, with a margin
// so a point just off screen counts); 0 when none of it is. toClip: world
// to clip, row-major with column vectors, clip depth 0 to 1; viewport: x,
// y, width, height in pixels.
inline float PixelsPerUnit( const Placement &placement, float unitsWide, float unitsTall,
    const float toClip[16], const float viewport[4] )
{
	if ( !( unitsWide > 0.0f ) || !( unitsTall > 0.0f ) || !( viewport[2] > 0.0f ) ||
	     !( viewport[3] > 0.0f ) )
		return 0.0f;
	float axes[2][3];
	for ( int k = 0; k < 3; ++k )
	{
		axes[0][k] = placement.right[k] / unitsWide;
		axes[1][k] = placement.down[k] / unitsTall;
	}
	auto row = [&]( int r, const float p[3], float w )
	{
		return toClip[r * 4 + 0] * p[0] + toClip[r * 4 + 1] * p[1] + toClip[r * 4 + 2] * p[2] +
		       toClip[r * 4 + 3] * w;
	};
	const int kSteps = 32;
	const float kMargin = 1.1f;
	float best = 0.0f;
	for ( int j = 0; j <= kSteps; ++j )
	{
		for ( int i = 0; i <= kSteps; ++i )
		{
			const float u = float( i ) / kSteps;
			const float v = float( j ) / kSteps;
			float p[3];
			for ( int k = 0; k < 3; ++k )
				p[k] = placement.origin[k] + placement.right[k] * u + placement.down[k] * v;
			const float clip[4] = {
			    row( 0, p, 1.0f ), row( 1, p, 1.0f ), row( 2, p, 1.0f ), row( 3, p, 1.0f ) };
			if ( !( clip[3] > 1e-4f ) )
				continue;
			const float x = clip[0] / clip[3];
			const float y = clip[1] / clip[3];
			const float z = clip[2] / clip[3];
			if ( std::fabs( x ) > kMargin || std::fabs( y ) > kMargin || z < 0.0f || z > 1.0f )
				continue;
			for ( const float *axis : axes )
			{
				// d(ndc)/d(unit) = (dc * w - c * dw) / w^2, then to pixels.
				const float dx0 = row( 0, axis, 0.0f );
				const float dy0 = row( 1, axis, 0.0f );
				const float dw = row( 3, axis, 0.0f );
				const float w2 = clip[3] * clip[3];
				const float dx = ( dx0 * clip[3] - clip[0] * dw ) / w2 * 0.5f * viewport[2];
				const float dy = ( dy0 * clip[3] - clip[1] * dw ) / w2 * 0.5f * viewport[3];
				best = std::max( best, std::sqrt( dx * dx + dy * dy ) );
			}
		}
	}
	return std::isfinite( best ) ? best : 0.0f;
}

// Whether a resolution is the one its density gives for these units.
inline bool ResolutionFits( const Resolution &r, float unitsWide, float unitsTall )
{
	return r.width > 0 && r.height > 0 && r.width <= kMaxDimension && r.height <= kMaxDimension &&
	       r.texelsPerUnit > 0.0f &&
	       r.width == std::max( 1u, std::uint32_t( std::ceil( unitsWide * r.texelsPerUnit ) ) ) &&
	       r.height == std::max( 1u, std::uint32_t( std::ceil( unitsTall * r.texelsPerUnit ) ) );
}

// The resolution for a panel needing `pixelsPerUnit`: the current one while
// it holds at least one texel per pixel and at most kDensityHold times that;
// else the lowest rung of the ladder kDensityStep^k that holds one texel per
// pixel. Clamped to kMinTexelsPerUnit and to maxDimension texels (at most
// kMaxDimension). pixelsPerUnit 0 (off screen) keeps a fitting current one.
inline Resolution ChooseResolution( float unitsWide, float unitsTall, float pixelsPerUnit,
    const Resolution &current, std::uint32_t maxDimension = kMaxDimension )
{
	Resolution none = { 0, 0, 0.0f };
	if ( !( unitsWide > 0.0f ) || !( unitsTall > 0.0f ) )
		return none;
	maxDimension = std::min( std::max( maxDimension, 1u ), kMaxDimension );
	const float maxDensity = float( maxDimension ) / std::max( unitsWide, unitsTall );
	auto make = [&]( float density )
	{
		density = std::min( std::max( density, kMinTexelsPerUnit ), maxDensity );
		Resolution r;
		r.texelsPerUnit = density;
		r.width = std::max( 1u, std::uint32_t( std::ceil( unitsWide * density ) ) );
		r.height = std::max( 1u, std::uint32_t( std::ceil( unitsTall * density ) ) );
		return r;
	};
	const bool currentFits = ResolutionFits( current, unitsWide, unitsTall ) &&
	                         current.width <= maxDimension && current.height <= maxDimension;
	if ( !( pixelsPerUnit > 0.0f ) || !std::isfinite( pixelsPerUnit ) )
		return currentFits ? current : make( kMinTexelsPerUnit );
	const float need = std::min( std::max( pixelsPerUnit, kMinTexelsPerUnit ), maxDensity );
	if ( currentFits && current.texelsPerUnit >= need &&
	     current.texelsPerUnit <= need * kDensityHold )
		return current;
	const float k = std::ceil( std::log( need ) / std::log( kDensityStep ) - 1e-4f );
	return make( std::pow( kDensityStep, k ) );
}

// The light grid for a panel: at most kMaxTiles tiles, as close to square as
// the panel's aspect allows (a 1:2 sign is 2 x 4).
constexpr int kMaxTiles = 8;
inline void Tiles( float unitsWide, float unitsTall, int &across, int &down )
{
	across = 1;
	down = 1;
	if ( !( unitsWide > 0.0f ) || !( unitsTall > 0.0f ) )
		return;
	float best = 1e30f;
	for ( int a = 1; a <= kMaxTiles; ++a )
	{
		for ( int d = 1; a * d <= kMaxTiles; ++d )
		{
			// Squareness of the tiles, then more tiles.
			const float aspect = ( unitsWide / a ) / ( unitsTall / d );
			const float score = std::fabs( std::log( aspect ) ) - 0.01f * float( a * d );
			if ( score < best )
			{
				best = score;
				across = a;
				down = d;
			}
		}
	}
}

inline float SrgbToLinear( float c )
{
	return c <= 0.04045f ? c / 12.92f : std::pow( ( c + 0.055f ) / 1.055f, 2.4f );
}

// The light a coating scatters (its particles catch light from the lit
// board around them): the scatter field. The emissive composite is sampled
// at the centers of a grid of cells kCoatingScatterCell panel units wide
// (ceil( units / cell ) cells along each axis, spanning the panel exactly);
// each cell's field is the mean of the samples of the (2 kCoatingScatterReach
// + 1)^2 cells centered on it that exist (a box of about 36 units); the field
// at a point is the bilinear interpolation of the cells' fields at their
// centers, clamped at the panel's edges. render.pass.panels computes the same
// grid on the GPU.
constexpr float kCoatingScatterCell = 4.0f;
constexpr int kCoatingScatterReach = 4;

inline void ScatterGridSize( float unitsWide, float unitsTall, int &wide, int &tall )
{
	wide = std::max( 1, int( std::ceil( unitsWide / kCoatingScatterCell ) ) );
	tall = std::max( 1, int( std::ceil( unitsTall / kCoatingScatterCell ) ) );
}

struct ScatterGrid
{
	int wide = 0;
	int tall = 0;
	float cellWide = 0.0f;
	float cellTall = 0.0f;
	std::vector<float> rgb; // linear, row by row; empty for a panel without coatings
};

// The list at one point of the panel: the emissive composite in linear light
// and the coatings' premultiplied linear composite (coating[3] their
// coverage). step: the texture footprint's size in panel units (for the
// sampler's filter).
template <typename Sample>
void CompositeAt( const DrawListView &list, float x, float y, const float step[2], Sample &sample,
    float emissive[3], float coating[4] )
{
	float dst[3] = { 0.0f, 0.0f, 0.0f }; // gamma, over opaque black
	coating[0] = coating[1] = coating[2] = coating[3] = 0.0f;
	for ( std::uint32_t q = 0; q < list.quadCount; ++q )
	{
		const Quad &quad = list.quads[q];
		const float qx0 = std::min( quad.x0, quad.x1 );
		const float qx1 = std::max( quad.x0, quad.x1 );
		const float qy0 = std::min( quad.y0, quad.y1 );
		const float qy1 = std::max( quad.y0, quad.y1 );
		if ( x < qx0 || x >= qx1 || y < qy0 || y >= qy1 )
			continue;
		const float fu = ( x - quad.x0 ) / ( quad.x1 - quad.x0 );
		const float fv = ( y - quad.y0 ) / ( quad.y1 - quad.y0 );
		const float s = quad.s0 + ( quad.s1 - quad.s0 ) * fu;
		const float t = quad.t0 + ( quad.t1 - quad.t0 ) * fv;
		float texel[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		if ( quad.texture != kWhite )
		{
			const float footprint =
			    std::max( std::fabs( ( quad.s1 - quad.s0 ) * step[0] / ( quad.x1 - quad.x0 ) ),
			        std::fabs( ( quad.t1 - quad.t0 ) * step[1] / ( quad.y1 - quad.y0 ) ) );
			const bool read = quad.texture < list.textureCount &&
			                  sample( list.textures[quad.texture], s, t, footprint, texel );
			if ( !read )
			{
				texel[0] = texel[1] = texel[2] = 1.0f;
				texel[3] = quad.coverage >= 0.0f ? quad.coverage : 1.0f;
			}
		}
		if ( quad.layer == kLayerCoating )
		{
			const float a = texel[3] * ( quad.color[3] / 255.0f );
			for ( int k = 0; k < 3; ++k )
				coating[k] = SrgbToLinear( texel[k] * ( quad.color[k] / 255.0f ) ) * a +
				             coating[k] * ( 1.0f - a );
			coating[3] = a + coating[3] * ( 1.0f - a );
			continue;
		}
		if ( quad.blend == kBlendOpaque )
			texel[3] = 1.0f;
		float c[4];
		for ( int k = 0; k < 4; ++k )
			c[k] = texel[k] * ( quad.color[k] / 255.0f );
		for ( int k = 0; k < 3; ++k )
		{
			if ( quad.blend == kBlendOpaque )
				dst[k] = c[k];
			else if ( quad.blend == kBlendAdditive )
				dst[k] = std::min( 1.0f, dst[k] + c[k] * c[3] );
			else
				dst[k] = c[k] * c[3] + dst[k] * ( 1.0f - c[3] );
		}
	}
	for ( int k = 0; k < 3; ++k )
		emissive[k] = SrgbToLinear( std::min( std::max( dst[k], 0.0f ), 1.0f ) );
}

// The scatter field of a list (empty when it has no coating quads).
template <typename Sample> ScatterGrid ScatterField( const DrawListView &list, Sample &&sample )
{
	ScatterGrid grid;
	bool coated = false;
	for ( std::uint32_t q = 0; q < list.quadCount; ++q )
		coated = coated || list.quads[q].layer == kLayerCoating;
	if ( !coated || !( list.unitsWide > 0.0f ) || !( list.unitsTall > 0.0f ) )
		return grid;
	ScatterGridSize( list.unitsWide, list.unitsTall, grid.wide, grid.tall );
	grid.cellWide = list.unitsWide / grid.wide;
	grid.cellTall = list.unitsTall / grid.tall;
	const float step[2] = { grid.cellWide, grid.cellTall };
	std::vector<float> samples( std::size_t( grid.wide ) * grid.tall * 3 );
	for ( int j = 0; j < grid.tall; ++j )
	{
		for ( int i = 0; i < grid.wide; ++i )
		{
			float coating[4];
			CompositeAt( list, ( i + 0.5f ) * grid.cellWide, ( j + 0.5f ) * grid.cellTall, step,
			    sample, &samples[( std::size_t( j ) * grid.wide + i ) * 3], coating );
		}
	}
	// The box, separable: rows, then columns (each mean over the cells that exist).
	std::vector<float> rows( samples.size() );
	grid.rgb.assign( samples.size(), 0.0f );
	for ( int pass = 0; pass < 2; ++pass )
	{
		const std::vector<float> &from = pass == 0 ? samples : rows;
		std::vector<float> &to = pass == 0 ? rows : grid.rgb;
		for ( int j = 0; j < grid.tall; ++j )
		{
			for ( int i = 0; i < grid.wide; ++i )
			{
				float sum[3] = { 0.0f, 0.0f, 0.0f };
				int count = 0;
				for ( int d = -kCoatingScatterReach; d <= kCoatingScatterReach; ++d )
				{
					const int x = pass == 0 ? i + d : i;
					const int y = pass == 0 ? j : j + d;
					if ( x < 0 || x >= grid.wide || y < 0 || y >= grid.tall )
						continue;
					for ( int k = 0; k < 3; ++k )
						sum[k] += from[( std::size_t( y ) * grid.wide + x ) * 3 + k];
					++count;
				}
				for ( int k = 0; k < 3; ++k )
					to[( std::size_t( j ) * grid.wide + i ) * 3 + k] = sum[k] / float( count );
			}
		}
	}
	return grid;
}

// The scatter field at a point (bilinear between cell centers, clamped).
inline void ScatterAt( const ScatterGrid &grid, float x, float y, float out[3] )
{
	out[0] = out[1] = out[2] = 0.0f;
	if ( grid.rgb.empty() )
		return;
	const float gx = std::min( std::max( x / grid.cellWide - 0.5f, 0.0f ), float( grid.wide - 1 ) );
	const float gy = std::min( std::max( y / grid.cellTall - 0.5f, 0.0f ), float( grid.tall - 1 ) );
	const int x0 = int( gx ), y0 = int( gy );
	const int x1 = std::min( x0 + 1, grid.wide - 1 ), y1 = std::min( y0 + 1, grid.tall - 1 );
	const float fx = gx - x0, fy = gy - y0;
	for ( int k = 0; k < 3; ++k )
	{
		auto at = [&]( int cx, int cy )
		{
			return grid.rgb[( std::size_t( cy ) * grid.wide + cx ) * 3 + k];
		};
		out[k] = ( at( x0, y0 ) * ( 1 - fx ) + at( x1, y0 ) * fx ) * ( 1 - fy ) +
		         ( at( x0, y1 ) * ( 1 - fx ) + at( x1, y1 ) * fx ) * fy;
	}
}

// What the panel emits at one point, in linear light (before emissionScale):
// the emissive composite E through the coatings' clear part, plus the light
// the coatings scatter (the scatter field): E (1 - A.a) + field A.rgb.
template <typename Sample>
void EmissionAt( const DrawListView &list, const ScatterGrid &field, float x, float y,
    const float step[2], Sample &&sample, float out[3] )
{
	float emissive[3], coating[4];
	CompositeAt( list, x, y, step, sample, emissive, coating );
	float scattered[3];
	ScatterAt( field, x, y, scattered );
	for ( int k = 0; k < 3; ++k )
		out[k] = emissive[k] * ( 1.0f - coating[3] ) + scattered[k] * coating[k];
}

// The mean emission of each tile of the panel's image, rgb per tile, row by
// row (tile 0 top-left), in linear light times emissionScale (EmissionAt), at
// samplesPerAxis x samplesPerAxis stratified points per tile.
// sample( key, s, t, footprint, rgba ) returns a texture's gamma RGBA at
// (s, t) filtered over `footprint` texture units, or false when the CPU
// cannot read it: the quad then counts as white at its coverage
// (Quad::coverage; a quad of unknown coverage counts as covering fully). The
// image the GPU draws is the same composite; the lab holds the two to each
// other.
template <typename Sample>
void TileRadiance( const DrawListView &list, float emissionScale, int across, int down,
    int samplesPerAxis, Sample &&sample, float ( *out )[3] )
{
	const int count = across * down;
	for ( int t = 0; t < count; ++t )
		out[t][0] = out[t][1] = out[t][2] = 0.0f;
	if ( !( list.unitsWide > 0.0f ) || !( list.unitsTall > 0.0f ) || across < 1 || down < 1 ||
	     samplesPerAxis < 1 )
		return;
	const float tileWide = list.unitsWide / across;
	const float tileTall = list.unitsTall / down;
	const float step[2] = { tileWide / samplesPerAxis, tileTall / samplesPerAxis };
	const ScatterGrid field = ScatterField( list, sample );
	for ( int ty = 0; ty < down; ++ty )
	{
		for ( int tx = 0; tx < across; ++tx )
		{
			double sum[3] = { 0.0, 0.0, 0.0 };
			for ( int j = 0; j < samplesPerAxis; ++j )
			{
				for ( int i = 0; i < samplesPerAxis; ++i )
				{
					float emitted[3];
					EmissionAt( list, field, tx * tileWide + ( i + 0.5f ) * step[0],
					    ty * tileTall + ( j + 0.5f ) * step[1], step, sample, emitted );
					for ( int k = 0; k < 3; ++k )
						sum[k] += emitted[k];
				}
			}
			const double n = double( samplesPerAxis ) * samplesPerAxis;
			for ( int k = 0; k < 3; ++k )
				out[ty * across + tx][k] = float( sum[k] / n ) * emissionScale;
		}
	}
}

} // namespace world_panel

#endif // RENDER_WORLD_PANEL_H
