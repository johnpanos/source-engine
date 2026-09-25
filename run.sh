#!/usr/bin/env bash
# run.sh — the single entrypoint to boot Portal from the newest Waf build.
#
#   1. (optional) `waf build` so build/ is current
#   2. stage build/ into a runnable tree (run/runtime), overlaying the newest
#      modules and removing dead 32-bit .so so each module loads exactly once
#   3. launch with the settings from run.conf
#
# Configure it by editing run.conf. Any key there can be overridden by an env
# var of the same name, and any extra command-line args are passed to the game:
#   MAP=testchmb_a_02 ./run.sh -console +sv_cheats 1
set -euo pipefail
cd "$(dirname "$0")"
ROOT="$(pwd)"

# ---- Configuration: built-in defaults < run.conf < environment --------------
CONFIG_KEYS="MAP WIDTH HEIGHT WINDOWED FPS_MAX RENDERER PHYSICS JOB_ARGS MAT_ARGS EXTRA_ARGS \
             BUILD BUILD_DIR RUNTIME BASE_RUNTIME SDL_VIDEODRIVER"
# Remember values already provided via the environment so they win over run.conf.
for k in $CONFIG_KEYS; do eval "__set_$k=\${$k+set}" "__val_$k=\${$k-}"; done

MAP=testchmb_a_01
WIDTH=1920; HEIGHT=1080; WINDOWED=1; FPS_MAX=120
RENDERER=native-vulkan
PHYSICS=vphysics_box3d
JOB_ARGS=                  # job-system ConVars; run.conf enables the pooled graphs
MAT_ARGS=                  # material-quality ConVars; run.conf pins full bump maps
EXTRA_ARGS="-novid -insecure +mat_queue_mode 2"
BUILD=1
BUILD_DIR=build
RUNTIME=run/runtime
BASE_RUNTIME=
SDL_VIDEODRIVER=wayland

[ -f run.conf ] && . ./run.conf                      # run.conf overrides defaults
for k in $CONFIG_KEYS; do                            # environment overrides run.conf
    if [ "$(eval echo \${__set_$k})" = set ]; then eval "$k=\${__val_$k}"; fi
done

# ---- Build (incremental; waf itself is the staleness check) ------------------
if [ "$BUILD" = 1 ]; then
    # Replay the tree's own configure options if a wscript, Waf tool, toolchain
    # policy or profile changed since it was configured.
    python3 tools/quality/ensure_configured.py --build "$BUILD_DIR"
    # The ccache directory and size limit shared with ./play_p2's tree.
    . tools/quality/launcher_ccache.sh
    launcher_ccache "$BUILD_DIR"
    echo "run.sh: building $BUILD_DIR (waf, incremental) ..."
    # Each configured tree keeps its own Waf lock file, so build from inside it.
    (cd "$BUILD_DIR" && python3 "$ROOT/waf" build)
fi
[ -d "$BUILD_DIR" ] || { echo "run.sh: no build output at $BUILD_DIR" >&2; exit 1; }

# ---- Stage the newest build into a clean, runnable tree ----------------------
if [ -z "$BASE_RUNTIME" ]; then
    BASE_RUNTIME="$(ls -dt "$ROOT"/quality-results/*/*/runtime 2>/dev/null | head -n1 || true)"
fi
echo "run.sh: staging $BUILD_DIR -> $RUNTIME"
python3 tools/quality/stage_runtime.py \
    --build "$BUILD_DIR" --runtime "$RUNTIME" \
    ${BASE_RUNTIME:+--base-runtime "$BASE_RUNTIME"}

# ---- Launch -----------------------------------------------------------------
cd "$RUNTIME"
export SDL_VIDEODRIVER="$SDL_VIDEODRIVER"
export SDL_VIDEO_DRIVER="$SDL_VIDEODRIVER"
export DXVK_WSI_DRIVER=SDL3
export SteamAppId=400 SteamGameId=400
export LD_LIBRARY_PATH="$PWD/bin:${LD_LIBRARY_PATH:-}"

# shellcheck disable=SC2206  # JOB_ARGS is intentionally word-split
job_args=($JOB_ARGS)
# Each +command and its values become ONE argument ("+mat_picmip -1"): the
# engine reads a separate argument starting with '-' as a launcher flag, so a
# negative value would otherwise be dropped. The engine runs these after
# config.cfg, so they win over saved settings.
mat_args=()
for word in $MAT_ARGS; do
    if [ "${word#+}" != "$word" ] || [ ${#mat_args[@]} -eq 0 ]; then
        mat_args+=("$word")
    else
        mat_args[-1]+=" $word"
    fi
done
# Job-system and material ConVars go before +map so the first frame (and the
# map's first texture load) already uses them.
cmd=(./hl2_launcher -game portal -w "$WIDTH" -h "$HEIGHT"
     -renderer "$RENDERER" -physics "$PHYSICS" +fps_max "$FPS_MAX" "${job_args[@]}" "${mat_args[@]}" +map "$MAP")
[ "$WINDOWED" = 1 ] && cmd+=(-windowed)
# Headless (offscreen) runs have no one listening: mute them.
[ "$SDL_VIDEODRIVER" = offscreen ] && cmd+=(+volume 0)
# shellcheck disable=SC2206  # EXTRA_ARGS is intentionally word-split
extra=($EXTRA_ARGS)

echo "run.sh: booting map=$MAP ${WIDTH}x${HEIGHT} renderer=$RENDERER physics=$PHYSICS"
exec "${cmd[@]}" "${extra[@]}" "$@"
