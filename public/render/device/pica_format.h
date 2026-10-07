//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device (the port): the two forms of ArtifactFormat::kPica
//			(RFC 0026 decision 2), their readers, writers and validation, for
//			the PICA adapter that runs them and the reduced material model
//			that writes them (decision 8). Portable: no 3DS SDK.
//
//			All integers are little-endian.
//
//			PVS1, a vertex stage:
//			  u32 magic 'PVS1'   u16 version (1)   u8 drawConstantRegister
//			  u8 uniformCount    u8 vertexIndexRegister   u8 reserved[3] (0)
//			  u32 dvlbBytes
//			  uniformCount x { u8 group, u8 binding, u8 firstRegister,
//			                   u8 registerCount }
//			  dvlbBytes of picasso output (magic "DVLB", one DVLE)
//			  drawConstantRegister is the first float uniform register of the
//			  D16 block (four floats per register), 0xFF for none. Registers
//			  are c0..c95; no two ranges overlap, nor the DVLE's own constants
//			  (checked where the DVLE is parsed, on the device).
//			  Vertex attribute location n arrives in input register vn;
//			  kUnorm8x4 attributes arrive as 0 to 255 (the program scales).
//			  vertexIndexRegister (0xFF: none) receives the vertex index the
//			  port's gl_VertexIndex names (first vertex, or index plus vertex
//			  offset), which the PICA200 has no register for: the adapter
//			  feeds it from its own attribute stream.
//			  The program writes PICA clip space: the port's clip z negated
//			  (PICA keeps -w <= z <= 0; the device maps -z/w to depth), x and y
//			  as the port's. Texture coordinates are (u, 1 - v) of the port's:
//			  textures and render targets are stored with the port's row 0
//			  first, which the GPU samples at t = 1 (measured in Azahar,
//			  2026-10-07: an 8x8 texture sampled with t = v came out upside
//			  down).
//
//			PFP1, a fragment stage (fixed function: data, no code):
//			  u32 magic 'PFP1'   u16 version (1)   u8 stageCount (1..6)
//			  u8 textureCount (0..3)
//			  u8 alphaTest (a GPU test function code, 0xFF for none)
//			  u8 alphaReference  u8 bufferWrite (bits 0-3 colour, 4-7 alpha:
//			                     stage n's result updates the combiner buffer)
//			  u8 specializationCount   u32 bufferColor (RGBA8, the buffer's
//			                           initial value)
//			  textureCount x { u8 textureGroup, u8 textureBinding,
//			                   u8 samplerGroup, u8 samplerBinding }
//			  stageCount x { u8 rgbSources[3], u8 alphaSources[3],
//			                 u8 rgbOperands[3], u8 alphaOperands[3],
//			                 u8 rgbCombine, u8 alphaCombine, u8 rgbScale,
//			                 u8 alphaScale, u8 constantKind, u8 constantGroup,
//			                 u8 constantBinding, u8 reserved (0),
//			                 u32 constantOffset, u32 constantColor }
//			  specializationCount x { u32 id, u32 value, u8 stage,
//			                          u8 reserved[3] (0), u32 color }
//			  Texture unit n samples with the vertex program's texcoord n.
//			  A stage's constant is constantColor (RGBA8, R in the low byte)
//			  for kLiteral; the vec4 of floats at constantOffset of the D16
//			  block for kDrawConstants; the vec4 at constantOffset of the
//			  uniform buffer bound at (constantGroup, constantBinding) for
//			  kUniformBuffer. Floats are clamped to 0..1 and rounded to 8 bits
//			  when the draw is recorded. A specialization entry makes stage's
//			  constant the literal color when the fragment stage's
//			  specialization constant id has value (D20); ids no entry names
//			  are ignored.
//
//=============================================================================//

#ifndef RENDER_DEVICE_PICA_FORMAT_H
#define RENDER_DEVICE_PICA_FORMAT_H

