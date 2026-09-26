# launcher_ccache.sh — the ccache settings shared by the launchers' Waf builds
# (./play through run.sh, and ./play_p2). Source it, then run
#   launcher_ccache <waf output tree>
# before `waf build`.
#
# wscript's build() owns the cache location: it sets CCACHE_DIR to
# <repo>/.ccache/<COMPILER_CC>/<DEST_OS>/<DEST_CPU>, so every tree configured
# with the same compiler and target (build/ and build-p2/ are gcc/linux/x86_64)
# compiles into one cache. This exports that same directory, read from the
# tree's own configuration, so the limit below applies to it.
#
# With CCACHE_DIR set, ccache reads $CCACHE_DIR/ccache.conf instead of
# ~/.config/ccache/ccache.conf, so this cache had ccache's default 5 GiB
# limit, and the many Waf trees sharing it evicted each other's objects. The
# limit is written into the cache's own ccache.conf (once; a value already
# there wins), so every tree using the directory keeps it, not only launcher
# builds. LAUNCHER_CCACHE_MAX_SIZE picks the size.
#
# The two trees' command lines differ by Portal 2's global STUDIO_MAX_BONES_256
# and HAVE_AV* defines, so their direct-mode keys match only where those are
# absent. A translation unit whose preprocessed output is the same in both
# trees still hits through ccache's preprocessor mode. GIT_COMMIT_HASH is
# defined for tier0 only (2026-09-25), so a new commit or reconfigure no longer
# changes every other compile command. Waf passes paths
# relative to the output tree, and both trees sit one level below the
# repository, so no CCACHE_BASEDIR rewriting is needed.

launcher_ccache() {
    local cache=$1/c4che/_cache.py root compiler os cpu
    command -v ccache >/dev/null 2>&1 || return 0
    [ -f "$cache" ] || return 0     # not configured yet: wscript picks the directory
    root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
    compiler=$(sed -n "s/^COMPILER_CC = '\(.*\)'$/\1/p" "$cache")
    os=$(sed -n "s/^DEST_OS = '\(.*\)'$/\1/p" "$cache")
    cpu=$(sed -n "s/^DEST_CPU = '\(.*\)'$/\1/p" "$cache")
    [ -n "$compiler" ] && [ -n "$os" ] && [ -n "$cpu" ] || return 0
    export CCACHE_DIR="$root/.ccache/$compiler/$os/$cpu"
    mkdir -p "$CCACHE_DIR"
    if ! grep -qs '^max_size' "$CCACHE_DIR/ccache.conf"; then
        ccache --set-config=max_size="${LAUNCHER_CCACHE_MAX_SIZE:-20G}" >/dev/null
    fi
}
