//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Readers, validation and writers of the kPica artifact forms
//			(artifacts.h owns the layout).
//
//=============================================================================//

#include "render/device/pica_format.h"

#include "render/device/pica_codes.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace render::device::pica_format
{
namespace
{

constexpr std::size_t kVertexHeaderBytes = 16;
constexpr std::size_t kUniformBytes = 4;
constexpr std::size_t kFragmentHeaderBytes = 16;
constexpr std::size_t kTextureBytes = 4;
constexpr std::size_t kStageBytes = 28;
constexpr std::size_t kSpecializationBytes = 16;
constexpr std::uint32_t kDvlbMagic = 0x424C5644; // "DVLB"
constexpr std::uint32_t kVec4Bytes = 16;

class Reader
{
public:
	explicit Reader( std::span<const std::byte> bytes ) : m_Bytes( bytes ) {}

	bool Has( std::size_t count ) const { return m_At + count <= m_Bytes.size(); }
	std::size_t Left() const { return m_Bytes.size() - m_At; }

	std::uint8_t U8() { return std::to_integer<std::uint8_t>( m_Bytes[m_At++] ); }
	std::uint16_t U16()
	{
		const std::uint16_t lo = U8();
		return std::uint16_t( lo | std::uint16_t( U8() ) << 8 );
	}
	std::uint32_t U32()
	{
		const std::uint32_t lo = U16();
		return lo | std::uint32_t( U16() ) << 16;
	}
	std::span<const std::byte> Take( std::size_t count )
	{
		auto view = m_Bytes.subspan( m_At, count );
		m_At += count;
		return view;
	}

private:
	std::span<const std::byte> m_Bytes;
	std::size_t m_At = 0;
};

class Writer
{
public:
	void U8( std::uint8_t value ) { m_Bytes.push_back( std::byte( value ) ); }
	void U16( std::uint16_t value )
	{
		U8( std::uint8_t( value ) );
		U8( std::uint8_t( value >> 8 ) );
	}
	void U32( std::uint32_t value )
	{
		U16( std::uint16_t( value ) );
		U16( std::uint16_t( value >> 16 ) );
	}
	void Bytes( std::span<const std::byte> bytes )
	{
		m_Bytes.insert( m_Bytes.end(), bytes.begin(), bytes.end() );
	}
	std::vector<std::byte> Take() { return std::move( m_Bytes ); }

private:
	std::vector<std::byte> m_Bytes;
};

ArtifactProblem Problem( std::string_view rule )
{
	return ArtifactProblem{ rule };
}

bool ValidSource( std::uint8_t source, std::size_t textures )
{
	switch ( source )
	{
	case source::kPrimaryColor:
	case source::kPreviousBuffer:
	case source::kConstant:
	case source::kPrevious:
		return true;
	case source::kTexture0:
	case source::kTexture1:
	case source::kTexture2:
		return std::size_t( source - source::kTexture0 ) < textures;
	default:
		// Fragment lighting (primary/secondary) and the procedural texture
		// (texture 3) have no port binding to feed them.
		return false;
	}
}

} // namespace

ArtifactProblem ReadVertexProgram(
    std::span<const std::byte> code, std::uint32_t drawConstantBytes, VertexProgram &out )
{
	out = {};
	Reader in( code );
	if ( !in.Has( kVertexHeaderBytes ) )
		return Problem( "PVS1 shorter than its header" );
	if ( in.U32() != kVertexMagic )
		return Problem( "PVS1 magic" );
	if ( in.U16() != kArtifactVersion )
		return Problem( "PVS1 version" );
	out.drawConstantRegister = in.U8();
	const std::uint8_t uniformCount = in.U8();
	out.vertexIndexRegister = in.U8();
	const std::uint8_t reserved = in.U8() | in.U8() | in.U8();
	const std::uint32_t dvlbBytes = in.U32();
	if ( reserved != 0 )
		return Problem( "PVS1 reserved byte set" );
	if ( out.vertexIndexRegister != kNone && out.vertexIndexRegister >= kInputRegisters )
		return Problem( "PVS1 vertex index register outside v0 to v15" );
	if ( !in.Has( std::size_t( uniformCount ) * kUniformBytes ) )
		return Problem( "PVS1 uniform table truncated" );

	std::array<bool, kFloatUniformRegisters> used{};
	auto claim = [&used]( std::uint32_t first, std::uint32_t count )
	{
		if ( count == 0 || first + count > kFloatUniformRegisters )
			return false;
		for ( std::uint32_t r = first; r < first + count; ++r )
		{
			if ( used[r] )
				return false;
			used[r] = true;
		}
		return true;
	};

	for ( std::uint8_t i = 0; i < uniformCount; ++i )
	{
		UniformRange range;
		range.group = in.U8();
		range.binding = in.U8();
		range.firstRegister = in.U8();
		range.registerCount = in.U8();
		if ( range.group >= kMaxBindGroups )
			return Problem( "PVS1 uniform group out of range" );
		for ( const UniformRange &earlier : out.uniforms )
			if ( earlier.group == range.group && earlier.binding == range.binding )
				return Problem( "PVS1 binding listed twice" );
		if ( !claim( range.firstRegister, range.registerCount ) )
			return Problem( "PVS1 uniform registers out of range or overlapping" );
		out.uniforms.push_back( range );
	}

	if ( out.drawConstantRegister != kNone )
	{
		if ( drawConstantBytes == 0 )
			return Problem( "PVS1 reads draw constants the pipeline does not declare" );
		const std::uint32_t registers = ( drawConstantBytes + kRegisterBytes - 1 ) / kRegisterBytes;
		if ( !claim( out.drawConstantRegister, registers ) )
			return Problem( "PVS1 draw-constant registers out of range or overlapping" );
	}

	if ( dvlbBytes < 8 || in.Left() != dvlbBytes )
		return Problem( "PVS1 DVLB size does not match the artifact" );
	out.dvlb = in.Take( dvlbBytes );
	Reader dvlb( out.dvlb );
	if ( dvlb.U32() != kDvlbMagic )
		return Problem( "PVS1 DVLB magic" );
	if ( dvlb.U32() != 1 )
		return Problem( "PVS1 DVLB must hold exactly one DVLE" );
	return {};
}

ArtifactProblem ReadFragmentProgram(
    std::span<const std::byte> code, std::uint32_t drawConstantBytes, FragmentProgram &out )
{
	out = {};
	Reader in( code );
	if ( !in.Has( kFragmentHeaderBytes ) )
		return Problem( "PFP1 shorter than its header" );
	if ( in.U32() != kFragmentMagic )
		return Problem( "PFP1 magic" );
	if ( in.U16() != kArtifactVersion )
		return Problem( "PFP1 version" );
	const std::uint8_t stageCount = in.U8();
	const std::uint8_t textureCount = in.U8();
	out.alphaTest = in.U8();
	out.alphaReference = in.U8();
	out.bufferWrite = in.U8();
	const std::uint8_t specializationCount = in.U8();
	out.bufferColor = in.U32();
	if ( stageCount < 1 || stageCount > kMaxCombinerStages )
		return Problem( "PFP1 stage count outside 1 to 6" );
	if ( textureCount > kMaxTextureUnits )
		return Problem( "PFP1 more than three texture units" );
	if ( out.alphaTest != kNone && out.alphaTest > test::kGreaterEqual )
		return Problem( "PFP1 alpha test function" );
	if ( in.Left() != std::size_t( textureCount ) * kTextureBytes +
	                      std::size_t( stageCount ) * kStageBytes +
	                      std::size_t( specializationCount ) * kSpecializationBytes )
		return Problem( "PFP1 size does not match its counts" );

	for ( std::uint8_t i = 0; i < textureCount; ++i )
	{
		TextureUnit unit;
		unit.textureGroup = in.U8();
		unit.textureBinding = in.U8();
		unit.samplerGroup = in.U8();
		unit.samplerBinding = in.U8();
		if ( unit.textureGroup >= kMaxBindGroups || unit.samplerGroup >= kMaxBindGroups )
			return Problem( "PFP1 texture group out of range" );
		out.textures.push_back( unit );
	}

	for ( std::uint8_t i = 0; i < stageCount; ++i )
	{
		CombinerStage stage;
		for ( auto &value : stage.rgbSources )
			value = in.U8();
		for ( auto &value : stage.alphaSources )
			value = in.U8();
		for ( auto &value : stage.rgbOperands )
			value = in.U8();
		for ( auto &value : stage.alphaOperands )
			value = in.U8();
		stage.rgbCombine = in.U8();
		stage.alphaCombine = in.U8();
		stage.rgbScale = in.U8();
		stage.alphaScale = in.U8();
		const std::uint8_t kind = in.U8();
		stage.constantGroup = in.U8();
		stage.constantBinding = in.U8();
		const std::uint8_t pad = in.U8();
		stage.constantOffset = in.U32();
		stage.constantColor = in.U32();
		if ( pad != 0 )
			return Problem( "PFP1 reserved byte set" );

		for ( std::size_t s = 0; s < 3; ++s )
		{
			if ( !ValidSource( stage.rgbSources[s], out.textures.size() ) ||
			     !ValidSource( stage.alphaSources[s], out.textures.size() ) )
				return Problem( "PFP1 source unknown or naming an absent texture unit" );
			if ( i == 0 && ( stage.rgbSources[s] == source::kPrevious ||
			                   stage.alphaSources[s] == source::kPrevious ) )
				return Problem( "PFP1 first stage reads the previous stage" );
			if ( stage.rgbOperands[s] > kRgbOperandLast ||
			     stage.alphaOperands[s] > kAlphaOperandLast )
				return Problem( "PFP1 operand out of range" );
		}
		if ( stage.rgbCombine > kCombineLast || stage.alphaCombine > kCombineLast )
			return Problem( "PFP1 combine function out of range" );
		// Dot3 is a colour-channel function; the alpha combiner has no such mode.
		if ( stage.alphaCombine == kCombineDot3Rgb || stage.alphaCombine == kCombineDot3Rgba )
			return Problem( "PFP1 dot3 in the alpha combiner" );
		if ( stage.rgbScale > kScaleLast || stage.alphaScale > kScaleLast )
			return Problem( "PFP1 scale out of range" );
		switch ( kind )
		{
		case std::uint8_t( ConstantKind::kLiteral ):
			break;
		case std::uint8_t( ConstantKind::kDrawConstants ):
			if ( stage.constantOffset % 4 != 0 ||
			     std::uint64_t( stage.constantOffset ) + kVec4Bytes > drawConstantBytes )
				return Problem( "PFP1 constant outside the draw-constant block" );
			break;
		case std::uint8_t( ConstantKind::kUniformBuffer ):
			if ( stage.constantGroup >= kMaxBindGroups || stage.constantOffset % 4 != 0 )
				return Problem( "PFP1 uniform-buffer constant group or offset" );
			break;
		default:
			return Problem( "PFP1 constant kind" );
		}
		stage.constantKind = ConstantKind( kind );
		out.stages.push_back( stage );
	}
	// The GPU's buffer-update bits exist for the first four stages.
	if ( ( out.bufferWrite & 0x0F ) >> std::min<std::uint32_t>( stageCount, 4 ) ||
	     ( out.bufferWrite >> 4 ) >> std::min<std::uint32_t>( stageCount, 4 ) )
		return Problem( "PFP1 buffer write names an absent stage" );

	for ( std::uint8_t i = 0; i < specializationCount; ++i )
	{
		Specialization entry;
		entry.id = in.U32();
		entry.value = in.U32();
		entry.stage = in.U8();
		const std::uint8_t reservedBytes = in.U8() | in.U8() | in.U8();
		entry.color = in.U32();
		if ( reservedBytes != 0 )
			return Problem( "PFP1 reserved byte set" );
		if ( entry.stage >= stageCount )
			return Problem( "PFP1 specialization names an absent stage" );
		out.specializations.push_back( entry );
	}
	return {};
}

std::vector<ReflectedBinding> BindingsOf( const VertexProgram &program )
{
	std::vector<ReflectedBinding> bindings;
	for ( const UniformRange &range : program.uniforms )
		bindings.push_back( { range.group, range.binding, BindingKind::kUniformBuffer } );
	return bindings;
}

std::vector<ReflectedBinding> BindingsOf( const FragmentProgram &program )
{
	std::vector<ReflectedBinding> bindings;
	auto add = [&bindings]( std::uint32_t group, std::uint32_t binding, BindingKind kind )
	{
		for ( const ReflectedBinding &existing : bindings )
			if ( existing.group == group && existing.binding == binding && existing.kind == kind )
				return;
		bindings.push_back( { group, binding, kind } );
	};
	for ( const TextureUnit &unit : program.textures )
	{
		add( unit.textureGroup, unit.textureBinding, BindingKind::kSampledTexture );
		add( unit.samplerGroup, unit.samplerBinding, BindingKind::kSampler );
	}
	for ( const CombinerStage &stage : program.stages )
		if ( stage.constantKind == ConstantKind::kUniformBuffer )
			add( stage.constantGroup, stage.constantBinding, BindingKind::kUniformBuffer );
	return bindings;
}

std::uint32_t DrawConstantBytesOf( const VertexProgram &program )
{
	// The program may read any word of its registers; the pipeline's block
	// decides how many (ReadVertexProgram claimed exactly those).
	(void)program;
	return 0;
}

std::uint32_t DrawConstantBytesOf( const FragmentProgram &program )
{
	std::uint32_t bytes = 0;
	for ( const CombinerStage &stage : program.stages )
		if ( stage.constantKind == ConstantKind::kDrawConstants )
			bytes = std::max( bytes, stage.constantOffset + kVec4Bytes );
	return bytes;
}

std::vector<CombinerStage> Specialize(
    const FragmentProgram &program, std::span<const SpecializationConstant> constants )
{
	std::vector<CombinerStage> stages = program.stages;
	for ( const SpecializationConstant &constant : constants )
	{
		if ( constant.stage != ShaderStage::kFragment )
			continue;
		for ( const Specialization &entry : program.specializations )
		{
			if ( entry.id != constant.id || entry.value != constant.value )
				continue;
			CombinerStage &stage = stages[entry.stage];
			stage.constantKind = ConstantKind::kLiteral;
			stage.constantColor = entry.color;
		}
	}
	return stages;
}

std::uint32_t PackConstant( const float ( &value )[4] )
{
	std::uint32_t packed = 0;
	for ( int c = 0; c < 4; ++c )
	{
		const float v = std::isnan( value[c] ) ? 0.0f : std::clamp( value[c], 0.0f, 1.0f );
		packed |= std::uint32_t( std::lround( v * 255.0f ) ) << ( 8 * c );
	}
	return packed;
}

std::vector<std::byte> WriteVertexProgram(
    const VertexProgram &program, std::span<const std::byte> dvlb )
{
	Writer out;
	out.U32( kVertexMagic );
	out.U16( kArtifactVersion );
	out.U8( program.drawConstantRegister );
	out.U8( std::uint8_t( program.uniforms.size() ) );
	out.U8( program.vertexIndexRegister );
	out.U8( 0 );
	out.U8( 0 );
	out.U8( 0 );
	out.U32( std::uint32_t( dvlb.size() ) );
	for ( const UniformRange &range : program.uniforms )
	{
		out.U8( range.group );
		out.U8( range.binding );
		out.U8( range.firstRegister );
		out.U8( range.registerCount );
	}
	out.Bytes( dvlb );
	return out.Take();
}

std::vector<std::byte> WriteFragmentProgram( const FragmentProgram &program )
{
	Writer out;
	out.U32( kFragmentMagic );
	out.U16( kArtifactVersion );
	out.U8( std::uint8_t( program.stages.size() ) );
	out.U8( std::uint8_t( program.textures.size() ) );
	out.U8( program.alphaTest );
	out.U8( program.alphaReference );
	out.U8( program.bufferWrite );
	out.U8( std::uint8_t( program.specializations.size() ) );
	out.U32( program.bufferColor );
	for ( const TextureUnit &unit : program.textures )
	{
		out.U8( unit.textureGroup );
		out.U8( unit.textureBinding );
		out.U8( unit.samplerGroup );
		out.U8( unit.samplerBinding );
	}
	for ( const CombinerStage &stage : program.stages )
	{
		for ( auto value : stage.rgbSources )
			out.U8( value );
		for ( auto value : stage.alphaSources )
			out.U8( value );
		for ( auto value : stage.rgbOperands )
			out.U8( value );
		for ( auto value : stage.alphaOperands )
			out.U8( value );
		out.U8( stage.rgbCombine );
		out.U8( stage.alphaCombine );
		out.U8( stage.rgbScale );
		out.U8( stage.alphaScale );
		out.U8( std::uint8_t( stage.constantKind ) );
		out.U8( stage.constantGroup );
		out.U8( stage.constantBinding );
		out.U8( 0 );
		out.U32( stage.constantOffset );
		out.U32( stage.constantColor );
	}
	for ( const Specialization &entry : program.specializations )
	{
		out.U32( entry.id );
		out.U32( entry.value );
		out.U8( entry.stage );
		out.U8( 0 );
		out.U8( 0 );
		out.U8( 0 );
		out.U32( entry.color );
	}
	return out.Take();
}

} // namespace render::device::pica_format
