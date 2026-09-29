# Render backend switches for ./play and ./play_p2 (sourced, bash).
#
# render_flags_parse "$@" consumes the leading render flags and sets:
#   RENDER_BACKEND      native | dxvk | null   (native unless a flag says otherwise)
#   RENDER_GAME_ARGS    game arguments the flags imply (an array)
#   RENDER_REST         the remaining arguments, unchanged (an array)
#
# Flags (any order, before the map and game arguments):
#   --native            native Vulkan renderer (shaderapivulkan): the default
#   --dxvk              D3D9 renderer through DXVK Native (./play only)
#   --null              no rendering (shaderapiempty), for headless checks
#   --core-world        the render core draws the BSP world it can (RFC 0016 K5,
#                       r_core_world 1); legacy draws the rest
#   --no-core           no render core at all (-norendercore): the legacy
#                       backend alone, as before RFC 0016
#   --core-probe=MODE   core-pass probe at each view's opaque stage: empty or
#                       seeded-clear (RFC 0016 K5 step 3)
#   --no-legacy-ports   native Vulkan without the stdshader GLSL ports
#                       (-novklegacyports)
#   --validate          Khronos validation with synchronization validation
#                       (-vkvalidate)
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
	while [ $# -gt 0 ]; do
		case "$1" in
			--native) RENDER_BACKEND=native ;;
			--dxvk) RENDER_BACKEND=dxvk ;;
			--null) RENDER_BACKEND=null ;;
			--core-world) RENDER_GAME_ARGS+=(+sv_cheats 1 +r_core_world 1) ;;
			--no-core) RENDER_GAME_ARGS+=(-norendercore) ;;
			--core-probe=*) RENDER_GAME_ARGS+=(-render-core-passes "${1#--core-probe=}") ;;
			--no-legacy-ports) RENDER_GAME_ARGS+=(-novklegacyports) ;;
			--validate) RENDER_GAME_ARGS+=(-vkvalidate) ;;
			--render-help) render_flags_usage; exit 0 ;;
			*) break ;;
		esac
		shift
	done
	RENDER_REST=("$@")
}
