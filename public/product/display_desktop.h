//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.display.desktop (RFC 0027 L1): the desktop display
//			sessions.
//			  `user`    the user's own session: nothing changes.
//			  `none`    no display: SDL's offscreen driver, no window anywhere.
//			  `private` a headless mutter with a virtual monitor, on a private
//			            D-Bus whose configuration blocks the Flatpak document
//			            portal (the retired tools/quality/private_session.py's reason: a
//			            second portal unmounts the login session's
//			            $XDG_RUNTIME_DIR/doc when the private bus ends). The
//			            launch runs as mutter's child, so the compositor ends
//			            with it.
//
//=============================================================================//

#ifndef PUBLIC_PRODUCT_DISPLAY_DESKTOP_H
#define PUBLIC_PRODUCT_DISPLAY_DESKTOP_H

#include "product/contracts.h"

#include <memory>

namespace product
{

std::unique_ptr<IDisplaySession> CreateUserDisplaySession();
std::unique_ptr<IDisplaySession> CreateHeadlessDisplaySession();
// `sessionConfigs` are the system D-Bus session configurations to include,
// first existing wins (normally /usr/share/dbus-1/session.conf, then
// /etc/dbus-1/session.conf); given by the root, never looked up here.
std::unique_ptr<IDisplaySession> CreatePrivateDisplaySession(
    std::vector<std::filesystem::path> sessionConfigs );
// The same isolated session for programs that need X11 (`private-x11`: SDL
// on mutter's Xwayland, as the 32-bit retail Portal 2 does).
std::unique_ptr<IDisplaySession> CreatePrivateX11DisplaySession(
    std::vector<std::filesystem::path> sessionConfigs );

} // namespace product

#endif // PUBLIC_PRODUCT_DISPLAY_DESKTOP_H
