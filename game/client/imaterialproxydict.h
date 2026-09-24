#ifndef IMATERIALPROXYDICT_H
#define IMATERIALPROXYDICT_H

#include "materialsystem/imaterialproxy.h"
#include "tier1/interface.h"

#define EXPOSE_MATERIAL_PROXY( className, proxyName ) \
	EXPOSE_INTERFACE( className, IMaterialProxy, #proxyName IMATERIAL_PROXY_INTERFACE_VERSION )

#endif
