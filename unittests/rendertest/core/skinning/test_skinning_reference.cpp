//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.skinning.reference (RFC 0016 K6, headless): the CPU oracle
//			SkinReference against an independent double-precision reference
//			that blends the bone matrices first, as studiorender's
//			ComputeSkinMatrix does (the oracle blends transformed points, as
//			common_vs_fxc.h does; the two are the same linear blend), over
//			200 seeded meshes; its rules on hand-built cases (identity and
//			translation bones, the 1 - w0 - w1 third weight, a stereo flex's
//			side, an index past the palette); the comparator's sensitivity;
//			and the graph pass on render.device.null (compute pass, storage
//			accesses and their transitions, kNoCompute without the
//			capability).
//
//=============================================================================//

#include "skinning_fixtures.h"
#include "render/device/null/provider.h"
#include "render/graph/compiled_graph.h"
#include "render/graph/executor.h"
#include "testing/checks.h"

#include <cmath>
#include <cstdio>

namespace
{

using namespace render;
using namespace render::pass::skinning;
namespace fixtures = rendertest::skinning;

// Blend the matrices, then transform, in double.
std::vector<SkinnedVertex> MatrixBlend( const fixtures::Mesh &mesh )
{
	std::vector<SkinnedVertex> out( mesh.vertices.size() );
	for ( std::size_t v = 0; v < mesh.vertices.size(); ++v )
	{
		const SkinVertex &vertex = mesh.vertices[v];
		double p[3], n[3], t[3];
		for ( int k = 0; k < 3; ++k )
		{
			p[k] = vertex.position[k];
			n[k] = vertex.normal[k];
			t[k] = vertex.tangent[k];
		}
		double wrinkle = 0.0;
		if ( !mesh.flexOffsets.empty() )
		{
			for ( std::uint32_t d = mesh.flexOffsets[v]; d < mesh.flexOffsets[v + 1]; ++d )
			{
				const FlexDelta &delta = mesh.flexDeltas[d];
				const FlexWeights &w = mesh.flexWeights[delta.flex];
				const FlexWeights &delayed = mesh.flexWeights[delta.delayedFlex];
				const double left = double( w.weight[0] ) * ( 1.0 - delta.delay ) +
				                    double( delayed.weight[0] ) * delta.delay;
				const double right = double( w.weight[1] ) * ( 1.0 - delta.delay ) +
				                     double( delayed.weight[1] ) * delta.delay;
				const double weight = left * ( 1.0 - delta.side ) + right * delta.side;
				for ( int k = 0; k < 3; ++k )
				{
					p[k] += weight * delta.position[k];
					n[k] += weight * delta.normal[k];
					t[k] += weight * delta.normal[k];
				}
				wrinkle += weight * delta.wrinkle;
			}
		}
		const double weights[3] = {
		    vertex.weight0, vertex.weight1, 1.0 - ( double( vertex.weight0 ) + vertex.weight1 ) };
		double m[3][4] = {};
		for ( int b = 0; b < 3; ++b )
		{
			std::size_t bone = ( vertex.bones >> ( 8 * b ) ) & 0xffu;
			if ( bone >= mesh.bones.size() )
				bone = 0;
			for ( int r = 0; r < 3; ++r )
				for ( int c = 0; c < 4; ++c )
					m[r][c] += weights[b] * mesh.bones[bone].rows[r][c];
		}
		SkinnedVertex &result = out[v];
		for ( int r = 0; r < 3; ++r )
		{
			result.position[r] =
			    float( m[r][0] * p[0] + m[r][1] * p[1] + m[r][2] * p[2] + m[r][3] );
			result.normal[r] = float( m[r][0] * n[0] + m[r][1] * n[1] + m[r][2] * n[2] );
			result.tangent[r] = float( m[r][0] * t[0] + m[r][1] * t[1] + m[r][2] * t[2] );
		}
		result.tangent[3] = vertex.tangent[3];
		result.wrinkle = float( wrinkle );
	}
	return out;
}

std::vector<SkinnedVertex> Skin( const fixtures::Mesh &mesh )
{
	std::vector<SkinnedVertex> out( mesh.vertices.size() );
	SkinReference( mesh.Inputs(), out );
	return out;
}

bool Near( const float a[3], float x, float y, float z )
{
	return std::fabs( a[0] - x ) < 1e-5f && std::fabs( a[1] - y ) < 1e-5f &&
	       std::fabs( a[2] - z ) < 1e-5f;
}

BoneMatrix Translation( float x, float y, float z )
{
	BoneMatrix bone;
	bone.rows[0][0] = bone.rows[1][1] = bone.rows[2][2] = 1.0f;
	bone.rows[0][3] = x;
	bone.rows[1][3] = y;
	bone.rows[2][3] = z;
	return bone;
}

void Rules( testing::Checks &checks )
{
	fixtures::Mesh mesh;
	mesh.bones = { Translation( 0, 0, 0 ), Translation( 10, 0, 0 ), Translation( 0, 20, 0 ) };
	SkinVertex vertex;
	vertex.position[0] = 1.0f;
	vertex.normal[2] = 1.0f;
	vertex.tangent[0] = 1.0f;
	vertex.tangent[3] = -1.0f;
	vertex.weight0 = 1.0f;
	vertex.weight1 = 0.0f;
	vertex.bones = 1; // bone 1 only
	mesh.vertices.push_back( vertex );
	vertex.weight0 = 0.25f;
	vertex.weight1 = 0.25f;
	vertex.bones = 1u | ( 2u << 8 ) | ( 0u << 16 ); // third weight 0.5 on bone 0
	mesh.vertices.push_back( vertex );
	vertex.weight0 = 1.0f;
	vertex.weight1 = 0.0f;
	vertex.bones = 200; // past the palette: bone 0
	mesh.vertices.push_back( vertex );
	std::vector<SkinnedVertex> out = Skin( mesh );
	checks.That( Near( out[0].position, 11, 0, 0 ), "rule.one-bone-translates" );
	checks.That( Near( out[0].normal, 0, 0, 1 ) && out[0].normal[3] == 0.0f,
	    "rule.translation-leaves-the-normal" );
	checks.That( out[0].tangent[3] == -1.0f, "rule.tangent-keeps-its-sign" );
	checks.That( Near( out[1].position, 1 + 2.5f, 5, 0 ), "rule.third-weight-is-one-minus-w0-w1" );
	checks.That( Near( out[2].position, 1, 0, 0 ), "rule.an-index-past-the-palette-reads-bone-0" );

	fixtures::Mesh flexed;
	flexed.bones = { Translation( 0, 0, 0 ) };
	SkinVertex base;
	base.bones = 0;
	flexed.vertices = { base, base };
	flexed.flexWeights = { { { 0.25f, 1.0f } } };
	FlexDelta left;
	left.position[1] = 4.0f;
	left.normal[0] = 1.0f;
	left.wrinkle = 2.0f;
	left.side = 0.0f;
	FlexDelta right = left;
	right.side = 1.0f;
	flexed.flexDeltas = { left, right };
	flexed.flexOffsets = { 0, 1, 2 };
	std::vector<SkinnedVertex> flexOut = Skin( flexed );
	checks.That(
	    Near( flexOut[0].position, 0, 1, 0 ) && std::fabs( flexOut[0].wrinkle - 0.5f ) < 1e-6f,
	    "rule.side-0-takes-the-first-weight" );
	checks.That(
	    Near( flexOut[1].position, 0, 4, 0 ) && std::fabs( flexOut[1].wrinkle - 2.0f ) < 1e-6f,
	    "rule.side-1-takes-the-second-weight" );
	checks.That( Near( flexOut[1].normal, 1, 0, 0 ) && Near( flexOut[1].tangent, 1, 0, 0 ),
	    "rule.the-normal-delta-moves-normal-and-tangent" );
	flexed.flexWeights.push_back( { { 1.0f, 0.0f } } );
	flexed.flexDeltas[0].delayedFlex = flexed.flexDeltas[1].delayedFlex = 1;
	flexed.flexDeltas[0].delay = 1.0f;
	flexed.flexDeltas[1].delay = 0.25f;
	flexOut = Skin( flexed );
	checks.That( Near( flexOut[0].position, 0, 4, 0 ) && flexOut[0].wrinkle == 2.0f,
	    "rule.fully-delayed-flex-uses-the-delayed-palette" );
	checks.That( Near( flexOut[1].position, 0, 3, 0 ) && flexOut[1].wrinkle == 1.5f,
	    "rule.partial-delay-blends-current-and-delayed-stereo-weights" );
	flexed.flexDeltas[1].side = 0.5f;
	flexOut = Skin( flexed );
	checks.That( Near( flexOut[1].position, 0, 2.375f, 0 ),
	    "rule.delay-and-stereo-side-compose" );
}

void Agreement( testing::Checks &checks )
{
	SkinError worst;
	for ( std::uint32_t seed = 0; seed < 200; ++seed )
	{
		const fixtures::Mesh mesh = fixtures::RandomMesh(
		    seed, 64 + seed * 7, 1 + seed % 128, seed % 2 ? 1 + seed % 40 : 0 );
		const SkinError error = CompareSkinned( Skin( mesh ), MatrixBlend( mesh ) );
		worst.position = std::max( worst.position, error.position );
		worst.normal = std::max( worst.normal, error.normal );
		worst.tangent = std::max( worst.tangent, error.tangent );
		worst.wrinkle = std::max( worst.wrinkle, error.wrinkle );
	}
	std::printf( "INFO skinning reference: worst difference position %.3g normal %.3g tangent %.3g "
	             "wrinkle %.3g over 200 meshes\n",
	    worst.position, worst.normal, worst.tangent, worst.wrinkle );
	checks.That( worst.position <= 1e-3f && worst.normal <= 1e-3f && worst.tangent <= 1e-3f &&
	                 worst.wrinkle <= 1e-3f,
	    "oracle.agrees-with-the-matrix-blend-reference" );

	// The comparator sees one wrong component in one vertex.
	const fixtures::Mesh mesh = fixtures::RandomMesh( 7, 100, 16, 4 );
	std::vector<SkinnedVertex> a = Skin( mesh );
	std::vector<SkinnedVertex> b = a;
	b[57].normal[1] += 0.01f;
	checks.That(
	    CompareSkinned( a, b ).normal >= 0.01f - 1e-6f && CompareSkinned( a, a ).normal == 0,
	    "compare.finds-one-component" );
	b = a;
	b[3].position[0] = NAN;
	checks.That( std::isinf( CompareSkinned( a, b ).position ), "compare.nan-is-a-failure" );
	b.pop_back();
	checks.That(
	    std::isinf( CompareSkinned( a, b ).position ), "compare.size-mismatch-is-a-failure" );
}

void GraphPass( testing::Checks &checks )
{
	device::null::NullOptions noCompute;
	noCompute.capabilities.Remove( device::Capability::kCompute );
	auto limited = device::null::Create( noCompute ).Value();
	auto refused = SkinningKernel::Create( *limited );
	checks.That( !refused && refused.Error() == SkinningStatus::kNoCompute,
	    "kernel.without-compute-fails-kNoCompute" );

	auto device = device::null::Create( {} ).Value();
	auto kernel = SkinningKernel::Create( *device );
	if ( !checks.That( kernel.HasValue(), "kernel.is-created-on-a-compute-device" ) )
		return;
	const fixtures::Mesh mesh = fixtures::RandomMesh( 3, 256, 8, 3 );
	auto buffer = [&]( std::uint64_t size, device::ResourceUsage usage )
	{
		device::BufferDesc desc;
		desc.size = size;
		desc.usages = { device::ResourceUsage::kCopyDestination, usage };
		return std::make_pair( device->CreateBuffer( desc ).Value(), desc );
	};
	graph::GraphBuilder builder;
	auto import = [&]( const char *name, std::uint64_t size, device::ResourceUsage usage )
	{
		auto [id, desc] = buffer( size, usage );
		return builder.ImportBuffer( name, id, desc, device::ResourceUsage::kUndefined, usage );
	};
	SkinningPassResources resources;
	resources.vertices =
	    import( "vertices", 256 * sizeof( SkinVertex ), device::ResourceUsage::kStorageRead );
	resources.bones =
	    import( "bones", 8 * sizeof( BoneMatrix ), device::ResourceUsage::kStorageRead );
	resources.flexOffsets = import( "offsets", 257 * 4, device::ResourceUsage::kStorageRead );
	resources.flexDeltas = import( "deltas", mesh.flexDeltas.size() * sizeof( FlexDelta ),
	    device::ResourceUsage::kStorageRead );
	resources.flexWeights =
	    import( "weights", 3 * sizeof( FlexWeights ), device::ResourceUsage::kStorageRead );
	resources.output =
	    import( "output", 256 * sizeof( SkinnedVertex ), device::ResourceUsage::kStorageWrite );
	resources.vertexCount = 256;
	resources.boneCount = 8;
	resources.deltaCount = static_cast<std::uint32_t>( mesh.flexDeltas.size() );
	resources.flexCount = 3;
	AddSkinningPass( builder, *kernel.Value(), resources );
	auto compiled = graph::CompileGraph( std::move( builder ) );
	if ( !checks.That( compiled.HasValue(), "pass.compiles" ) )
		return;
	checks.That( compiled.Value().order.size() == 1 &&
	                 compiled.Value().passes[0].kind == graph::PassKind::kCompute,
	    "pass.is-one-compute-pass" );
	checks.Equal( compiled.Value().order[0].transitions.size(), std::size_t( 6 ),
	    "pass.each-buffer-enters-its-storage-usage" );
	graph::SerialGraphExecutor executor;
	auto executed = executor.Execute( compiled.Value(), *device );
	checks.That( executed.HasValue(), "pass.the-null-device-accepts-the-dispatch" );
	checks.Equal( kernel.Value()->RecordFailures(), 0u, "pass.records-without-failure" );
	if ( executed )
		kernel.Value()->Collect( executed.Value().token );
}

} // namespace

int main()
{
	testing::Checks checks;
	Rules( checks );
	Agreement( checks );
	GraphPass( checks );
	return checks.Report();
}