#include "render/device/pipeline.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace render::device::pica_format
{

inline constexpr std::uint32_t kVertexMagic = 0x31535650;   // "PVS1"
inline constexpr std::uint32_t kFragmentMagic = 0x31504650; // "PFP1"
inline constexpr std::uint16_t kArtifactVersion = 1;
inline constexpr std::uint8_t kNone = 0xFF;
inline constexpr std::uint32_t kFloatUniformRegisters = 96;
inline constexpr std::uint32_t kInputRegisters = 16;
inline constexpr std::uint32_t kMaxCombinerStages = 6;
inline constexpr std::uint32_t kMaxTextureUnits = 3;
inline constexpr std::uint32_t kRegisterBytes = 16;

struct UniformRange
{
	std::uint8_t group = 0;
	std::uint8_t binding = 0;
	std::uint8_t firstRegister = 0;
	std::uint8_t registerCount = 0;
};

struct VertexProgram
{
	std::uint8_t drawConstantRegister = kNone;
	std::uint8_t vertexIndexRegister = kNone;
	std::vector<UniformRange> uniforms;
	std::span<const std::byte> dvlb; // a view into the artifact's bytes
};

struct TextureUnit
{
	std::uint8_t textureGroup = 0;
	std::uint8_t textureBinding = 0;
	std::uint8_t samplerGroup = 0;
	std::uint8_t samplerBinding = 0;
};

enum class ConstantKind : std::uint8_t
{
	kLiteral,
	kDrawConstants,
	kUniformBuffer
};

struct CombinerStage
{
	std::array<std::uint8_t, 3> rgbSources{};
	std::array<std::uint8_t, 3> alphaSources{};
	std::array<std::uint8_t, 3> rgbOperands{};
	std::array<std::uint8_t, 3> alphaOperands{};
	std::uint8_t rgbCombine = 0;
	std::uint8_t alphaCombine = 0;
	std::uint8_t rgbScale = 0;
	std::uint8_t alphaScale = 0;
	ConstantKind constantKind = ConstantKind::kLiteral;
	std::uint8_t constantGroup = 0;
	std::uint8_t constantBinding = 0;
	std::uint32_t constantOffset = 0;
	std::uint32_t constantColor = 0;
};

struct Specialization
{
	std::uint32_t id = 0;
	std::uint32_t value = 0;
	std::uint8_t stage = 0;
	std::uint32_t color = 0;
};

struct FragmentProgram
{
	std::uint8_t alphaTest = kNone;
	std::uint8_t alphaReference = 0;
	std::uint8_t bufferWrite = 0;
	std::uint32_t bufferColor = 0;
	std::vector<TextureUnit> textures;
	std::vector<CombinerStage> stages;
	std::vector<Specialization> specializations;
};

// The first rule an artifact breaks; empty when it is sound.
struct ArtifactProblem
{
	std::string_view rule;
	explicit operator bool() const { return !rule.empty(); }
};

// Parse and validate. drawConstantBytes is the pipeline's D16 block size: a
// draw-constant register range or constant outside it is a problem.
ArtifactProblem ReadVertexProgram(
    std::span<const std::byte> code, std::uint32_t drawConstantBytes, VertexProgram &out );
ArtifactProblem ReadFragmentProgram(
    std::span<const std::byte> code, std::uint32_t drawConstantBytes, FragmentProgram &out );

// The port bindings an artifact uses, as ShaderArtifactView::bindings must
// reflect them (each once, in artifact order): uniform buffers of a vertex
// program; sampled textures, samplers and constant uniform buffers of a
// fragment program.
std::vector<ReflectedBinding> BindingsOf( const VertexProgram &program );
std::vector<ReflectedBinding> BindingsOf( const FragmentProgram &program );

// The D16 bytes a stage reads (ShaderArtifactView::drawConstantBytes must be
// at least this).
std::uint32_t DrawConstantBytesOf( const VertexProgram &program );
std::uint32_t DrawConstantBytesOf( const FragmentProgram &program );

// Each stage's constant after the pipeline's fragment specialization
// constants (D20): the literal of the last entry matching, else the stage's
// own description.
std::vector<CombinerStage> Specialize(
    const FragmentProgram &program, std::span<const SpecializationConstant> constants );

// A vec4 of floats as the combiner's RGBA8 constant (R in the low byte):
// each channel clamped to 0..1, times 255, rounded.
std::uint32_t PackConstant( const float ( &value )[4] );

// Writers, for the artifact compiler's tests and the host suite.
std::vector<std::byte> WriteVertexProgram(
    const VertexProgram &program, std::span<const std::byte> dvlb );
std::vector<std::byte> WriteFragmentProgram( const FragmentProgram &program );

} // namespace render::device::pica_format

#endif // RENDER_DEVICE_PICA_FORMAT_H
