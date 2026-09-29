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
//			Skeleton. The bones (name, parent, reference position and
//			rotation, the compressed-value base and scales, poseToBone,
//			flags), each mesh vertex's bone weights (the VVD's, up to three)
//			and, per sequence, its label, flags, the compiler's bounding box
//			and the local pose of every bone at its first frame. A parent
//			precedes its child.
//
//			First frame. A sequence draws its animation at blend (0, 0) (for
//			a blend grid, the first animation of the grid: pose parameters
//			are not modelled). Frame 0 of that animation is decoded as
//			public/bone_setup.cpp decodes it, from section 0 when the
//			animation is sectioned: RLE bone x frame records (raw
//			Quaternion48/Quaternion64 rotations, raw Vector48 positions, and
//			run-length value streams times the bone's rotscale/posscale added
//			to its reference euler/position), or frame x bone data
//			(STUDIO_FRAMEANIM; Quaternion48, Quaternion48S, Vector48 or float
//			vectors, per frame or constant). Bones without data keep the
//			reference (identity for a delta animation). Data in an animation
//			block lives in the .ani file (ModelBytes::ani); without it the
//			MDL's zero-frame data stands in, as the engine does while a block
//			loads, and with neither the model is MissingFile in ani. The
//			sequence's per-bone weights then blend the frame with the
//			reference (a delta sequence adds its weighted frame to the
//			reference). Not modelled: bone controllers, procedural bones,
//			IK, local hierarchy, autolayers, pose parameters.
//
//			Included models ($includemodel). As the engine's virtual model
//			(public/studio_virtualmodel.cpp) builds it: the model's own
//			sequences, then each included model's (ModelBytes::includes, in
//			the engine's depth-first group order), a label already present
//			(ASCII case-insensitive) keeping the first unless that one is a
//			forward declaration (STUDIO_OVERRIDE). An included sequence draws
//			its own model's animation; its bones map to this model's by name
//			(case-insensitive), and a bone it does not name keeps this
//			model's reference. Animations shared between models by name are
//			not merged. LoadModel reads the includes (and theirs, and their
//			.ani files) through the same IModelFiles; one that is absent is
//			skipped, as the engine skips it.
//
//			Posing. PoseModel skins vertices and normals by their weights to
//			the model space of a pose (bone-to-model = parent's x the local
//			transform; skin = bone-to-model x poseToBone) and recomputes the
//			bounds. The reference pose returns the bind vertices. PoseModel by
//			sequence returns a $staticprop model (kStaticPropFlag: one
//			collapsed bone) unchanged, as the engine's static-prop path draws
//			it; its first frame is the reference anyway (the corpus checks).
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

#include <array>
#include <cstdint>
#include <span>
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

// (x, y, z, w), w the real part, as Source stores them.
struct Quaternion
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 1.0f;

	friend bool operator==( const Quaternion &, const Quaternion & ) = default;
};

// Three rows of a 4x4 affine transform: p' = m[r][0..2] . p + m[r][3].
struct Matrix3x4
{
	std::array<std::array<float, 4>, 3> m = {
	    { { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f } } };

	friend bool operator==( const Matrix3x4 &, const Matrix3x4 & ) = default;
};

// A vertex's bones (indices into Model::bones) and their weights, as the VVD
// stores them: 'count' of three are used.
struct BoneWeights
{
	std::uint8_t count = 1;
	std::array<std::uint8_t, 3> bones = { 0, 0, 0 };
	std::array<float, 3> weights = { 1.0f, 0.0f, 0.0f };

	friend bool operator==( const BoneWeights &, const BoneWeights & ) = default;
};

struct Mesh
{
	std::int32_t textureRef = 0; // column of the skin table
	std::int32_t bodyPart = 0;
	std::vector<Vertex> vertices;
	std::vector<BoneWeights> weights;   // one per vertex
	std::vector<std::uint32_t> indices; // triangle list, counter-clockwise from outside

	friend bool operator==( const Mesh &, const Mesh & ) = default;
};

struct Bone
{
	std::string name;         // as stored
	std::int32_t parent = -1; // an earlier bone, or -1 for a root
	Float3 position;          // reference pose, in the parent's space
	Quaternion rotation;
	Float3 rotationEuler; // radians: the base compressed rotation values add to
	Float3 positionScale; // compressed value scales
	Float3 rotationScale;
	Matrix3x4 poseToBone; // bind-pose model space to this bone's space
	std::uint32_t flags = 0;

