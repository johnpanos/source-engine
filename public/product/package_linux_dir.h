//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.package.linux-dir (RFC 0027 L1): the `linux-dir` packager,
//			a runnable directory tree for desktop Linux. It replaces the run/
//			staging of the retired tools/quality/stage_runtime.py and its siblings.
//
//			The package is a persistent runtime directory: the game writes its
//			own files there (configs, saves, logs). The packager owns only the
//			entries it places; each is replaced through a temporary name and a
//			rename, and entries it does not own are never touched. Its manifest
//			lists the entries it owns.
//
//			The steps are profile data (`package.steps`, run in order):
//			  {"op": "seed", "from": <locator>, "marker": <path>, "game": <dir>,
//			   "content_only": bool, "shared_suffixes": [...],
//			   "skip_directories": [...], "skip_suffixes": [...]}
//			    Seed once (while `marker` is missing) from a located base tree:
//			    files with a shared suffix become links to the source, others
//			    private copies; linked directories stay links.
//			  {"op": "overlay", "artifact": <name>, "include": [globs]}
//			    Copy the artifact's files matching `include` into place.
//			  {"op": "remove-elf32", "directories": [...]}
//			    Remove 32-bit ELF shared objects the 64-bit engine cannot load.
//			  {"op": "mount-each", "from": <locator>, "into": <dir>,
//			   "prefix": <name prefix>, "record": <file>, "require": <path>,
//			   "skip_if_exists": <path>, "optional": bool}
//			    Link every entry of a located store that has `record` and
//			    `require` as <into>/<prefix><name>, relative; a stale managed
//			    link is removed; `{name}` in paths is the entry's name.
//			  {"op": "link", "path": <path>, "from": <locator>, "source": <dir>,
//			   "check": <file>, "optional": bool}
//			    Link a located directory; an optional link whose content is
//			    absent is removed.
//			  {"op": "search-paths", "file": <gameinfo>, "block": "SearchPaths",
//			   "lines": [<text> | {"line", "when_exists", "each_directory"}]}
//			    Rewrite the block (the file is rewritten with LF newlines, as
//			    text); unchanged text keeps its modification time.
//			  {"op": "extract", "from": <locator>, "archive": <vpk>,
//			   "entry": <path>, "path": <path>}
//			    Copy one VPK entry into place when it is missing.
//			Globs: `*` matches within one path segment, `**` across segments.
//
//=============================================================================//

#ifndef PUBLIC_PRODUCT_PACKAGE_LINUX_DIR_H
#define PUBLIC_PRODUCT_PACKAGE_LINUX_DIR_H

#include "product/contracts.h"

#include <memory>

namespace product
{

inline constexpr std::string_view kLinuxDirPackager = "linux-dir";

std::unique_ptr<IPackager> CreateLinuxDirPackager();

// Glob matching as the steps use it (exposed for tests).
bool GlobMatch( std::string_view pattern, std::string_view path );

} // namespace product

#endif // PUBLIC_PRODUCT_PACKAGE_LINUX_DIR_H
