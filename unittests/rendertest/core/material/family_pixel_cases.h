//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: What the render.family.* suites (RFC 0016 K4 "Families match
//			ports") share: reading a family's case file
//			(quality/fixtures/legacy-shaders/families/<family>.vdf) and its
//			recorded port pixels (quality/fixtures/render-families/
//			<family>-port-v1.vdf), importing a case's material, drawing a
//			case's quad with a family's pipeline and bind groups into the
//			material pixel harness's 256x256 sRGB target, and judging the
//			drawn pixels against the port's.
//
//			A family suite supplies what differs: its claim, its constants,
//			its vertex layout and which case textures its groups bind.
//
//=============================================================================//

#ifndef RENDERTEST_FAMILY_PIXEL_CASES_H
#define RENDERTEST_FAMILY_PIXEL_CASES_H

#include "render/device/device.h"
#include "render/material/registry.h"
#include "render/material/vmt_import.h"
#include "testing/checks.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace rendertest::families
{

inline constexpr std::uint32_t kSize = 256; // the material pixel harness's target
inline constexpr int kTolerance = 2;        // levels per channel, against the port

struct CaseTexture
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	bool clamp = false;
	bool point = false;
	bool cube = false; // six faces (+x -x +y -y +z -z), one after the other
	bool array = false; // two layers of the one image (a texture2DArray binding)
	render::device::Format format = render::device::Format::kRGBA8Srgb;
	std::vector<std::uint8_t> texels; // bytes in `format`, row 0 at the top
};

struct CaseVertex
{
	float position[3] = {};
	float uv0[2] = {};
	float uv1[2] = {};
	float normal[3] = {};
	float tangentS[3] = {};
	float tangentT[3] = {};
	float uv2[2] = {}; // TEXCOORD2: x, the bumped lightmap pages' offset
	std::uint8_t color[4] = { 255, 255, 255, 255 };
};

struct PortPixel
{
	int x = 0;
	int y = 0;
	int rgba[4] = {};
	int channels = 4; // 3 when the port's capture holds no alpha
};

// A Source model light (LightDesc_t), as a case or a model fixture gives it.
struct ModelLight
{
	std::string type; // "point", "spot" or "directional"
	float color[3] = {};
	float position[3] = {};
	float direction[3] = {};
	float attenuation[3] = {};
	float theta = 0.0f;
	float phi = 0.0f;
	float falloff = 0.0f;
};

struct FamilyCase
{
	std::string name;
	std::string vmt; // the case's material as VMT text
	const CaseTexture *lightmap = nullptr;
	std::vector<CaseVertex> triangles; // the quad as two triangles
	int clear[4] = { 0, 0, 0, 255 };
	std::vector<PortPixel> port; // the port's pixels from the fixture
	// The case's vertex lighting: "ambient" (+x -x +y -y +z -z) and its
	// "light" blocks, with the material pixel harness's defaults for absent
	// keys.
	float cube[6][3] = {};
	std::vector<ModelLight> lights;
};

struct CaseSet
{
	std::map<std::string, CaseTexture> textures; // by normalized name
	std::vector<FamilyCase> cases;
};

// Reads the case file and the fixture and checks their setup clauses
// (setup.case-file-parses, setup.fixture-parses,
// fixture.records-the-current-case-file, case.<name>.is-a-quad). nullopt
// when either file does not parse.
std::optional<CaseSet> LoadCases(
    testing::Checks &checks, const char *caseFile, const char *fixtureFile );

// A model family's fixture (family_port_pixels.py record-model): the model
// pixel harness's own inputs, carried with the port's pixels. Each case is a
// FamilyCase (its material as VMT text, the port's RGB pixels) with its
// placement and Source model lighting.
struct ModelQuad
{
	float corners[4][2] = {}; // clip-space x, y at the fixture's quad_z
	float normal[3] = {};
	float tangent[4] = {};
};

