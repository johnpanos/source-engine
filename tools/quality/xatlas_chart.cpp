// Chart and flatten a curved mesh with xatlas, for lightmap_layout.py.
//
//     xatlas_chart in.bin out.bin
//
// xatlas decides where a curved surface is cut into charts and flattens each
// chart; lightmap_layout.py owns everything else (density, rotation, packing).
// xatlas's packer runs only to read the charts out: with no padding and no
// rotation, each chart's output UVs are its flattened coordinates times the
// packer's texels per unit plus an offset; the scale is divided out here and
// the caller removes the offset. The packer picks its own scale (about a
// 1024 atlas), so its rasterization cost does not grow with the input units.
//
// Input (little-endian):
//   char[4]  "XAC1"
//   uint32   vertex count V, triangle count T
//   float32  positions[3V], normals[3V]
//   uint32   indices[3T]
//   float32  maxCost, normalDeviationWeight, roundnessWeight,
//            straightnessWeight, normalSeamWeight, textureSeamWeight,
//            maxChartArea, maxBoundaryLength
//   uint32   maxIterations
// Output:
//   char[4]  "XAO1"
//   uint32   T, chart count
//   int32    chart[T] (-1: not charted)
//   float32  uv[6T]: each triangle's three corners, in input units
//
// Faces keep their input order: xatlas's output mesh has one index per input
// index, face for face.

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

#include "xatlas.h"

namespace
{

bool ReadAll( FILE *file, void *data, size_t bytes )
{
	return fread( data, 1, bytes, file ) == bytes;
}

bool WriteAll( FILE *file, const void *data, size_t bytes )
{
	return fwrite( data, 1, bytes, file ) == bytes;
}

int Fail( const char *message )
{
	fprintf( stderr, "xatlas_chart: %s\n", message );
	return 1;
}

} // namespace

int main( int argc, char **argv )
{
	if ( argc != 3 )
		return Fail( "usage: xatlas_chart in.bin out.bin" );
	FILE *in = fopen( argv[1], "rb" );
	if ( !in )
		return Fail( "cannot open the input" );
	char magic[4];
	uint32_t counts[2];
	if ( !ReadAll( in, magic, 4 ) || memcmp( magic, "XAC1", 4 ) != 0 ||
		 !ReadAll( in, counts, sizeof( counts ) ) )
	{
		fclose( in );
		return Fail( "not an XAC1 input" );
	}
	const uint32_t vertexCount = counts[0];
	const uint32_t triangleCount = counts[1];
	std::vector<float> positions( 3 * size_t( vertexCount ) );
	std::vector<float> normals( 3 * size_t( vertexCount ) );
	std::vector<uint32_t> indices( 3 * size_t( triangleCount ) );
	float weights[8];
	uint32_t maxIterations = 0;
	const bool read = ReadAll( in, positions.data(), positions.size() * sizeof( float ) ) &&
					  ReadAll( in, normals.data(), normals.size() * sizeof( float ) ) &&
					  ReadAll( in, indices.data(), indices.size() * sizeof( uint32_t ) ) &&
					  ReadAll( in, weights, sizeof( weights ) ) &&
					  ReadAll( in, &maxIterations, sizeof( maxIterations ) );
	fclose( in );
	if ( !read )
		return Fail( "truncated input" );
	for ( uint32_t index : indices )
	{
		if ( index >= vertexCount )
			return Fail( "index out of range" );
	}

	xatlas::Atlas *atlas = xatlas::Create();
	xatlas::MeshDecl mesh;
	mesh.vertexCount = vertexCount;
	mesh.vertexPositionData = positions.data();
	mesh.vertexPositionStride = 3 * sizeof( float );
	mesh.vertexNormalData = normals.data();
	mesh.vertexNormalStride = 3 * sizeof( float );
	mesh.indexCount = uint32_t( indices.size() );
	mesh.indexData = indices.data();
	mesh.indexFormat = xatlas::IndexFormat::UInt32;
	if ( xatlas::AddMesh( atlas, mesh, 1 ) != xatlas::AddMeshError::Success )
	{
		xatlas::Destroy( atlas );
		return Fail( "xatlas rejected the mesh" );
	}
	xatlas::ChartOptions chartOptions;
	chartOptions.maxCost = weights[0];
	chartOptions.normalDeviationWeight = weights[1];
	chartOptions.roundnessWeight = weights[2];
	chartOptions.straightnessWeight = weights[3];
	chartOptions.normalSeamWeight = weights[4];
	chartOptions.textureSeamWeight = weights[5];
	chartOptions.maxChartArea = weights[6];
	chartOptions.maxBoundaryLength = weights[7];
	chartOptions.maxIterations = maxIterations;
	xatlas::ComputeCharts( atlas, chartOptions );
	xatlas::PackOptions packOptions;
	packOptions.padding = 0;
	packOptions.texelsPerUnit = 0.0f;
	packOptions.resolution = 0;
	packOptions.bilinear = false;
	packOptions.blockAlign = false;
	packOptions.bruteForce = false;
	packOptions.rotateChartsToAxis = false;
	packOptions.rotateCharts = false;
	xatlas::PackCharts( atlas, packOptions );

	const xatlas::Mesh &out = atlas->meshes[0];
	if ( out.indexCount != indices.size() )
	{
		xatlas::Destroy( atlas );
		return Fail( "xatlas changed the face count" );
	}
	std::vector<int32_t> charts( triangleCount, -1 );
	std::vector<float> uvs( 6 * size_t( triangleCount ) );
	for ( uint32_t face = 0; face < triangleCount; face++ )
	{
		for ( uint32_t corner = 0; corner < 3; corner++ )
		{
			const xatlas::Vertex &vertex = out.vertexArray[out.indexArray[3 * face + corner]];
			if ( vertex.xref != indices[3 * face + corner] )
			{
				xatlas::Destroy( atlas );
				return Fail( "xatlas reordered the faces" );
			}
			if ( corner == 0 )
				charts[face] = vertex.chartIndex;
			else if ( vertex.chartIndex != charts[face] )
				charts[face] = -1;
			uvs[6 * face + 2 * corner] = vertex.uv[0] / atlas->texelsPerUnit;
			uvs[6 * face + 2 * corner + 1] = vertex.uv[1] / atlas->texelsPerUnit;
		}
	}
	const uint32_t header[2] = { triangleCount, out.chartCount };
	xatlas::Destroy( atlas );

	FILE *file = fopen( argv[2], "wb" );
	if ( !file )
		return Fail( "cannot open the output" );
	const bool written = WriteAll( file, "XAO1", 4 ) && WriteAll( file, header, sizeof( header ) ) &&
						 WriteAll( file, charts.data(), charts.size() * sizeof( int32_t ) ) &&
						 WriteAll( file, uvs.data(), uvs.size() * sizeof( float ) );
	if ( fclose( file ) != 0 || !written )
		return Fail( "cannot write the output" );
	return 0;
}
