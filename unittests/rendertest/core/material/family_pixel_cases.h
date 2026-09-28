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
	std::vector<std::uint8_t> texels; // RGBA8, row 0 at the top
};

struct CaseVertex
{
	float position[3] = {};
	float uv0[2] = {};
	float uv1[2] = {};
	std::uint8_t color[4] = { 255, 255, 255, 255 };
};

struct PortPixel
{
	int x = 0;
	int y = 0;
	int rgba[4] = {};
};

struct FamilyCase
{
	std::string name;
	std::string vmt; // the case's material as VMT text
	const CaseTexture *lightmap = nullptr;
	std::vector<CaseVertex> triangles; // the quad as two triangles
	int clear[4] = { 0, 0, 0, 255 };
	std::vector<PortPixel> port; // the port's pixels from the fixture
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

// The texture a material value names, or nullptr.
const CaseTexture *FindTexture( const CaseSet &set, const std::string &name );

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
struct CaseGroup
{
	render::device::BindGroupRole role = render::device::BindGroupRole::kMaterial;
	render::device::BindGroupLayoutId layout;
	std::span<const std::byte> constants;
	std::vector<const CaseTexture *> textures;
};

struct CaseDraw
{
	render::device::PipelineId pipeline;
	std::vector<CaseGroup> groups;
	std::span<const std::byte> vertices;
	std::uint32_t vertexCount = 0;
	int clear[4] = { 0, 0, 0, 255 };
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