struct ModelCase
{
	FamilyCase common;          // name, vmt, clear, port pixels
	float modelMatrix[12] = {}; // 3x4, row-major with column vectors
	float cube[6][3] = {};
	std::vector<ModelLight> lights;
};

struct ModelCaseSet
{
	std::map<std::string, CaseTexture> textures; // by normalized name, RGBA8 sRGB
	std::vector<ModelQuad> quads;
	std::vector<ModelCase> cases;
	float quadZ = 0.0f;
	float eye[3] = {};
};

// Reads a model fixture: setup.fixture-parses and case.<name>.has-a-material.
std::optional<ModelCaseSet> LoadModelCases( testing::Checks &checks, const char *fixtureFile );

// The texture a material value names, or nullptr.
const CaseTexture *FindTexture( const CaseSet &set, const std::string &name );
const CaseTexture *FindTexture(
    const std::map<std::string, CaseTexture> &textures, const std::string &name );

// A registry with every family of the built-in VMT mapping.
render::material::FamilyRegistry BuiltinFamilies();

// The case's material imported and applied to a block of `family`, with a
// stand-in id for every texture the material binds (a claim only asks
// whether one is bound). Checks import.<name>.is-<family> and .applies.
struct ImportedCase
{
	render::material::MaterialDesc material;
	std::optional<render::material::ParameterBlock> block;
};
ImportedCase ImportCase( testing::Checks &checks, const FamilyCase &testCase,
    const render::material::FamilySchema &family );

// A check whose failure prints why.
bool That(
    testing::Checks &checks, bool condition, const std::string &what, const std::string &detail );

// One bind group of a draw: binding 0 the constants when there are any, then
// a texture and its sampler per case texture.
// A 1x1 white texture (six faces for a cube) in `format`: the neutral input
// an unread slot of the surface program takes.
CaseTexture NeutralCaseTexture( render::device::Format format, bool cube = false );

struct CaseGroup
{
	render::device::BindGroupRole role = render::device::BindGroupRole::kMaterial;
	render::device::BindGroupLayoutId layout;
	std::span<const std::byte> constants;
	std::vector<const CaseTexture *> textures;
	// The bindings are each texture and its sampler, with the constants and
	// then each storage buffer placed after the first constantsAfter textures
	// (0: the constants at binding 0; the surface draw group: after its page,
	// before its gradient page).
	std::uint32_t constantsAfter = 0;
	std::vector<std::span<const std::byte>> storage = {}; // read-only storage buffers
};

// The surface program's view group of a view with no clustered lights
// (SurfaceProgram::NeutralViewGroup), for a draw that reads none.
CaseGroup NeutralViewGroup( render::device::BindGroupLayoutId layout );

struct CaseDraw
{
	render::device::PipelineId pipeline;
	std::vector<CaseGroup> groups;
	std::span<const std::byte> vertices;
	std::uint32_t vertexCount = 0;
	int clear[4] = { 0, 0, 0, 255 };
	// The draw constants; empty for CaseToClip() alone.
	std::span<const std::byte> drawConstants;
};

// The draw constants' world-to-clip for a case: the cases are clip-space
// quads in D3D9's convention, where pixel centers sit on integer
// coordinates, so the half-pixel shift the legacy frontend applies to a D3D9
// transform (content right and down by half a pixel), then the identity.
std::array<float, 16> CaseToClip();

struct Drawn
{
	bool ok = false;
	std::vector<std::uint8_t> rgba; // kSize x kSize, row 0 at the top
};

Drawn DrawCase( render::device::IRenderDevice2 &device, const CaseDraw &draw );

// Judges the drawn pixels against the port's: fixture.<name>.has-port-pixels
// and pixels.<name>.within-tolerance-of-the-port.
void JudgeCase( testing::Checks &checks, const FamilyCase &testCase, const Drawn &drawn );

} // namespace rendertest::families

#endif // RENDERTEST_FAMILY_PIXEL_CASES_H