	friend bool operator==( const Bone &, const Bone & ) = default;
};

// A bone's local transform: in its parent's space (the model's for a root).
struct BoneTransform
{
	Float3 position;
	Quaternion rotation;

	friend bool operator==( const BoneTransform &, const BoneTransform & ) = default;
};

struct Sequence
{
	std::string label;      // as stored ("idle", "close_idle")
	std::int32_t group = 0; // 0: this model's; n: from ModelBytes::includes[n - 1]
	std::uint32_t flags = 0;
	Float3 boundsMin; // the compiler's box over the sequence's frames, as stored
	Float3 boundsMax;
	std::int32_t blendCount = 1;
	std::int32_t animation = 0;            // the animation drawn: blend (0, 0)
	std::vector<float> boneWeights;        // one per bone of this model (0: the reference)
	std::vector<BoneTransform> firstFrame; // one per bone (see "First frame")

	friend bool operator==( const Sequence &, const Sequence & ) = default;
};

constexpr std::uint32_t kStaticPropFlag = 0x10;     // Model::flags: compiled $staticprop
constexpr std::uint32_t kDeltaSequence = 0x04;      // Sequence::flags: adds to the reference
constexpr std::uint32_t kOverrideSequence = 0x0800; // Sequence::flags: a forward declaration

struct Model
{
	std::int32_t version = 0;
	std::int32_t checksum = 0;
	std::uint32_t flags = 0;              // the header's flags
	std::string name;                     // the header's name, as stored
	std::vector<std::string> textures;    // as stored ("metal_box"), lower case
	std::vector<std::string> cdMaterials; // lower case, '/' separated, ending in '/' (or empty)
	std::vector<std::vector<std::int16_t>> skinFamilies; // [family][textureRef] -> texture
	std::vector<Mesh> meshes;
	std::vector<Bone> bones; // at least one
	std::vector<Sequence> sequences;
	std::vector<std::string> includeModels; // $includemodel paths, as stored (lower case)
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
	Ani,        // the animation-block file a sequence's first frame lives in
	IncludeMdl, // an included model (ModelBytes::includes; offset in that file)
	IncludeAni, // an included model's animation-block file
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

struct IncludedModelBytes
{
	std::string_view mdl;
	std::string_view ani; // its .ani file, when it names animation blocks (else empty)
};

struct ModelBytes
{
	std::string_view mdl;
	std::string_view vvd;
	std::string_view vtx;
	std::string_view ani; // the model's .ani file, when it names animation blocks (else empty)
	std::vector<IncludedModelBytes> includes = {}; // see "Included models"
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
// VTX is MissingFile in that file. A model that names animation blocks also
// has the .ani file its header names read, when present (see "First frame").
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

// The sequence labelled 'label' (ASCII case-insensitive); -1 when none is.
std::int32_t FindSequence( const Model &model, std::string_view label );

// Every bone's reference transform.
std::vector<BoneTransform> ReferencePose( const Model &model );

// The rotation-and-translation matrix of a quaternion and a position.
Matrix3x4 TransformMatrix( const Quaternion &rotation, const Float3 &position );
Matrix3x4 Concat( const Matrix3x4 &a, const Matrix3x4 &b ); // a x b
Float3 TransformPoint( const Matrix3x4 &m, const Float3 &p );

// Bone-to-model matrices of a local pose: a root's is its local transform, a
// child's its parent's x its local transform. Empty unless 'pose' has one
// transform per bone.
std::vector<Matrix3x4> BoneToModel( const Model &model, std::span<const BoneTransform> pose );

// The model skinned to 'pose' (see "Posing"): vertices and normals in the
// pose's model space, bounds recomputed; everything else as 'model'. 'model'
// unchanged unless 'pose' has one transform per bone.
Model PoseModel( const Model &model, std::span<const BoneTransform> pose );

// The model at the first frame of sequence 'sequence'; 'model' unchanged when
// no sequence has that index or the model is a $staticprop.
Model PoseModel( const Model &model, std::int32_t sequence );

} // namespace mdl

#endif // MDL_STUDIO_MODEL_H
