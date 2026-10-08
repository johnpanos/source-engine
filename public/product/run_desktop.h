//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.run.desktop (RFC 0027 L1): the desktop run providers.
//			  `single`           one program, waited on.
//			  `external-install` an installed program run as is (the retail
//			                     game), waited on.
//			  `coop-pair`        Portal 2 co-op on two copies of the runtime,
//			                     as the retired ./play_p2_coop did: the host first; once its
//			                     server answers the engine's challenge, the
//			                     client, with {lan_address} filled; then the
//			                     host's log is watched for the remote join.
//			Facts read from the request (profile launch.facts): `port`,
//			`join_log` (relative to the host's working directory), `timeout`
//			(seconds) and `stop_after_join` ("1" stops both once joined).
//			  `browser-page`     a web product: its page server, outside the
//			                     display session, then (once it listens on the
//			                     loopback `port`) the browser on the page, in
//			                     it. The run's status is the server's, which
//			                     ends with the engine's exit status from the
//			                     page; a browser closed first ends the run
//			                     (130). Facts: `port`, `timeout` (seconds for
//			                     the server to listen).
//
//=============================================================================//

#ifndef PUBLIC_PRODUCT_RUN_DESKTOP_H
#define PUBLIC_PRODUCT_RUN_DESKTOP_H

#include "product/contracts.h"

#include <memory>

namespace product
{

std::unique_ptr<IRunProvider> CreateSingleRunProvider();
std::unique_ptr<IRunProvider> CreateExternalInstallRunProvider();
std::unique_ptr<IRunProvider> CreateCoopPairRunProvider();
std::unique_ptr<IRunProvider> CreateBrowserPageRunProvider();

} // namespace product

#endif // PUBLIC_PRODUCT_RUN_DESKTOP_H
