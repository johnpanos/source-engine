//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: kiln.fixture-platform (RFC 0027 "Proving the core stays closed"):
//			a platform that exists only for tests, with one provider per
//			product contract. A test root composes it into its own catalog;
//			kiln.core, compiled unchanged, then runs build -> content ->
//			package -> deploy -> run on it.
//
//			- toolchain `fixture-host`: the host C++ compiler (`c++`);
//			- stage `fixture-compile` (engine): compiles a one-file program;
//			- stage `fixture-content` (content): encodes an image in the
//			  fixture pixel format (RGB332) and applies the fixture device
//			  layout (a deliberately unusual 2x2 tile byte swizzle);
//			- stage `fixture-symbols` (extra): a symbol list of the program;
//			- packager `fixture-dir`: a directory and its manifest;
//			- transport `fixture-device`: an in-memory fake device that runs
//			  the installed program through the process provider;
//			- display session `fixture-headless`.
//
//=============================================================================//

#ifndef UNITTESTS_KILNTEST_FIXTURE_PLATFORM_H
#define UNITTESTS_KILNTEST_FIXTURE_PLATFORM_H

#include "product/contracts.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace fixture
{

// The fake device: a content root, an install slot and counters the shared
// transport suite observes. Paths are '/'-separated; anything not under
// `root` is outside the device's declared content root.
struct FakeDevice
{
	std::string root = "content";
	bool reachable = true;
	std::map<std::string, std::string> files;   // path -> bytes, root-relative
	std::map<std::string, std::string> outside; // writes or deletes outside the root
	std::string installedDigest;
	std::filesystem::path installedPackage;
	std::uint64_t writes = 0;
	std::uint64_t deletes = 0;
	std::uint64_t installs = 0;
	std::uint64_t launches = 0;
	std::vector<std::string> log;
};

// The fixture pixel format: RGB332, one byte per texel.
std::vector<std::uint8_t> EncodeRgb332(
    const std::vector<std::uint8_t> &rgba, int width, int height );
std::vector<std::uint8_t> DecodeRgb332(
    const std::vector<std::uint8_t> &texels, int width, int height );
// The fixture device layout: within each 2x2 tile the bytes are stored in the
// order 3, 0, 2, 1. Apply and Remove are exact inverses; widths and heights
// are padded to even sizes with zero texels.
std::vector<std::uint8_t> ApplyLayout(
    const std::vector<std::uint8_t> &texels, int width, int height );
std::vector<std::uint8_t> RemoveLayout(
    const std::vector<std::uint8_t> &tiled, int width, int height );

std::unique_ptr<product::ITargetToolchain> CreateHostToolchain(
    platform::IToolProcessProvider &processes );
std::unique_ptr<product::IProductStage> CreateCompileStage();
std::unique_ptr<product::IProductStage> CreateContentStage();
std::unique_ptr<product::IProductStage> CreateSymbolStage();
std::unique_ptr<product::IPackager> CreateDirectoryPackager();
// The device and process provider are borrowed and must outlive the transport.
std::unique_ptr<product::IDeployTransport> CreateFakeTransport(
    FakeDevice &device, platform::IToolProcessProvider &processes );
std::unique_ptr<product::IDisplaySession> CreateHeadlessSession();

// A catalog with every fixture provider and nothing else.
product::ProviderCatalog ComposeFixtureCatalog(
    FakeDevice &device, platform::IToolProcessProvider &processes );

// Helpers shared with the suites.
std::string ReadBytes( const std::filesystem::path &path );
bool WriteBytes( const std::filesystem::path &path, const std::string &bytes );
// Copy a staged directory into `output` through a sibling staging directory
// and one rename; a failure leaves `output` as it was.
bool PublishDirectory( const std::filesystem::path &staging, const std::filesystem::path &output );

} // namespace fixture

#endif // UNITTESTS_KILNTEST_FIXTURE_PLATFORM_H
