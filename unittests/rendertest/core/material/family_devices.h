//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: the devices a family pixel suite draws its cases on (RFC 0016 K4
//			"Families match ports", K10 "Pixels").
//
//			Built as before, a suite draws on render.device.vulkan alone.
//			With RENDERTEST_FAMILY_GL it draws on render.device.gl alone, and
//			its cases are judged against the same port fixtures. With
//			RENDERTEST_FAMILY_CROSS it draws every case on Vulkan, then on
//			OpenGL, and each GL frame must also be within the cross-backend
//			tolerance of the Vulkan frame of the same case over every pixel
//			(pixels.<case>.gl-within-cross-backend-tolerance-of-vulkan): the
//			per-case limits (levels, and channels allowed over them) are the
//			recorded fixture
//			quality/fixtures/render-families/cross-backend-v1.vdf, never
//			byte identity (AGENTS.md: no cross-backend pixel identity).
//
//			RENDERTEST_FAMILY_GLES, with either, selects the GL adapter's
//			OpenGL ES 3.1 dialect (RFC 0022 E6) in place of OpenGL 4.5; the
//			entry is named "gles" and the same fixtures and limits judge it.
//
//			Each device's debug or validation messages are counted, and with
//			the layer or debug output available the run must report none.
//
//=============================================================================//

#ifndef RENDERTEST_FAMILY_DEVICES_H
#define RENDERTEST_FAMILY_DEVICES_H

#include "family_pixel_cases.h"
#include "render/device/device.h"
#include "testing/checks.h"

#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace rendertest::families
{

class CaseDevices
{
public:
	struct Entry
	{
		std::string name; // "vulkan" or "gl"
		std::unique_ptr<render::device::IRenderDevice2> device;
	};

	// Creates the build's devices (device.a-<name>-device-is-created); a
	// device that cannot be created is not in `entries`.
	explicit CaseDevices( testing::Checks &checks );
	~CaseDevices();

	std::vector<Entry> entries;

	// Called once per drawn case on each device, in entry order: the first
	// device's frame is the reference; a later device's frame is judged
	// against it with the case's recorded cross-backend limit. A build of
	// one device judges nothing here.
	void Compare( testing::Checks &checks, const Entry &entry, const std::string &caseName,
	    const Drawn &drawn );

	// Waits each device idle, releases them, then judges the message counts
	// (validation.no-messages, gl.debug-output.no-messages) and, in a cross
	// build, that every reference case was compared. Returns checks.Report().
	int Finish( testing::Checks &checks );

private:
	std::atomic<std::uint64_t> m_VulkanMessages{ 0 };
	std::atomic<std::uint64_t> m_GlMessages{ 0 };
	[[maybe_unused]] bool m_VulkanLayer = false;
	bool m_GlDebug = false;
	std::map<std::string, Drawn> m_Reference;
	// The fixture's per-case limit: levels per channel, and how many channels
	// of the frame may exceed them (pixels on a hard shading edge, where the
	// two drivers' compilers round differently).
	struct Limit
	{
		int levels = 0;
		std::size_t outliers = 0;
	};
	std::map<std::string, Limit> m_Limits;
	int m_Compared = 0;
};

} // namespace rendertest::families

#endif // RENDERTEST_FAMILY_DEVICES_H
