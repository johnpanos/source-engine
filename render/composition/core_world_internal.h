//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's world: the includes its files share (RFC 0030).
//			Included only by core_world*.cpp.
//
//=============================================================================//

#pragma once

#include "core_world.h"
#ifdef RENDER_CORE_VULKAN
#include "render/device/vulkan/fsr.h"
#endif

#include "mapcontainer/probe_volume.h"
#include "mapcontainer/light_shadow_masks.h"
#include "mapcontainer/world_lightmap.h"
#include "mapcontainer/reflection_probes.h"
#include "mapcontainer/world_mesh_decode.h"
#include "render/graph/compiled_graph.h"
#include "render/graph/executor.h"
#include "render/graph/graph_builder.h"
#include "render/pass/lights/clusters.h"
#include "render/pass/shadows/atlas.h"
#include "mdl/studio_model.h"
#include "texturecontainer/vtf_decompress.h"

#include <algorithm>
#include <limits>
#include <array>
#include <chrono>
#include <cmath>
#include <cctype>
#include <cstdlib>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <limits>
#include <map>
#include <optional>
