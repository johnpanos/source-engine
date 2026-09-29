//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Studio model mesh reader (content.studio-model): the drawable
//			level of detail 0 of a Source model, read from its three files --
//			the .mdl header (textures, $cdmaterials, skin families, body
//			parts, meshes), the .vvd vertices and the .vtx strip groups --
//			as values. A strict, portable C++20 library over bytes: no tier0,
//			tier1, mathlib, studio.h or application header, so the editor,
//			compile tools and engine can each compose it.
//
//			Accepted input. MDL "IDST" versions 44 to 49 (Portal: 44, 45, 46
//			and 48; Portal 2: 49), VVD "IDSV" version 4, VTX version 7. The
//			three files' checksums must agree. A version 49 model's VTX strip
//			groups and strips carry the topology fields (33 and 35 bytes;
//			25 and 27 otherwise), as datacache/mdlcache.cpp decides it.
//
//			Output. For the chosen body (legacy body-group arithmetic: body
//			part p draws model (body / base_p) % count_p), every mesh of LOD
//			0 that has triangles, in body part then mesh order: its skin
//			reference, its vertices (the mesh's own range of the VVD's LOD 0
//			vertex list after fixups; model space, the bind pose: position,
//			normal, uv) and a triangle list whose indices name those
//			vertices. Triangles are wound counter-clockwise seen from outside
//			(the side the vertex normals face; the files store them
//			clockwise, Direct3D's convention, so each triangle's second and
//			third index are swapped). Tristrip strips become lists. Bounds are
//			those of the vertices the triangles use.
//
//			Materials. A mesh's texture is skinFamilies[skin][textureRef]
//			(a skin out of range uses family 0, as the engine does).
//			ResolveMaterials names each texture as a VMF names a material:
//			the first "<cdmaterials><texture>" whose
//			"materials/<cdmaterials><texture>.vmt" exists, in $cdmaterials
//			order; otherwise the first directory's candidate, marked not
//			found. Names are lower case with forward slashes.
//
//			Failure. Malformed input is an error naming its status, the file
//			and the byte offset where reading stopped; nothing is partially
//			returned. Every count and offset is bounded by the file's size
//			before it is used.
//
//=============================================================================//

#ifndef MDL_STUDIO_MODEL_H
#define MDL_STUDIO_MODEL_H

#include "foundation/expected.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace mdl
{

struct Float3
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	friend bool operator==( const Float3 &, const Float3 & ) = default;
};

struct Vertex
{
	Float3 position; // model space, bind pose
	Float3 normal;
	float u = 0.0f;
	float v = 0.0f;

	friend bool operator==( const Vertex &, const Vertex & ) = default;
};

struct Mesh
{
	std::int32_t textureRef = 0; // column of the skin table
	std::int32_t bodyPart = 0;
	std::vector<Vertex> vertices;
	std::vector<std::uint32_t> indices; // triangle list, counter-clockwise from outside

	friend bool operator==( const Mesh &, const Mesh & ) = default;
};

struct Model
{
	std::int32_t version = 0;
	std::int32_t checksum = 0;
	std::string name;                     // the header's name, as stored
	std::vector<std::string> textures;    // as stored ("metal_box"), lower case
	std::vector<std::string> cdMaterials; // lower case, '/' separated, ending in '/' (or empty)
	std::vector<std::vector<std::int16_t>> skinFamilies; // [family][textureRef] -> texture
	std::vector<Mesh> meshes;
	Float3 mins; // of the vertices the triangles use; zero when there are none
	Float3 maxs;
	Float3 hullMins; // the header's boxes, as stored
	Float3 hullMaxs;
	Float3 viewMins;
	Float3 viewMaxs;

	std::size_t TriangleCount() const;

	friend bool operator==( const Model &, const Model & ) = default;
};

enum class ModelStatus : std::uint8_t
{
	MissingFile,        // a file could not be read (LoadModel only)
	BadMagic,           // not "IDST" / "IDSV"
	UnsupportedVersion, // outside the accepted versions
	Truncated,          // a read ran past the end of the file
	BadCount,           // a negative count, or counts that disagree between the files
	BadOffset,          // an offset outside the file or misaligned
	BadIndex,           // an index names nothing (vertex, texture, skin reference)
	ChecksumMismatch,   // the three files are not one model
};

enum class ModelFile : std::uint8_t
{
	Mdl,
	Vvd,
	Vtx,
};

struct ModelError
{
	ModelStatus status = ModelStatus::MissingFile;
	ModelFile file = ModelFile::Mdl;
	std::uint32_t offset = 0; // where reading stopped (0 for MissingFile)

	friend bool operator==( const ModelError &, const ModelError & ) = default;
};

const char *StatusName( ModelStatus status );
const char *FileName( ModelFile file );
// "Truncated in vvd at byte 64".
std::string Describe( const ModelError &error );

struct ModelBytes
{
	std::string_view mdl;
	std::string_view vvd;
	std::string_view vtx;
};

foundation::Expected<Model, ModelError> ParseModel(
    const ModelBytes &bytes, std::int32_t body = 0 );

// Where a model's files come from: a VPK, a search path, loose files, a fake.
// Paths are canonical asset paths ("models/props/metal_box.mdl": lower case,
// forward slashes, extension included).
class IModelFiles
{
public:
	virtual ~IModelFiles() = default;
	virtual bool Exists( const std::string &path ) const = 0;
	// Reads the whole file into 'out'; false (out unspecified) when absent.
	virtual bool Read( const std::string &path, std::string &out ) const = 0;
};

// The canonical form of a model path as an entity's "model" key spells it:
// lower case, '/' separated, no leading '/'. "Models\Props\Box.MDL" ->
// "models/props/box.mdl".
std::string CanonicalModelPath( std::string_view path );

// Reads "<path>.mdl", "<path>.vvd" and the first VTX present of ".dx90.vtx",
// ".vtx", ".dx80.vtx" and ".sw.vtx", then parses them. A missing MDL, VVD or
// VTX is MissingFile in that file.
foundation::Expected<Model, ModelError> LoadModel(
    const IModelFiles &files, std::string_view mdlPath, std::int32_t body = 0 );

struct ResolvedMaterial
{
	std::string name; // as a VMF names a material ("models/props/metal_box")
	bool found = false;

	friend bool operator==( const ResolvedMaterial &, const ResolvedMaterial & ) = default;
};

// One entry per texture (see "Materials" above).
std::vector<ResolvedMaterial> ResolveMaterials( const Model &model, const IModelFiles &files );

// The texture a mesh draws with 'skin'; -1 when the skin table has no entry.
std::int32_t TextureIndex( const Model &model, const Mesh &mesh, std::int32_t skin );

} // namespace mdl

#endif // MDL_STUDIO_MODEL_H
