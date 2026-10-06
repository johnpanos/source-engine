# Render backend switches for ./play and ./play_p2 (sourced, bash).
#
# render_flags_parse "$@" consumes the leading render flags and sets:
#   RENDER_BACKEND      native | dxvk | null   (native unless a flag says otherwise)
#   RENDER_GAME_ARGS    game arguments the flags imply (an array)
#   RENDER_REST         the remaining arguments, unchanged (an array)
#
# RENDER_CORE_WORLD_DEFAULT (1 or 0, set by the caller before parsing) is
# whether the render core draws the world without a flag: ./play and ./play_p2
# set 1, the render core's best default on native Vulkan (user direction,
# 2026-09-29); CORE_WORLD=0 or --no-core-world turns it off. The engine's own
# default stays r_core_world 0 until RFC 0016 K12's "game matches lab".
#
# Flags (any order, before the map and game arguments):
#   --native            native Vulkan renderer (shaderapivulkan): the default
#   --dxvk              D3D9 renderer through DXVK Native (./play only)
#   --null              no rendering (shaderapiempty), for headless checks
#   --core-world        the render core draws the BSP world it can (RFC 0016 K5,
#                       r_core_world 1, strict); legacy draws the rest. The
#                       default in ./play and ./play_p2 on native Vulkan
#   --no-core-world     the legacy world only (r_core_world 0)
#   --no-core           no render core at all (-norendercore): the legacy
#                       backend alone, as before RFC 0016
#   --core-probe=MODE   core-pass probe at each view's opaque stage: empty or
#                       seeded-clear (RFC 0016 K5 step 3)
#   --no-legacy-ports   native Vulkan without the stdshader GLSL ports
#                       (-novklegacyports)
#   --validate          Khronos validation with synchronization validation
#                       (-vkvalidate)
#   --moving-light-gi   runtime indirect light for moved and switched lights
#                       (RFC 0011's SDF producer, +r_indirect_producer sdf).
#                       Off by default: moving-light GI is out of scope (user
#                       decision, 2026-09-30), so the indirect light is the
#                       bake's (+r_indirect_producer baked)
#   --baked-direct      the bake's direct light on the core's world (the total
#                       lightmap layer, +r_core_runtime_direct 0) instead of
#                       every light's direct light drawn at runtime, shadowed
#                       (id Tech's split, the default)
#   --area-lights       runtime LTC area lights (+r_core_area_lights 1). Off by
#                       default (user direction, 2026-10-06: Source 2's
#                       lighting): fixtures light through the bake and reflect
#                       through the probes
#   --hard-shadows      soft shadows (PCSS) off (+r_core_shadow_pcss 0; Advanced
#                       Video: Soft Shadows (PCSS))
#   --render-help       this list
render_flags_usage()
{
	sed -n '/^# Flags/,/^#   --render-help/p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
}

render_flags_parse()
{
	RENDER_BACKEND=native
	RENDER_GAME_ARGS=()
	RENDER_REST=()
	local core_world=${RENDER_CORE_WORLD_DEFAULT:-0}
	local no_core=0
	local producer=baked
	local runtime_direct=1
	local area_lights=0
	while [ $# -gt 0 ]; do
		case "$1" in
			--native) RENDER_BACKEND=native ;;
			--dxvk) RENDER_BACKEND=dxvk ;;
			--null) RENDER_BACKEND=null ;;
			--core-world) core_world=1 ;;
			--no-core-world) core_world=0 ;;
			--no-core) no_core=1; RENDER_GAME_ARGS+=(-norendercore) ;;
			--core-probe=*) RENDER_GAME_ARGS+=(-render-core-passes "${1#--core-probe=}") ;;
			--no-legacy-ports) RENDER_GAME_ARGS+=(-novklegacyports) ;;
			--validate) RENDER_GAME_ARGS+=(-vkvalidate) ;;
			--moving-light-gi) producer=sdf ;;
			--baked-direct) runtime_direct=0 ;;
			--area-lights) area_lights=1 ;;
			--hard-shadows) RENDER_GAME_ARGS+=(+r_core_shadow_pcss 0) ;;
			--render-help) render_flags_usage; exit 0 ;;
			*) break ;;
		esac
		shift
	done
	RENDER_REST=("$@")
	# The core world draws through the native backend's core passes only.
	if [ "$core_world" = 1 ] && [ "$no_core" = 0 ] && [ "$RENDER_BACKEND" = native ]; then
		RENDER_GAME_ARGS+=(+sv_cheats 1 +r_core_world 1)
	fi
	# The indirect-light producer (an archived setting, so always passed) and
	# the core's direct light.
	RENDER_GAME_ARGS+=(+r_indirect_producer "$producer" +r_core_runtime_direct "$runtime_direct"
		+r_core_area_lights "$area_lights")
}
